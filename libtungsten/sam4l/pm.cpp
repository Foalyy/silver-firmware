#include "pm.h"
#include "scif.h"
#include "core.h"
#include "error.h"

namespace PM {

    // Clock frequencies
    unsigned long _mainClockFrequency = RCSYS_FREQUENCY;
    unsigned long _cpuClockFrequency = RCSYS_FREQUENCY;
    unsigned long _hsbClockFrequency = RCSYS_FREQUENCY;
    unsigned long _pbaClockFrequency = RCSYS_FREQUENCY;

    // Interrupt handlers
    extern uint8_t INTERRUPT_PRIORITY;
    uint32_t _interruptHandlers[N_INTERRUPTS];
    const int _interruptBits[N_INTERRUPTS] = {SR_CFD, SR_CKRDY, SR_WAKE};
    void interruptHandlerWrapper();
    

    // The main clock is used by the CPU and the peripheral buses and can be connected
    // to any of the clock sources listed in MainClockSource
    void setMainClockSource(MainClockSource clockSource, unsigned long cpudiv) {
        if (cpudiv >= 1) {
            // Unlock the CPUSEL register
            (*(volatile uint32_t*)(BASE + OFFSET_UNLOCK))
                    = UNLOCK_KEY           // KEY : Magic word (see datasheet)
                    | OFFSET_CPUSEL;        // ADDR : unlock CPUSEL

            // Configure the CPU clock divider
            if (cpudiv > 7) {
                cpudiv = 7;
            }
            (*(volatile uint32_t*)(BASE + OFFSET_CPUSEL))
                    = (cpudiv - 1) << CPUSEL_CPUSEL  // CPUSEL : select divider factor
                    | 1 << CPUSEL_CPUDIV;         // CPUDIV : enable divider

            // Wait for the divider to be ready
            while (!((*(volatile uint32_t*)(BASE + OFFSET_SR)) & (1 << SR_CKRDY)));
        }

        // Unlock the MCCTRL register, which is locked by default as a safety mesure
        (*(volatile uint32_t*)(BASE + OFFSET_UNLOCK))
                = UNLOCK_KEY               // KEY : Magic word (see datasheet)
                | OFFSET_MCCTRL;           // ADDR : unlock MCCTRL

        // Change the main clock source
        (*(volatile uint32_t*)(BASE + OFFSET_MCCTRL))
                = static_cast<int>(clockSource) << MCCTRL_MCSEL; // MCSEL : select clock source

        // Save the frequency
        switch (clockSource) {
            case MainClockSource::RCSYS:
                _mainClockFrequency = RCSYS_FREQUENCY;
                break;
            
            case MainClockSource::OSC0:
                _mainClockFrequency = SCIF::getOSC0Frequency();
                break;
            
            case MainClockSource::PLL:
                _mainClockFrequency = SCIF::getPLLFrequency();
                break;
            
            case MainClockSource::DFLL:
                _mainClockFrequency = SCIF::getDFLLFrequency();
                break;

            case MainClockSource::RCFAST:
                _mainClockFrequency = SCIF::getRCFASTFrequency();
                break;

            case MainClockSource::RC80M:
                _mainClockFrequency = RC80M_FREQUENCY;
                break;
        }
        _cpuClockFrequency = _mainClockFrequency / (1 << cpudiv);
        _hsbClockFrequency = _mainClockFrequency;
        _pbaClockFrequency = _mainClockFrequency;
    }

    unsigned long getModuleClockFrequency(uint8_t peripheral) {
        if (peripheral >= HSBMASK && peripheral < PBAMASK) {
            return _hsbClockFrequency;
        } else if (peripheral >= PBAMASK && peripheral < PBBMASK) {
            return _pbaClockFrequency;
        }
        return 0;
    }

    void enablePeripheralClock(uint8_t peripheral, bool enabled) {
        // Select the correct register
        int offset = 0;
        if (peripheral >= HSBMASK && peripheral < PBAMASK) {
            offset = OFFSET_HSBMASK;
            peripheral -= HSBMASK;
        } else if (peripheral >= PBAMASK && peripheral < PBBMASK) {
            offset = OFFSET_PBAMASK;
            peripheral -= PBAMASK;
        } else if (peripheral >= PBBMASK) {
            offset = OFFSET_PBBMASK;
            peripheral -= PBBMASK;
        }

        // Unlock the selected register, which is locked by default as a safety mesure
        (*(volatile uint32_t*)(BASE + OFFSET_UNLOCK))
                = UNLOCK_KEY                // KEY : Magic word (see datasheet)
                | offset;                   // ADDR : unlock the selected register

        if (enabled) {
            // Unmask the corresponding clock
            (*(volatile uint32_t*)(BASE + offset)) |= 1 << peripheral;
        } else {
            // Unmask the corresponding clock
            (*(volatile uint32_t*)(BASE + offset)) &= ~(uint32_t)(1 << peripheral);
        }
    }

