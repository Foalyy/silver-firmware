#include "scif.h"
#include "bscif.h"
#include "error.h"

namespace SCIF {

    // Package-dependant, defined in pins_sam4l_XX.cpp
    extern struct GPIO::Pin PINS_GCLK[];
    extern struct GPIO::Pin PINS_GCLK_IN[];

    // Clocks frequencies
    unsigned long _rcsysFrequency = 115000UL;
    unsigned long _osc0Frequency = 0;
    unsigned long _pllFrequency = 0;
    unsigned long _dfllFrequency = 0;
    unsigned long _rcfastFrequency = 0;
    unsigned long _rc80mFrequency = 80000000UL;

    // Output mask of the generic clocks
    bool _genericClockOutputEnabled[4] = {false, false, false, false};


    // RCSYS frequency is fixed
    unsigned long getRCSYSFrequency() {
        return _rcsysFrequency;
    }


    // RCFAST

    void enableRCFAST(RCFASTFrequency frequency) {
        // If the RCFAST is already enabled, disable it first
        if (_rcfastFrequency > 0) {
            disableRCFAST();
        }

        // Save the frequency for future use
        switch (frequency) {
            case RCFASTFrequency::RCFAST_4MHZ:
                _rcfastFrequency = 4000000UL;
                break;

            case RCFASTFrequency::RCFAST_8MHZ:
                _rcfastFrequency = 8000000UL;
                break;

            case RCFASTFrequency::RCFAST_12MHZ:
                _rcfastFrequency = 12000000UL;
                break;

            default:
                return;
        }

        // Unlock the RCFASTCFG register, which is locked by default as a safety mesure
        (*(volatile uint32_t*)(SCIF_BASE + OFFSET_UNLOCK))
                = UNLOCK_KEY            // KEY : Magic word (see datasheet)
                | OFFSET_RCFASTCFG;     // ADDR : unlock RCFASTCFG

        // Configure RCFAST
        (*(volatile uint32_t*)(SCIF_BASE + OFFSET_RCFASTCFG))
                = 1 << RCFASTCFG_EN                                // EN : enable the oscillator
                | 1 << RCFASTCFG_TUNEEN                            // TUNEEN : enable the tuner (closed-loop mode)
                | 0 << RCFASTCFG_JITMODE                           // JITMODE : update trim value when lock lost
                | 5 << RCFASTCFG_NBPERIODS                         // NBPERIODS : number of 32kHz periods (max : 7)
                | static_cast<int>(frequency) << RCFASTCFG_FRANGE  // FRANGE : desired frequency
                | 5 << RCFASTCFG_LOCKMARGIN;                       // LOCKMARGIN : error tolerance of the tuner

        // Wait for RCFAST to be ready
        while (!((*(volatile uint32_t*)(SCIF_BASE + OFFSET_RCFASTCFG)) & (1 << RCFASTCFG_EN)));
    }

    void disableRCFAST() {
        // Reset the saved frequency
        _rcfastFrequency = 0;

        // Unlock the RCFASTCFG register, which is locked by default as a safety mesure
        (*(volatile uint32_t*)(SCIF_BASE + OFFSET_UNLOCK))
                = UNLOCK_KEY            // KEY : Magic word (see datasheet)
                | OFFSET_RCFASTCFG;     // ADDR : unlock RCFASTCFG

        // Disable RCFAST
        (*(volatile uint32_t*)(SCIF_BASE + OFFSET_RCFASTCFG)) = 0;
    }

    unsigned long getRCFASTFrequency() {
        return _rcfastFrequency;
    }


    // OSC0

    void enableOSC0(unsigned long frequency) {
        // Unlock the OSCCTRL0 register, which is locked by default as a safety mesure
        (*(volatile uint32_t*)(SCIF_BASE + OFFSET_UNLOCK))
                = UNLOCK_KEY            // KEY : Magic word (see datasheet)
                | OFFSET_OSCCTRL0;      // ADDR : unlock OSCCTRL0

        // Save the indicated frequency for future use
        _osc0Frequency = frequency;

        // Configure OSC0
        (*(volatile uint32_t*)(SCIF_BASE + OFFSET_OSCCTRL0))
                = 1 << OSCCTRL0_MODE     // MODE : crystal
                | 3 << OSCCTRL0_GAIN     // GAIN : G3 (8MHz to 16MHz)
                | 3 << OSCCTRL0_STARTUP  // STARTUP : ~18ms
                | 1 << OSCCTRL0_OSCEN;   // OSCEN : enabled

        // Wait for OSC0 to be ready
        while (!((*(volatile uint32_t*)(SCIF_BASE + OFFSET_PCLKSR)) & (1 << PCLKSR_OSC0RDY)));
    }

    void disableOSC0() {
        // Unlock the OSCCTRL0 register, which is locked by default as a safety mesure
        (*(volatile uint32_t*)(SCIF_BASE + OFFSET_UNLOCK))
                = UNLOCK_KEY            // KEY : Magic word (see datasheet)
                | OFFSET_OSCCTRL0;      // ADDR : unlock OSCCTRL0

        // Reset the saved frequency
        _osc0Frequency = 0;

        // Disable OSC0
        (*(volatile uint32_t*)(SCIF_BASE + OFFSET_OSCCTRL0)) = 0;
    }

