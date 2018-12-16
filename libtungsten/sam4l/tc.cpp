#include "tc.h"
#include "core.h"
#include "pm.h"
#include "scif.h"

namespace TC {

    // Package-dependant, defined in pins_sam4l_XX.cpp
    extern struct GPIO::Pin PINS[MAX_N_TC][N_CHANNELS * N_LINES];
    extern struct GPIO::Pin PINS_CLK[MAX_N_TC][N_CHANNELS * N_LINES];

    // Internal list of delayed callbacks to execute
    extern uint8_t INTERRUPT_PRIORITY;
    struct ExecDelayedData {
        uint32_t handler;
        int counter;
        int counterReset;
        int rest;
        int restReset;
        bool repeat;
    };
    ExecDelayedData execDelayedData[MAX_N_TC][N_CHANNELS];

    void execDelayedHandlerWrapper();

    // Initialize a TC channel with the given period and hightime in microseconds
    void init(const Channel& channel, unsigned int period, unsigned int highTime, bool output) {
        uint32_t REG = TC_BASE + channel.tc * TC_SIZE + channel.subchannel * OFFSET_CHANNEL_SIZE;

        // Enable the module clock
        PM::enablePeripheralClock(PM::CLK_TC0 + channel.tc);

        // Enable the divided clock powering the counter (PBA Clock / 8)
        PM::enablePBADivClock(2);

        // WPMR (Write Protect Mode Register) : disable write protect
        (*(volatile uint32_t*)(TC_BASE + channel.tc * TC_SIZE + OFFSET_WPMR))
            = 0 << WPMR_WPEN            // WPEN : write protect disabled
            | UNLOCK_KEY << WPMR_WPKEY; // WPKEY : write protect key

        // CCR (Channel Control Register) : disable the clock
        (*(volatile uint32_t*)(REG + OFFSET_CCR0))
            = 1 << CCR_CLKDIS;       // CLKDIS : disable the clock

        // CMR (Channel Mode Register) : setup the channel in Waveform Generation Mode
        (*(volatile uint32_t*)(REG + OFFSET_CMR0))
            = 2 << CMR_TCCLKS       // TCCLKS : clock selection to PBA Clock / 8
            | 0 << CMR_CLKI         // CLKI : disable clock invert
            | 0 << CMR_BURST        // BURST : disable burst mode
            | 0 << CMR_CPCSTOP      // CPCSTOP : clock is not stopped with RC compare
            | 0 << CMR_CPCDIS       // CPCDIS : clock is not disabled with RC compare
            | 1 << CMR_EEVT         // EEVT : external event selection to XC0 (TIOB is therefore an output)
            | 2 << CMR_WAVSEL       // WAVSEL : UP mode with automatic trigger on RC Compare
            | 1 << CMR_WAVE         // WAVE : waveform generation mode
            | 2 << CMR_ACPA         // ACPA : RA/TIOA : clear
            | 1 << CMR_ACPC         // ACPC : RC/TIOA : set
            | 1 << CMR_ASWTRG       // ASWTRG : SoftwareTrigger/TIOA : set
            | 2 << CMR_BCPB         // BCPA : RA/TIOB : clear
            | 1 << CMR_BCPC         // BCPC : RC/TIOB : set
            | 1 << CMR_BSWTRG;      // BSWTRG : SoftwareTrigger/TIOB : set

        // Set the period and high time
        setPeriod(channel, period);
        setHighTime(channel, highTime);

        // WPMR (Write Protect Mode Register) : disable write protect
        (*(volatile uint32_t*)(TC_BASE + channel.tc * TC_SIZE + OFFSET_WPMR))
            = 0 << WPMR_WPEN            // WPEN : write protect disabled
            | UNLOCK_KEY << WPMR_WPKEY; // WPKEY : write protect key

        // CCR (Channel Control Register) : enable and start the clock
        (*(volatile uint32_t*)(REG + OFFSET_CCR0))
            = 1 << CCR_CLKEN;        // CLKEN : enable the clock
        (*(volatile uint32_t*)(REG + OFFSET_CCR0))
            = 1 << CCR_SWTRG;        // SWTRG : software trigger

        // WPMR (Write Protect Mode Register) : re-enable write protect
        (*(volatile uint32_t*)(TC_BASE + channel.tc * TC_SIZE + OFFSET_WPMR))
            = 1 << WPMR_WPEN            // WPEN : write protect enabled
            | UNLOCK_KEY << WPMR_WPKEY; // WPKEY : write protect key

        // Set the pin in peripheral mode
        if (output) {
            GPIO::enablePeripheral(PINS[channel.tc][N_LINES * channel.subchannel + channel.line]);
        }
    }

    // Set the period in microseconds for both TIOA and TIOB of the specified channel
    void setPeriod(const Channel& channel, double period) {
        unsigned int basePeriod = 80000000L / PM::getModuleClockFrequency(PM::CLK_TC0 + channel.tc);
        setRC(channel, period * 10 / basePeriod);
    }

