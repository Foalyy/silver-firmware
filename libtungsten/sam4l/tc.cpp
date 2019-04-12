#include "tc.h"
#include "core.h"
#include "pm.h"
#include "scif.h"

namespace TC {

    // Package-dependant, defined in pins_sam4l_XX.cpp
    extern struct GPIO::Pin PINS[MAX_N_TC][N_COUNTERS_PER_TC * N_CHANNELS_PER_COUNTER];
    extern struct GPIO::Pin PINS_CLK[MAX_N_TC][N_COUNTERS_PER_TC * N_CHANNELS_PER_COUNTER];

    // Used to save the counters' current configuration
    struct CounterConfig {
        SourceClock sourceClock;
        unsigned long sourceClockFrequency;
    };
    CounterConfig _countersConfig[MAX_N_TC][N_COUNTERS_PER_TC];

    // Internal list of delayed callbacks to execute
    extern uint8_t INTERRUPT_PRIORITY;
    struct ExecDelayedData {
        uint32_t handler;
        int skipPeriods;
        int skipPeriodsReset;
        int rest;
        int restReset;
        bool repeat;
    };
    ExecDelayedData execDelayedData[MAX_N_TC][N_COUNTERS_PER_TC];

    void execDelayedHandlerWrapper();

    // Internal functions
    inline void checkTC(Counter counter) {
        if (counter.tc + 1 > N_TC) {
            Error::happened(Error::Module::TC, ERR_INVALID_TC, Error::Severity::CRITICAL);
        }
    }
    inline void checkTC(Channel channel) {
        checkTC(channel.counter);
    }

    void initCounter(Counter counter, SourceClock sourceClock, unsigned long sourceClockFrequency) {
        // Save the config
        _countersConfig[counter.tc][counter.n].sourceClock = sourceClock;
        _countersConfig[counter.tc][counter.n].sourceClockFrequency = sourceClockFrequency;

        // Enable the module clock
        PM::enablePeripheralClock(PM::CLK_TC0 + counter.tc);

        // Enable the divided clock powering the counter
        if (sourceClock == SourceClock::PBA_OVER_2) {
            PM::enablePBADivClock(1); // 2^1 = 2
        } else if (sourceClock == SourceClock::PBA_OVER_8) {
            PM::enablePBADivClock(3); // 2^3 = 8
        } else if (sourceClock == SourceClock::PBA_OVER_32) {
            PM::enablePBADivClock(5); // 2^5 = 32
        } else if (sourceClock == SourceClock::PBA_OVER_128) {
            PM::enablePBADivClock(7); // 2^7 = 128
        }

        // Enable the external input clock pin
        if (sourceClock == SourceClock::CLK0) {
            GPIO::enablePeripheral(PINS_CLK[counter.tc][0]);
        } else if (sourceClock == SourceClock::CLK1) {
            GPIO::enablePeripheral(PINS_CLK[counter.tc][1]);
        } else if (sourceClock == SourceClock::CLK2) {
            GPIO::enablePeripheral(PINS_CLK[counter.tc][2]);
        }
    }


    // Simple counter mode