    unsigned long getOSC0Frequency() {
        return _osc0Frequency;
    }


    // PLL

    void enablePLL(int mul, int div, GCLKSource referenceClock, unsigned long referenceFrequency) {
        // Enable reference clock
        SCIF::enableGenericClock(SCIF::GCLKChannel::GCLK9_PLL0_PEVC1, referenceClock);

        // Check frequency-related parameters
        if (mul == 0 || mul > 15 || div > 15) {
            Error::happened(Error::Module::SCIF, ERR_PLL_OUT_OF_RANGE, Error::Severity::CRITICAL);
            return;
        }

        // Save the frequency for future use
        if (div == 0) {
            _pllFrequency = 2 * (mul + 1) * referenceFrequency;
        } else {
            _pllFrequency = ((mul + 1) * referenceFrequency) / div;
        }

        // Disable PLL
        (*(volatile uint32_t*)(SCIF_BASE + OFFSET_UNLOCK))
            = UNLOCK_KEY       // KEY : Magic word (see datasheet)
            | OFFSET_PLL0;     // ADDR : unlock PLL0
        (*(volatile uint32_t*)(SCIF_BASE + OFFSET_PLL0)) = 0;

        // Configure PLL
        (*(volatile uint32_t*)(SCIF_BASE + OFFSET_UNLOCK))
            = UNLOCK_KEY       // KEY : Magic word (see datasheet)
            | OFFSET_PLL0;     // ADDR : unlock PLL0
        (*(volatile uint32_t*)(SCIF_BASE + OFFSET_PLL0))
            = 1 << PLL0_PLLOSC     // PLLOSC : select GCLK9 as a reference
            | div << PLL0_PLLDIV   // PLLDIV : configure division factor
            | mul << PLL0_PLLMUL;  // PLLMUL : configure multiplication factor

        // Enable PLL
        (*(volatile uint32_t*)(SCIF_BASE + OFFSET_UNLOCK))
            = UNLOCK_KEY       // KEY : Magic word (see datasheet)
            | OFFSET_PLL0;     // ADDR : unlock PLL0
        (*(volatile uint32_t*)(SCIF_BASE + OFFSET_PLL0))
            |= 1 << PLL0_EN;   // EN : enable the oscillator

        // Wait for PLL to be ready
        while (!((*(volatile uint32_t*)(SCIF_BASE + OFFSET_PCLKSR)) & (1 << PCLKSR_PLL0LOCK)));
    }

    void disablePLL() {
        // Reset the saved frequency
        _pllFrequency = 0;

        // Disable PLL
        (*(volatile uint32_t*)(SCIF_BASE + OFFSET_UNLOCK))
            = UNLOCK_KEY            // KEY : Magic word (see datasheet)
            | OFFSET_PLL0;          // ADDR : unlock PLL0

        (*(volatile uint32_t*)(SCIF_BASE + OFFSET_PLL0)) = 0;
    }

    unsigned long getPLLFrequency() {
        return _pllFrequency;
    }


    // DFLL