    void disablePeripheralClock(uint8_t peripheral) {
        enablePeripheralClock(peripheral, false);
    }

    void enablePBADivClock(uint8_t pow) {
        // Unlock the selected register, which is locked by default as a safety mesure
        (*(volatile uint32_t*)(BASE + OFFSET_UNLOCK))
                = UNLOCK_KEY                // KEY : Magic word (see datasheet)
                | OFFSET_PBADIVMASK;        // ADDR : unlock PBADIVMASK

        // Check that the power of two selected is in range
        if (pow < 1 || pow > 7) {
            return;
        }

        // Unmask the corresponding divided clock
        (*(volatile uint32_t*)(BASE + OFFSET_PBADIVMASK)) |= 1 << (pow - 1);
    }


    // Returns the cause of the last reset. This is useful for example to handle faults
    // detected by the watchdog or the brown-out detectors.
    ResetCause resetCause() {
        uint32_t rcause = (*(volatile uint32_t*)(BASE + OFFSET_RCAUSE));
        if (rcause != 0) {
            for (int i = 0; i < 32; i++) {
                if (rcause & 1 << i) {
                    return static_cast<ResetCause>(i);
                }
            }
        }
        return ResetCause::UNKNOWN;
    }

    // Returns the cause of the last wake up
    WakeUpCause wakeUpCause() {
        uint32_t wcause = (*(volatile uint32_t*)(BASE + OFFSET_WCAUSE));
        if (wcause != 0) {
            for (int i = 0; i < 32; i++) {
                if (wcause & 1 << i) {
                    return static_cast<WakeUpCause>(i);
                }
            }
        }
        return WakeUpCause::UNKNOWN;
    }

    void enableWakeUpSource(WakeUpSource src) {
        // AWEN (Asynchronous Wake Up Enable Register) : set the corresponding bit
        (*(volatile uint32_t*)(BASE + OFFSET_AWEN))
            |= 1 << static_cast<int>(src);
    }

    void disableWakeUpSource(WakeUpSource src) {
        // AWEN (Asynchronous Wake Up Enable Register) : clear the corresponding bit
        (*(volatile uint32_t*)(BASE + OFFSET_AWEN))
            &= ~(uint32_t)(1 << static_cast<int>(src));
    }

    void disableWakeUpSources() {
        // AWEN (Asynchronous Wake Up Enable Register) : clear the register
        (*(volatile uint32_t*)(BASE + OFFSET_AWEN)) = 0;
    }


    void enableInterrupt(void (*handler)(), Interrupt interrupt) {
        // Save the user handler
        _interruptHandlers[static_cast<int>(interrupt)] = (uint32_t)handler;

        // IER (Interrupt Enable Register) : enable the requested interrupt (WAKE by default)
        (*(volatile uint32_t*)(BASE + OFFSET_IER))
                = 1 << _interruptBits[static_cast<int>(interrupt)];

        // Set the handler and enable the module interrupt at the Core level
        Core::setInterruptHandler(Core::Interrupt::PM, interruptHandlerWrapper);
        Core::enableInterrupt(Core::Interrupt::PM, INTERRUPT_PRIORITY);
    }

    void disableInterrupt(Interrupt interrupt) {
        // IDR (Interrupt Disable Register) : disable the requested interrupt (WAKE by default)
        (*(volatile uint32_t*)(BASE + OFFSET_IDR))
                = 1 << _interruptBits[static_cast<int>(interrupt)];

        // If no interrupt is enabled anymore, disable the module interrupt at the Core level
        if ((*(volatile uint32_t*)(BASE + OFFSET_IMR)) == 0) {
            Core::disableInterrupt(Core::Interrupt::PM);
        }
    }

    void interruptHandlerWrapper() {
        // Call the user handler of every interrupt that is enabled and pending
        for (int i = 0; i < N_INTERRUPTS; i++) {
            if ((*(volatile uint32_t*)(BASE + OFFSET_IMR)) & (1 << _interruptBits[i]) // Interrupt is enabled
                    && (*(volatile uint32_t*)(BASE + OFFSET_ISR)) & (1 << _interruptBits[i])) { // Interrupt is pending
                void (*handler)() = (void (*)())_interruptHandlers[i];
                if (handler != nullptr) {
                    handler();
                }

                // Clear the interrupt by reading ISR
                (*(volatile uint32_t*)(BASE + OFFSET_ICR)) = 1 << _interruptBits[i];
            }
        }
    }

}