    void enableSimpleCounter(Counter counter, uint16_t maxValue, SourceClock sourceClock, unsigned long sourceClockFrequency, bool invert, bool upDown) {
        checkTC(counter);
        uint32_t REG = TC_BASE + counter.tc * TC_SIZE + counter.n * OFFSET_COUNTER_SIZE;

        // Initialize the counter and its clock
        initCounter(counter, sourceClock, sourceClockFrequency);

        // WPMR (Write Protect Mode Register) : disable write protect
        (*(volatile uint32_t*)(TC_BASE + counter.tc * TC_SIZE + OFFSET_WPMR))
            = 0 << WPMR_WPEN            // WPEN : write protect disabled
            | UNLOCK_KEY << WPMR_WPKEY; // WPKEY : write protect key

        // CCR (Channel Control Register) : disable the clock
        (*(volatile uint32_t*)(REG + OFFSET_CCR0))
            = 1 << CCR_CLKDIS;       // CLKDIS : disable the clock

        // CMR (Channel Mode Register) : setup the counter in Waveform Generation Mode
        (*(volatile uint32_t*)(REG + OFFSET_CMR0))
            =                                 // TCCLKS : clock selection
              (static_cast<int>(sourceClock) & 0b111) << CMR_TCCLKS
            | invert << CMR_CLKI              // CLKI : clock invert
            | (upDown ? 3 : 2) << CMR_WAVSEL  // WAVSEL : UP or UP/DOWN mode with automatic trigger on RC Compare
            | 1 << CMR_WAVE;                  // WAVE : waveform generation mode

        // Set the max value in RC
        (*(volatile uint32_t*)(REG + OFFSET_RC0)) = maxValue;

        // CCR (Channel Control Register) : enable the clock
        (*(volatile uint32_t*)(REG + OFFSET_CCR0))
            = 1 << CCR_CLKEN;        // CLKEN : enable the clock

        // WPMR (Write Protect Mode Register) : re-enable write protect
        (*(volatile uint32_t*)(TC_BASE + counter.tc * TC_SIZE + OFFSET_WPMR))
            = 1 << WPMR_WPEN            // WPEN : write protect enabled
            | UNLOCK_KEY << WPMR_WPKEY; // WPKEY : write protect key

        // Start the counter
        start(counter);
    }


    // PWM mode

    // Initialize a TC channel and counter in PWM mode with the given period and hightime in microseconds
    bool enablePWM(Channel channel, float period, float highTime, bool output, SourceClock sourceClock, unsigned long sourceClockFrequency) {
        checkTC(channel);
        uint32_t REG = TC_BASE + channel.counter.tc * TC_SIZE + channel.counter.n * OFFSET_COUNTER_SIZE;

        // Initialize the counter and its clock
        initCounter(channel.counter, sourceClock, sourceClockFrequency);

        // WPMR (Write Protect Mode Register) : disable write protect
        (*(volatile uint32_t*)(TC_BASE + channel.counter.tc * TC_SIZE + OFFSET_WPMR))
            = 0 << WPMR_WPEN            // WPEN : write protect disabled
            | UNLOCK_KEY << WPMR_WPKEY; // WPKEY : write protect key

        // CCR (Channel Control Register) : disable the clock
        (*(volatile uint32_t*)(REG + OFFSET_CCR0))
            = 1 << CCR_CLKDIS;       // CLKDIS : disable the clock

        // CMR (Channel Mode Register) : setup the counter in Waveform Generation Mode
        (*(volatile uint32_t*)(REG + OFFSET_CMR0))
            =                   // TCCLKS : clock selection
              (static_cast<int>(sourceClock) & 0b111) << CMR_TCCLKS
            | 0 << CMR_CLKI     // CLKI : disable clock invert
            | 0 << CMR_BURST    // BURST : disable burst mode
            | 0 << CMR_CPCSTOP  // CPCSTOP : clock is not stopped with RC compare
            | 0 << CMR_CPCDIS   // CPCDIS : clock is not disabled with RC compare
            | 1 << CMR_EEVT     // EEVT : external event selection to XC0 (TIOB is therefore an output)
            | 2 << CMR_WAVSEL   // WAVSEL : UP mode with automatic trigger on RC Compare
            | 1 << CMR_WAVE     // WAVE : waveform generation mode
            | 2 << CMR_ACPA     // ACPA : RA/TIOA : clear
            | 1 << CMR_ACPC     // ACPC : RC/TIOA : set
            | 1 << CMR_ASWTRG   // ASWTRG : SoftwareTrigger/TIOA : set
            | 2 << CMR_BCPB     // BCPA : RA/TIOB : clear
            | 1 << CMR_BCPC     // BCPC : RC/TIOB : set
            | 1 << CMR_BSWTRG;  // BSWTRG : SoftwareTrigger/TIOB : set

        // Set the period and high time
        bool isValueValid = true;
        isValueValid = isValueValid && setPeriod(channel.counter, period);
        isValueValid = isValueValid && setHighTime(channel, highTime);

        // CCR (Channel Control Register) : enable and start the clock
        (*(volatile uint32_t*)(REG + OFFSET_CCR0))
            = 1 << CCR_CLKEN;        // CLKEN : enable the clock

        // WPMR (Write Protect Mode Register) : re-enable write protect
        (*(volatile uint32_t*)(TC_BASE + channel.counter.tc * TC_SIZE + OFFSET_WPMR))
            = 1 << WPMR_WPEN            // WPEN : write protect enabled
            | UNLOCK_KEY << WPMR_WPKEY; // WPKEY : write protect key

        // If output is enabled, set the pin in peripheral mode
        if (output) {
            GPIO::enablePeripheral(PINS[channel.counter.tc][N_CHANNELS_PER_COUNTER * channel.counter.n + channel.line]);
        }

        // Start the counter
        start(channel.counter);

        return isValueValid;
    }