    // Set the high time of the specified channel in microseconds
    void setHighTime(const Channel& channel, double highTime) {
        unsigned int basePeriod = 80000000L / PM::getModuleClockFrequency(PM::CLK_TC0 + channel.tc);
        setRX(channel, highTime * 10 / basePeriod);
    }

    // Set the duty cycle of the specified channel in percent
    void setDutyCycle(const Channel& channel, int percent) {
        uint32_t REG = TC_BASE + channel.tc * TC_SIZE + channel.subchannel * OFFSET_CHANNEL_SIZE;
        uint32_t rc = (*(volatile uint32_t*)(REG + OFFSET_RC0));
        setRX(channel, rc * percent / 100);
    }

    // Enable the output of the selected channel
    void enableOutput(const Channel& channel) {
        GPIO::enablePeripheral(PINS[channel.tc][N_LINES * channel.subchannel + channel.line]);
    }

    // Disable the output of the selected channel
    void disableOutput(const Channel& channel) {
        GPIO::disablePeripheral(PINS[channel.tc][N_LINES * channel.subchannel + channel.line]);
    }

    // Set the RC register which defines the period for both TIOA and TIOB of the given channel
    void setRC(const Channel& channel, uint16_t rc) {
        uint32_t REG = TC_BASE + channel.tc * TC_SIZE + channel.subchannel * OFFSET_CHANNEL_SIZE;

        // WPMR (Write Protect Mode Register) : disable write protect
        (*(volatile uint32_t*)(TC_BASE + channel.tc * TC_SIZE + OFFSET_WPMR))
            = 0 << WPMR_WPEN            // WPEN : write protect enabled
            | UNLOCK_KEY << WPMR_WPKEY; // WPKEY : write protect key

        // Set the signal period
        (*(volatile uint32_t*)(REG + OFFSET_RC0)) = rc;

        // Software trigger
        (*(volatile uint32_t*)(REG + OFFSET_CCR0)) = 1 << CCR_SWTRG;

        // WPMR (Write Protect Mode Register) : re-enable write protect
        (*(volatile uint32_t*)(TC_BASE + channel.tc * TC_SIZE + OFFSET_WPMR))
            = 1 << WPMR_WPEN            // WPEN : write protect enabled
            | UNLOCK_KEY << WPMR_WPKEY; // WPKEY : write protect key
    }

    // Set the RA or RB register of the specified channel which defines the time at high state
    void setRX(const Channel& channel, uint16_t rx) {
        uint32_t REG = TC_BASE + channel.tc * TC_SIZE + channel.subchannel * OFFSET_CHANNEL_SIZE;

        // WPMR (Write Protect Mode Register) : disable write protect
        (*(volatile uint32_t*)(TC_BASE + channel.tc * TC_SIZE + OFFSET_WPMR))
            = 0 << WPMR_WPEN            // WPEN : write protect enabled
            | UNLOCK_KEY << WPMR_WPKEY; // WPKEY : write protect key

        // If the channel compare register (RA or RB) is zero, the output will be set by the RC compare
        // (CMR0.ACPC or CMR0.BCPC) but not immediately cleared by the RA/RB compare, and the line will
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

        // Software trigger
        //(*(volatile uint32_t*)(REG + OFFSET_CCR0)) = 1 << CCR_SWTRG;

        // WPMR (Write Protect Mode Register) : re-enable write protect
        (*(volatile uint32_t*)(TC_BASE + channel.tc * TC_SIZE + OFFSET_WPMR))
            = 1 << WPMR_WPEN            // WPEN : write protect enabled
            | UNLOCK_KEY << WPMR_WPKEY; // WPKEY : write protect key
    }

    uint32_t getCounterValue(const Channel& channel) {
        // Return the current counter value
        return (*(volatile uint32_t*)(TC_BASE + channel.tc * TC_SIZE + channel.subchannel * OFFSET_CHANNEL_SIZE + OFFSET_CV0));
    }

    void wait(const Channel& channel, unsigned long delay, Unit unit) {
        uint32_t REG = TC_BASE + channel.tc * TC_SIZE + channel.subchannel * OFFSET_CHANNEL_SIZE;

        // Compute timing
        if (unit == Unit::MILLISECONDS) {
            delay *= 1000;
        }
        unsigned int basePeriod = 80000000L / PM::getModuleClockFrequency(PM::CLK_TC0 + channel.tc);
        delay = delay * 10 / basePeriod;
        unsigned int repeat = delay / 0x10000; // Max counter value
        unsigned int rest = delay % 0x10000;

        // WPMR (Write Protect Mode Register) : disable write protect
        (*(volatile uint32_t*)(TC_BASE + channel.tc * TC_SIZE + OFFSET_WPMR)) = 0 << WPMR_WPEN | UNLOCK_KEY << WPMR_WPKEY;

        for (unsigned int i = 0; i <= repeat; i++) {
            // Set the period length
            (*(volatile uint32_t*)(REG + OFFSET_RC0)) = (i == repeat ? rest : 0xFFFF);

            // Software trigger
            (*(volatile uint32_t*)(REG + OFFSET_CCR0)) = 1 << CCR_SWTRG;

            // Wait for RC value to be reached
            while (!((*(volatile uint32_t*)(REG + OFFSET_SR0)) & (1 << SR_CPCS)));
        }

        // WPMR (Write Protect Mode Register) : re-enable write protect
        (*(volatile uint32_t*)(TC_BASE + channel.tc * TC_SIZE + OFFSET_WPMR)) = 1 << WPMR_WPEN | UNLOCK_KEY << WPMR_WPKEY;
    }