    void enableDFLL(unsigned long frequency, GCLKSource referenceClock, unsigned long referenceFrequency) {
        // Enable reference 32 KHz clock
        SCIF::enableGenericClock(SCIF::GCLKChannel::GCLK0_DFLL, referenceClock);

        // Compute frequency-related parameters
        uint8_t range = 0;
        if (frequency >= 96000000 && frequency <= 150000000) {
            range = 0;
        } else if (frequency >= 50000000) {
            range = 1;
        } else if (frequency >= 25000000) {
            range = 2;
        } else if (frequency >= 20000000) {
            range = 3;
        } else {
            Error::happened(Error::Module::SCIF, ERR_DFLL_OUT_OF_RANGE, Error::Severity::CRITICAL);
            return;
        }
        unsigned long mul = frequency / referenceFrequency;
        if (mul > 65535) {
            Error::happened(Error::Module::SCIF, ERR_DFLL_OUT_OF_RANGE, Error::Severity::CRITICAL);
            return;
        }

        // Save the frequency for future use
        _dfllFrequency = frequency;

        // Enable DFLL
        (*(volatile uint32_t*)(SCIF_BASE + OFFSET_UNLOCK))
            = UNLOCK_KEY            // KEY : Magic word (see datasheet)
            | OFFSET_DFLL0CONF;     // ADDR : unlock DFLL0CONF
        (*(volatile uint32_t*)(SCIF_BASE + OFFSET_DFLL0CONF))
                |= 1 << DFLL0CONF_EN;            // EN : enable the oscillator

        // Wait for DFLL to be ready
        while (!((*(volatile uint32_t*)(SCIF_BASE + OFFSET_PCLKSR)) & (1 << PCLKSR_DFLL0RDY)));

        // Configure frequency range
        (*(volatile uint32_t*)(SCIF_BASE + OFFSET_UNLOCK))
            = UNLOCK_KEY            // KEY : Magic word (see datasheet)
            | OFFSET_DFLL0CONF;     // ADDR : unlock DFLL0CONF
        (*(volatile uint32_t*)(SCIF_BASE + OFFSET_DFLL0CONF))
                |= range << DFLL0CONF_RANGE; // RANGE : Frequency range value

        // Configure frequency multiplier
        (*(volatile uint32_t*)(SCIF_BASE + OFFSET_UNLOCK))
            = UNLOCK_KEY            // KEY : Magic word (see datasheet)
            | OFFSET_DFLL0MUL;     // ADDR : unlock DFLL0CONF
        (*(volatile uint32_t*)(SCIF_BASE + OFFSET_DFLL0MUL)) = mul;

        // Configure maximum steps (used to adjust the speed at which the DFLL locks)
        (*(volatile uint32_t*)(SCIF_BASE + OFFSET_UNLOCK))
            = UNLOCK_KEY            // KEY : Magic word (see datasheet)
            | OFFSET_DFLL0STEP;     // ADDR : unlock DFLL0CONF
        (*(volatile uint32_t*)(SCIF_BASE + OFFSET_DFLL0STEP))
                = 10 << DFLL0STEP_FSTEP          // FSTEP : Fine maximum step
                | 10 << DFLL0STEP_CSTEP;         // CSTEP : Coarse maximum step

        // Enable Closed Loop mode
        (*(volatile uint32_t*)(SCIF_BASE + OFFSET_UNLOCK))
            = UNLOCK_KEY            // KEY : Magic word (see datasheet)
            | OFFSET_DFLL0CONF;     // ADDR : unlock DFLL0CONF
        (*(volatile uint32_t*)(SCIF_BASE + OFFSET_DFLL0CONF))
                |= 1 << DFLL0CONF_MODE;          // MODE : closed loop mode

        // Wait for DFLL to be ready
        while (!((*(volatile uint32_t*)(SCIF_BASE + OFFSET_PCLKSR)) & (1 << PCLKSR_DFLL0RDY)));
    }

    void disableDFLL() {
        // Reset the saved frequency
        _dfllFrequency = 0;

        // Disable DFLL
        (*(volatile uint32_t*)(SCIF_BASE + OFFSET_UNLOCK))
            = UNLOCK_KEY            // KEY : Magic word (see datasheet)
            | OFFSET_DFLL0CONF;     // ADDR : unlock DFLL0CONF

        (*(volatile uint32_t*)(SCIF_BASE + OFFSET_DFLL0CONF)) = 0;
    }

    unsigned long getDFLLFrequency() {
        return _dfllFrequency;
    }


    // RC80M

    void enableRC80M() {
        // Configure RC80M
        (*(volatile uint32_t*)(SCIF_BASE + OFFSET_RC80MCR))
                = 1 << RCFASTCFG_EN;             // EN : enable the oscillator
    }

    void disableRC80M() {
        // Configure RC80M
        (*(volatile uint32_t*)(SCIF_BASE + OFFSET_RC80MCR))
                = 0 << RCFASTCFG_EN;             // EN : enable the oscillator
    }

    unsigned long getRC80MFrequency() {
        return _rc80mFrequency;
    }


    // Generic clocks

    void enableGenericClock(GCLKChannel channel, GCLKSource source, bool output, uint32_t divider) {
        // If this clock has an output, set the corresponding pin in peripheral mode
        if (output && channel <= GCLKChannel::GCLK3_CATB) {
            GPIO::enablePeripheral(PINS_GCLK[static_cast<int>(channel)]);
            _genericClockOutputEnabled[static_cast<int>(channel)] = true;
        }

        uint16_t d = 0;
        if (divider >= 2) {
            d = divider / 2 - 1;
        }

        // Configure clock
        (*(volatile uint32_t*)(SCIF_BASE + OFFSET_GCCTRL0 + static_cast<int>(channel) * 0x04))
                = 1 << GCCTRL_CEN           // CEN : enable the clock
                | (d > 0) << GCCTRL_DIVEN   // DIVEN : enable the clock divider if desired
                | static_cast<int>(source) << GCCTRL_OSCSEL   // OSCSEL : select desired clock
                | d << GCCTRL_DIV;          // DIV : desired division factor
    }

    void disableGenericClock(GCLKChannel channel) {
        // Disable the output if it was enabled
        if (_genericClockOutputEnabled[static_cast<int>(channel)]) {
            GPIO::disablePeripheral(PINS_GCLK[static_cast<int>(channel)]);
            _genericClockOutputEnabled[static_cast<int>(channel)] = false;
        }

        // Disable the clock
        (*(volatile uint32_t*)(SCIF_BASE + OFFSET_GCCTRL0 + static_cast<int>(channel) * 0x04)) = 0;
    }


    void setPin(PinFunction function, int channel, GPIO::Pin pin) {
        switch (function) {
            case PinFunction::GCLK:
                PINS_GCLK[static_cast<int>(channel)] = pin;
                break;

            case PinFunction::GCLK_IN:
                PINS_GCLK_IN[static_cast<int>(channel)] = pin;
                break;
        }
    }

}