    // Set the period in microseconds for both TIOA and TIOB of the specified counter
    bool setPeriod(Counter counter, float period) {
        checkTC(counter);

        uint64_t clockFrequency = sourceClockFrequency(counter);
        if (clockFrequency == 0) {
            return false;
        }
        return setRC(counter, period * clockFrequency / 1000000L);
    }

    // Set the high time of the specified channel in microseconds
    bool setHighTime(Channel channel, float highTime) {
        checkTC(channel);

        uint64_t clockFrequency = sourceClockFrequency(channel.counter);
        if (clockFrequency == 0) {
            return false;
        }
        return setRX(channel, highTime * clockFrequency / 1000000L);
    }

    // Set the duty cycle of the specified channel in percent
    bool setDutyCycle(Channel channel, int percent) {
        checkTC(channel);
        uint32_t REG = TC_BASE + channel.counter.tc * TC_SIZE + channel.counter.n * OFFSET_COUNTER_SIZE;
        uint32_t rc = (*(volatile uint32_t*)(REG + OFFSET_RC0));
        return setRX(channel, rc * percent / 100);
    }

    // Enable the output of the selected channel
    void enableOutput(Channel channel) {
        checkTC(channel);
        GPIO::enablePeripheral(PINS[channel.counter.tc][N_CHANNELS_PER_COUNTER * channel.counter.n + channel.line]);
    }

    // Disable the output of the selected channel
    void disableOutput(Channel channel) {
        checkTC(channel);
        GPIO::disablePeripheral(PINS[channel.counter.tc][N_CHANNELS_PER_COUNTER * channel.counter.n + channel.line]);
    }


    // Measure mode

    void enableMeasure(Counter counter, SourceClock sourceClock, unsigned long sourceClockFrequency) {
        checkTC(counter);
        uint32_t REG = TC_BASE + counter.tc * TC_SIZE + counter.n * OFFSET_COUNTER_SIZE;

        // Initialize the counter and its clock
        initCounter(counter, sourceClock, sourceClockFrequency);

        // WPMR (Write Protect Mode Register) : disable write protect
        (*(volatile uint32_t*)(TC_BASE + counter.tc * TC_SIZE + OFFSET_WPMR))
            = 0 << WPMR_WPEN            // WPEN : write protect disabled
            | UNLOCK_KEY << WPMR_WPKEY; // WPKEY : write protect key

        // CCR (Channel Control Register) : disable the clock
        (*(volatile uint32_t*)(REG + OFFSET_CCR0))
            = 1 << CCR_CLKDIS;       // CLKDIS : disable the clock

        // CMR (Channel Mode Register) : setup the counter in Capture Mode
        (*(volatile uint32_t*)(REG + OFFSET_CMR0))
            =                   // TCCLKS : clock selection
              (static_cast<int>(sourceClock) & 0b111) << CMR_TCCLKS
            | 0 << CMR_CLKI     // CLKI : disable clock invert
            | 0 << CMR_BURST    // BURST : disable burst mode
            | 1 << CMR_LDBSTOP  // LDBSTOP : stop clock after RB load
            | 1 << CMR_LDBDIS   // LDBSTOP : disable clock after RB load
            | 1 << CMR_ETRGEDG  // ETRGEDG : external trigger on rising edge
            | 1 << CMR_ABETRG   // ABETRG : external trigger on TIOA
            | 0 << CMR_CPCTRG   // CPCTRG : RC disabled
            | 0 << CMR_WAVE     // WAVE : capture mode
            | 2 << CMR_LDRA     // LDRA : load RA on falling edge of TIOA
            | 1 << CMR_LDRB;    // LDRB : load RB on rising edge of TIOA

        // WPMR (Write Protect Mode Register) : re-enable write protect
        (*(volatile uint32_t*)(TC_BASE + counter.tc * TC_SIZE + OFFSET_WPMR))
            = 1 << WPMR_WPEN            // WPEN : write protect enabled
            | UNLOCK_KEY << WPMR_WPKEY; // WPKEY : write protect key

        // Enable the input pin for TIOA
        GPIO::enablePeripheral(PINS[counter.tc][N_CHANNELS_PER_COUNTER * counter.n]);
    }