    void execDelayed(const Channel& channel, void (*handler)(), unsigned long delay, bool repeat, Unit unit) {
        uint32_t REG = TC_BASE + channel.tc * TC_SIZE + channel.subchannel * OFFSET_CHANNEL_SIZE;

        // Stop the timer
        (*(volatile uint32_t*)(REG + OFFSET_CCR0)) = 1 << CCR_CLKDIS;

        // Set the handler
        ExecDelayedData& data = execDelayedData[channel.tc][channel.subchannel];
        data.handler = (uint32_t)handler;

        // Compute timings
        if (unit == Unit::MILLISECONDS) {
            delay *= 1000;
        }
        unsigned int basePeriod = 80000000L / PM::getModuleClockFrequency(PM::CLK_TC0 + channel.tc);
        unsigned long value = delay * 10 / basePeriod;
        unsigned int counter = value / 0x10000; // Max counter value
        unsigned int rest = value % 0x10000;
        data.counter = counter;
        data.counterReset = data.counter;
        data.rest = (counter > 0 ? rest : 0);
        data.restReset = data.rest;
        data.repeat = repeat;
        (*(volatile uint32_t*)(TC_BASE + channel.tc * TC_SIZE + OFFSET_WPMR)) = 0 << WPMR_WPEN | UNLOCK_KEY << WPMR_WPKEY;
        (*(volatile uint32_t*)(REG + OFFSET_RC0)) = (counter > 0 ? 0xFFFF : rest);
        (*(volatile uint32_t*)(TC_BASE + channel.tc * TC_SIZE + OFFSET_WPMR)) = 1 << WPMR_WPEN | UNLOCK_KEY << WPMR_WPKEY;

        // Enable the interrupt at the core level
        Core::Interrupt interrupt = static_cast<Core::Interrupt>(static_cast<int>(Core::Interrupt::TC00) + channel.tc * N_CHANNELS + channel.subchannel);
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
        int tc = interrupt / N_CHANNELS;
        int channel = interrupt % N_CHANNELS;
        uint32_t REG = TC_BASE + tc * TC_SIZE + channel * OFFSET_CHANNEL_SIZE;
        ExecDelayedData& data = execDelayedData[tc][channel];

        (*(volatile uint32_t*)(REG + OFFSET_SR0));

        // Decrease the counters
        if (data.counter > 0) {
            data.counter--;

        } else if (data.rest > 0) {
            (*(volatile uint32_t*)(TC_BASE + tc * TC_SIZE + OFFSET_WPMR)) = 0 << WPMR_WPEN | UNLOCK_KEY << WPMR_WPKEY;
            (*(volatile uint32_t*)(REG + OFFSET_RC0)) = data.rest;
            (*(volatile uint32_t*)(TC_BASE + tc * TC_SIZE + OFFSET_WPMR)) = 1 << WPMR_WPEN | UNLOCK_KEY << WPMR_WPKEY;
            (*(volatile uint32_t*)(REG + OFFSET_CCR0)) = 1 << CCR_SWTRG;
            data.rest = 0;

        } else {
            // Call the user handler
            void (*handler)() = (void (*)())data.handler;
            handler();

            // Repeat
            if (data.repeat) {
                data.counter = data.counterReset;
                data.rest = data.restReset;
                (*(volatile uint32_t*)(TC_BASE + tc * TC_SIZE + OFFSET_WPMR)) = 0 << WPMR_WPEN | UNLOCK_KEY << WPMR_WPKEY;
                (*(volatile uint32_t*)(REG + OFFSET_RC0)) = (data.counter > 0 ? 0xFFFF : data.rest);
                (*(volatile uint32_t*)(TC_BASE + tc * TC_SIZE + OFFSET_WPMR)) = 1 << WPMR_WPEN | UNLOCK_KEY << WPMR_WPKEY;
                (*(volatile uint32_t*)(REG + OFFSET_CCR0)) = 1 << CCR_SWTRG;

            } else {
                (*(volatile uint32_t*)(REG + OFFSET_IDR0)) = 1 << SR_CPCS;
            }
        }
    }

    void setPin(const Channel& channel, PinFunction function, GPIO::Pin pin) {
        switch (function) {
            case PinFunction::OUT:
                PINS[channel.tc][N_LINES * channel.subchannel + channel.line] = pin;
                break;

            case PinFunction::CLK:
                PINS_CLK[channel.tc][N_LINES * channel.subchannel + channel.line] = pin;
                break;
        }
    }


}