    void measure(Counter counter, bool continuous) {
        checkTC(counter);
        uint32_t REG = TC_BASE + counter.tc * TC_SIZE + counter.n * OFFSET_COUNTER_SIZE;

        // CCR (Channel Control Register) : disable the clock
        (*(volatile uint32_t*)(REG + OFFSET_CCR0))
            = 1 << CCR_CLKDIS;       // CLKDIS : disable the clock

        // WPMR (Write Protect Mode Register) : disable write protect
        (*(volatile uint32_t*)(TC_BASE + counter.tc * TC_SIZE + OFFSET_WPMR))
            = 0 << WPMR_WPEN            // WPEN : write protect disabled
            | UNLOCK_KEY << WPMR_WPKEY; // WPKEY : write protect key

        // CMR (Channel Mode Register) : in one-shot mode, configure the TC to 
        // disable the clock after a measure
        if (!continuous) {
            (*(volatile uint32_t*)(REG + OFFSET_CMR0))
                |= 1 << CMR_LDBSTOP    // LDBSTOP : stop clock after RB load
                |  1 << CMR_LDBDIS;    // LDBSTOP : disable clock after RB load
        } else {
            (*(volatile uint32_t*)(REG + OFFSET_CMR0))
                &= ~(uint32_t)(
                      1 << CMR_LDBSTOP // LDBSTOP : do not stop clock after RB load
                    | 1 << CMR_LDBDIS  // LDBSTOP : do not disable clock after RB load
                );
        }

        // WPMR (Write Protect Mode Register) : re-enable write protect
        (*(volatile uint32_t*)(TC_BASE + counter.tc * TC_SIZE + OFFSET_WPMR))
            = 1 << WPMR_WPEN            // WPEN : write protect enabled
            | UNLOCK_KEY << WPMR_WPKEY; // WPKEY : write protect key

        // CCR (Channel Control Register) : enable the clock
        (*(volatile uint32_t*)(REG + OFFSET_CCR0))
            = 1 << CCR_CLKEN;        // CLKEN : enable the clock
    }

    uint16_t measuredPeriodRaw(Counter counter) {
        return rbValue(counter);
    }

    unsigned long measuredPeriod(Counter counter) {
        unsigned long clockFrequency = sourceClockFrequency(counter);
        if (clockFrequency == 0) {
            return false;
        }
        return (uint64_t)measuredPeriodRaw(counter) * 1000000L / clockFrequency;
    }

    uint16_t measuredHighTimeRaw(Counter counter) {
        return raValue(counter);
    }

    unsigned long measuredHighTime(Counter counter) {
        unsigned long clockFrequency = sourceClockFrequency(counter);
        if (clockFrequency == 0) {
            return false;
        }
        return (uint64_t)measuredHighTimeRaw(counter) * 1000000L / clockFrequency;
    }

    unsigned int measuredDutyCycle(Counter counter) {
        return measuredHighTimeRaw(counter) * 100 / measuredPeriodRaw(counter);
    }

    bool isMeasureOverflow(Counter counter) {
        uint32_t REG = TC_BASE + counter.tc * TC_SIZE + counter.n * OFFSET_COUNTER_SIZE;
        return ((*(volatile uint32_t*)(REG + OFFSET_SR0)) >> SR_COVFS) & 1;
    }


    // Low-level counter functions

    // Set the RA or RB register of the given channel
    bool setRX(Channel channel, unsigned int rx) {
        checkTC(channel);
        uint32_t REG = TC_BASE + channel.counter.tc * TC_SIZE + channel.counter.n * OFFSET_COUNTER_SIZE;

        // Clip value
        bool isValueValid = true;
        if (rx > 0xFFFF) {
            rx = 0xFFFF;
            isValueValid = false;
        }

        // WPMR (Write Protect Mode Register) : disable write protect
        (*(volatile uint32_t*)(TC_BASE + channel.counter.tc * TC_SIZE + OFFSET_WPMR))
            = 0 << WPMR_WPEN            // WPEN : write protect enabled
            | UNLOCK_KEY << WPMR_WPKEY; // WPKEY : write protect key

        // If the counter compare register (RA or RB) is zero, the output will be set by the RC compare
        // (CMR0.ACPC or CMR0.BCPC) but not immediately cleared by the RA/RB compare, and the output will
        // stay high instead of staying low. To match the expected behaviour the CMR register need to be
        // temporarily reconfigured to clear the output on RC compare.
        // When quitting this edge case (current RA or RB is 0), the default behaviour must be reset.
        // Depending on the case, the RA/RB value must be set either before of after configuring CMR.
        if (rx == 0) {
            // CMR (Channel Mode Register) : set RC compare over TIOA to 2
            uint32_t cmr = (*(volatile uint32_t*)(REG + OFFSET_CMR0));
            cmr &= ~(uint32_t)(0b11 << (channel.line == TIOB ? CMR_BCPC : CMR_ACPC));
            cmr |= 2 << (channel.line == TIOB ? CMR_BCPC : CMR_ACPC);
            (*(volatile uint32_t*)(REG + OFFSET_CMR0)) = cmr;

            // Set the signal high time *after* configuring CMR
            (*(volatile uint32_t*)(REG + (channel.line == TIOB ? OFFSET_RB0 : OFFSET_RA0))) = rx;
            
        } else if ((*(volatile uint32_t*)(REG + (channel.line == TIOB ? OFFSET_RB0 : OFFSET_RA0))) == 0) {
            // Set the signal high time *before* configuring CMR
            (*(volatile uint32_t*)(REG + (channel.line == TIOB ? OFFSET_RB0 : OFFSET_RA0))) = rx;

            // CMR (Channel Mode Register) : set RC compare over TIOA to 2
            uint32_t cmr = (*(volatile uint32_t*)(REG + OFFSET_CMR0));
            cmr &= ~(uint32_t)(0b11 << (channel.line == TIOB ? CMR_BCPC : CMR_ACPC));
            cmr |= 1 << (channel.line == TIOB ? CMR_BCPC : CMR_ACPC);
            (*(volatile uint32_t*)(REG + OFFSET_CMR0)) = cmr;

        } else {
            // Set the signal high time
            (*(volatile uint32_t*)(REG + (channel.line == TIOB ? OFFSET_RB0 : OFFSET_RA0))) = rx;
        }

        // WPMR (Write Protect Mode Register) : re-enable write protect
        (*(volatile uint32_t*)(TC_BASE + channel.counter.tc * TC_SIZE + OFFSET_WPMR))
            = 1 << WPMR_WPEN            // WPEN : write protect enabled
            | UNLOCK_KEY << WPMR_WPKEY; // WPKEY : write protect key

        return isValueValid;
    }

    // Set the RC register of the given counter
    bool setRC(Counter counter, unsigned int rc) {
        checkTC(counter);
        uint32_t REG = TC_BASE + counter.tc * TC_SIZE + counter.n * OFFSET_COUNTER_SIZE;

        // Clip value
        bool isValueValid = true;
        if (rc > 0xFFFF) {
            rc = 0xFFFF;
            isValueValid = false;
        }

        // WPMR (Write Protect Mode Register) : disable write protect
        (*(volatile uint32_t*)(TC_BASE + counter.tc * TC_SIZE + OFFSET_WPMR))
            = 0 << WPMR_WPEN            // WPEN : write protect enabled
            | UNLOCK_KEY << WPMR_WPKEY; // WPKEY : write protect key

        // Set the signal period
        (*(volatile uint32_t*)(REG + OFFSET_RC0)) = rc;

        // WPMR (Write Protect Mode Register) : re-enable write protect
        (*(volatile uint32_t*)(TC_BASE + counter.tc * TC_SIZE + OFFSET_WPMR))
            = 1 << WPMR_WPEN            // WPEN : write protect enabled
            | UNLOCK_KEY << WPMR_WPKEY; // WPKEY : write protect key

        return isValueValid;
    }

    // Get the value of the given counter
    uint16_t counterValue(Counter counter) {
        checkTC(counter);
        return (*(volatile uint32_t*)(TC_BASE + counter.tc * TC_SIZE + counter.n * OFFSET_COUNTER_SIZE + OFFSET_CV0));
    }

    // Get the value of the RA register for the given counter
    uint16_t raValue(Counter counter) {
        checkTC(counter);
        return (*(volatile uint32_t*)(TC_BASE + counter.tc * TC_SIZE + counter.n * OFFSET_COUNTER_SIZE + OFFSET_RA0));
    }

    // Get the value of the RB register for the given counter
    uint16_t rbValue(Counter counter) {
        checkTC(counter);
        return (*(volatile uint32_t*)(TC_BASE + counter.tc * TC_SIZE + counter.n * OFFSET_COUNTER_SIZE + OFFSET_RB0));
    }

    // Get the value of the RC register for the given counter
    uint16_t rcValue(Counter counter) {
        checkTC(counter);
        return (*(volatile uint32_t*)(TC_BASE + counter.tc * TC_SIZE + counter.n * OFFSET_COUNTER_SIZE + OFFSET_RC0));
    }

    unsigned long sourceClockFrequency(Counter counter) {
        switch (_countersConfig[counter.tc][counter.n].sourceClock) {
            case SourceClock::GENERIC_CLOCK:
            case SourceClock::CLK0:
            case SourceClock::CLK1:
            case SourceClock::CLK2:
                return _countersConfig[counter.tc][counter.n].sourceClockFrequency;

            case SourceClock::PBA_OVER_2:
                return PM::getModuleClockFrequency(PM::CLK_TC0 + counter.tc) / 2;

            case SourceClock::PBA_OVER_8:
                return PM::getModuleClockFrequency(PM::CLK_TC0 + counter.tc) / 8;

            case SourceClock::PBA_OVER_32:
                return PM::getModuleClockFrequency(PM::CLK_TC0 + counter.tc) / 32;

            case SourceClock::PBA_OVER_128:
                return PM::getModuleClockFrequency(PM::CLK_TC0 + counter.tc) / 128;
        }
        return 0;
    }

    // Wait for the specified delay
    void wait(Counter counter, unsigned long delay, Unit unit, SourceClock sourceClock, unsigned long sourceClockFrequency) {
        checkTC(counter);
        uint32_t REG = TC_BASE + counter.tc * TC_SIZE + counter.n * OFFSET_COUNTER_SIZE;

        // Initialize the counter and its clock
        initCounter(counter, sourceClock, sourceClockFrequency);

        // Compute timing
        if (unit == Unit::MILLISECONDS) {
            delay *= 1000;
        }
        unsigned int basePeriod = 80000000L / PM::getModuleClockFrequency(PM::CLK_TC0 + counter.tc);
        delay = delay * 10 / basePeriod;
        unsigned int repeat = delay / 0x10000; // Max counter value
        unsigned int rest = delay % 0x10000;

        // WPMR (Write Protect Mode Register) : disable write protect
        (*(volatile uint32_t*)(TC_BASE + counter.tc * TC_SIZE + OFFSET_WPMR)) = 0 << WPMR_WPEN | UNLOCK_KEY << WPMR_WPKEY;

        for (unsigned int i = 0; i <= repeat; i++) {
            // Set the period length
            (*(volatile uint32_t*)(REG + OFFSET_RC0)) = (i == repeat ? rest : 0xFFFF);

            // Software trigger
            (*(volatile uint32_t*)(REG + OFFSET_CCR0)) = 1 << CCR_SWTRG;

            // Wait for RC value to be reached
            while (!((*(volatile uint32_t*)(REG + OFFSET_SR0)) & (1 << SR_CPCS)));
        }

        // WPMR (Write Protect Mode Register) : re-enable write protect
        (*(volatile uint32_t*)(TC_BASE + counter.tc * TC_SIZE + OFFSET_WPMR)) = 1 << WPMR_WPEN | UNLOCK_KEY << WPMR_WPKEY;
    }

    // Call the given handler after the specified delay
    void execDelayed(Counter counter, void (*handler)(), unsigned long delay, bool repeat, Unit unit, SourceClock sourceClock, unsigned long sourceClockFrequency) {
        checkTC(counter);
        uint32_t REG = TC_BASE + counter.tc * TC_SIZE + counter.n * OFFSET_COUNTER_SIZE;

        // Initialize the counter and its clock
        initCounter(counter, sourceClock, sourceClockFrequency);

        // Stop the timer
        (*(volatile uint32_t*)(REG + OFFSET_CCR0)) = 1 << CCR_CLKDIS;

        // Set the handler
        ExecDelayedData& data = execDelayedData[counter.tc][counter.n];
        data.handler = (uint32_t)handler;

        // Compute timings
        // If the requested delay is longer than a full period of the counter, compute and save the number
        // of periods to skip before calling the user handler
        if (unit == Unit::MILLISECONDS) {
            delay *= 1000;
        }
        unsigned int basePeriod = 80000000L / PM::getModuleClockFrequency(PM::CLK_TC0 + counter.tc);
        unsigned long value = delay * 10 / basePeriod;
        unsigned int skipPeriods = value / 0x10000; // 0x10000 is the max counter value
        unsigned int rest = value % 0x10000;
        data.skipPeriods = data.skipPeriodsReset = skipPeriods;
        data.rest = data.restReset = (skipPeriods > 0 ? rest : 0);
        data.repeat = repeat;
        (*(volatile uint32_t*)(TC_BASE + counter.tc * TC_SIZE + OFFSET_WPMR)) = 0 << WPMR_WPEN | UNLOCK_KEY << WPMR_WPKEY;
        (*(volatile uint32_t*)(REG + OFFSET_RC0)) = (skipPeriods > 0 ? 0xFFFF : rest);
        (*(volatile uint32_t*)(TC_BASE + counter.tc * TC_SIZE + OFFSET_WPMR)) = 1 << WPMR_WPEN | UNLOCK_KEY << WPMR_WPKEY;

        // Enable the interrupt at the core level
        Core::Interrupt interrupt = static_cast<Core::Interrupt>(static_cast<int>(Core::Interrupt::TC00) + counter.tc * N_COUNTERS_PER_TC + counter.n);
        Core::setInterruptHandler(interrupt, &execDelayedHandlerWrapper);
        Core::enableInterrupt(interrupt, INTERRUPT_PRIORITY);

        // IER (Interrupt Enable Register) : enable the CPCS (RC value reached) interrupt
        (*(volatile uint32_t*)(REG + OFFSET_IER0)) = 1 << SR_CPCS;

        // Start the timer
        (*(volatile uint32_t*)(REG + OFFSET_CCR0)) = 1 << CCR_CLKEN;
        (*(volatile uint32_t*)(REG + OFFSET_CCR0)) = 1 << CCR_SWTRG;
    }

    void execDelayedHandlerWrapper() {
        // Get the channel which generated the interrupt
        int interrupt = static_cast<int>(Core::currentInterrupt()) - static_cast<int>(Core::Interrupt::TC00);
        int tc = interrupt / N_COUNTERS_PER_TC;
        int counter = interrupt % N_COUNTERS_PER_TC;
        uint32_t REG = TC_BASE + tc * TC_SIZE + counter * OFFSET_COUNTER_SIZE;
        ExecDelayedData& data = execDelayedData[tc][counter];

        (*(volatile uint32_t*)(REG + OFFSET_SR0));

        // Decrease the counters
        if (data.skipPeriods > 0) {
            // If there are still periods to skip, decrease the periods counter
            data.skipPeriods--;

        } else if (data.rest > 0) {
            // Otherwise, if rest > 0, this is the last period : configure the counter with the remaining time
            (*(volatile uint32_t*)(TC_BASE + tc * TC_SIZE + OFFSET_WPMR)) = 0 << WPMR_WPEN | UNLOCK_KEY << WPMR_WPKEY;
            (*(volatile uint32_t*)(REG + OFFSET_RC0)) = data.rest;
            (*(volatile uint32_t*)(TC_BASE + tc * TC_SIZE + OFFSET_WPMR)) = 1 << WPMR_WPEN | UNLOCK_KEY << WPMR_WPKEY;
            (*(volatile uint32_t*)(REG + OFFSET_CCR0)) = 1 << CCR_SWTRG;
            data.rest = 0; // There will be no time remaining after this

        } else {
            // Otherwise, if skipPeriods == 0 and rest == 0, the time has expired

            // Call the user handler
            void (*handler)() = (void (*)())data.handler;
            if (handler) {
                handler();
            }

            // Repeat
            if (data.repeat) {
                // Reset the data structure with their initial value
                data.skipPeriods = data.skipPeriodsReset;
                data.rest = data.restReset;
                (*(volatile uint32_t*)(TC_BASE + tc * TC_SIZE + OFFSET_WPMR)) = 0 << WPMR_WPEN | UNLOCK_KEY << WPMR_WPKEY;
                (*(volatile uint32_t*)(REG + OFFSET_RC0)) = (data.skipPeriods > 0 ? 0xFFFF : data.rest);
                (*(volatile uint32_t*)(TC_BASE + tc * TC_SIZE + OFFSET_WPMR)) = 1 << WPMR_WPEN | UNLOCK_KEY << WPMR_WPKEY;
                (*(volatile uint32_t*)(REG + OFFSET_CCR0)) = 1 << CCR_SWTRG;

            } else {
                // Disable the interrupt
                (*(volatile uint32_t*)(REG + OFFSET_IDR0)) = 1 << SR_CPCS;
            }
        }
    }

    // Start the counter and reset its value by issuing a software trigger
    void start(Counter counter) {
        checkTC(counter);
        uint32_t REG = TC_BASE + counter.tc * TC_SIZE + counter.n * OFFSET_COUNTER_SIZE;

        // CCR (Channel Control Register) : issue a software trigger
        (*(volatile uint32_t*)(REG + OFFSET_CCR0))
            = 1 << CCR_SWTRG;        // SWTRG : software trigger
    }

    // Stop the clock of the given counter and freeze its value.
    // If the output is currently high, it will stay that way. Use disableOutput() if necessary.
    void stop(Counter counter) {
        checkTC(counter);
        uint32_t REG = TC_BASE + counter.tc * TC_SIZE + counter.n * OFFSET_COUNTER_SIZE;

        // CCR (Channel Control Register) : disable and reenable the clock to stop it
        (*(volatile uint32_t*)(REG + OFFSET_CCR0))
            = 1 << CCR_CLKDIS;       // CLKDIS : disable the clock
        (*(volatile uint32_t*)(REG + OFFSET_CCR0))
            = 1 << CCR_CLKEN;       // CLKEN : enable the clock
    }

    // Start all the enabled counters simultaneously
    void sync() {
        // BCR (Block Control Register) : issue a sync command
        (*(volatile uint32_t*)(TC_BASE + OFFSET_BCR))
            = 1 << BCR_SYNC;        // SYNC : synch command
    }

    void setPin(Channel channel, PinFunction function, GPIO::Pin pin) {
        checkTC(channel);

        switch (function) {
            case PinFunction::OUT:
                PINS[channel.counter.tc][N_CHANNELS_PER_COUNTER * channel.counter.n + channel.line] = pin;
                break;

            case PinFunction::CLK:
                PINS_CLK[channel.counter.tc][N_CHANNELS_PER_COUNTER * channel.counter.n + channel.line] = pin;
                break;
        }
    }


}

