#include "wdt.h"
#include "scif.h"
#include "bscif.h"
#include "core.h"

namespace WDT {

    // Watchdog interrupt
    extern uint8_t INTERRUPT_PRIORITY;
    void (*_interruptHandler)() = nullptr;
    void interruptHandlerWrapper();

    // Clock source
    bool _useOSC32K = false;

    void enable(unsigned int timeout, Unit unit, void (*timeoutHandler)(), unsigned int windowStart, bool useOSC32K) {
        // If the WDT is already enabled, disable it first
        if (isEnabled()) {
            disable();
        }

        // Interrupt mode
        bool interruptMode = false;
        if (timeoutHandler != nullptr) {
            interruptMode = true;
            _interruptHandler = timeoutHandler;
            // Actual interrupt enabling is done at the end if the function, after the WDT is enabled
        }

        // Window mode
        bool windowMode = false;
        if (windowStart > 0 && windowStart < timeout) {
            windowMode = true;
            timeout -= windowStart; // Assume that timeout starts at T0, not at windowStart
        }

        // Compute timings
        uint8_t psel = 0;
        uint8_t tban = 0;
        if (timeout > 0) {
            // Get the base period of the input clock in 10th of microseconds
            unsigned int basePeriod = 10000000L / (useOSC32K ? BSCIF::getOSC32KFrequency() : SCIF::getRCSYSFrequency());

            // Convert the timeout in units of input clock
            if (unit == Unit::MILLISECONDS) {
                timeout *= 1000; // Convert to microseconds
            }
            timeout *= 10;  // Convert to 10th of microseconds
            timeout = timeout / basePeriod;

            // Find the closest power of two greater than the computed timeout
            // cf datasheet p 491 (20. WDT / 20.6 User Interface / 20.6.1 CTRL Control Register)
            for (psel = 0; psel < 32; psel++) {
                if (psel == 31 || static_cast<unsigned int>(1 << (psel + 1)) >= timeout) {
                    break;
                }
            }

            if (windowMode) {
                // Convert the time in units of input clock
                if (unit == Unit::MILLISECONDS) {
                    windowStart *= 1000; // Convert to microseconds
                }
                windowStart *= 10;  // Convert to 10th of microseconds
                windowStart = windowStart / basePeriod;

                // Find the closest power of two lower than the computed time
                // cf datasheet p 491 (20. WDT / 20.6 User Interface / 20.6.1 CTRL Control Register)
                for (tban = 30; tban >= 0; tban--) {
                    if (static_cast<unsigned int>(1 << (tban + 1)) <= windowStart) {
                        break;
                    }
                }
            }
        }

        // Clock source
        if (useOSC32K != _useOSC32K) {
            // CTRL (Control Register) : disable the watchdog clock
            uint32_t ctrl = (*(volatile uint32_t*)(WDT_BASE + OFFSET_CTRL)) & 0x00FFFFFF;
            (*(volatile uint32_t*)(WDT_BASE + OFFSET_CTRL))
                = (ctrl & ~(uint32_t)(1 << CTRL_CEN))
                | CTRL_KEY_1;
            (*(volatile uint32_t*)(WDT_BASE + OFFSET_CTRL))
                = (ctrl & ~(uint32_t)(1 << CTRL_CEN))
                | CTRL_KEY_2;

            // Wait for the clock to be disabled
            while ((*(volatile uint32_t*)(WDT_BASE + OFFSET_CTRL)) & (1 << CTRL_CEN));

            // Select the clock
            ctrl = (*(volatile uint32_t*)(WDT_BASE + OFFSET_CTRL)) & 0x00FFFFFF;
            (*(volatile uint32_t*)(WDT_BASE + OFFSET_CTRL))
                = (ctrl & ~(uint32_t)(1 << CTRL_CSSEL))
                | static_cast<int>(useOSC32K) << CTRL_CSSEL
                | CTRL_KEY_1;
            (*(volatile uint32_t*)(WDT_BASE + OFFSET_CTRL))
                = (ctrl & ~(uint32_t)(1 << CTRL_CSSEL))
                | static_cast<int>(useOSC32K) << CTRL_CSSEL
                | CTRL_KEY_2;

            // Enable the watchdog clock again
            ctrl = (*(volatile uint32_t*)(WDT_BASE + OFFSET_CTRL)) & 0x00FFFFFF;
            (*(volatile uint32_t*)(WDT_BASE + OFFSET_CTRL))
                = ctrl
                | 1 << CTRL_CEN
                | CTRL_KEY_1;
            (*(volatile uint32_t*)(WDT_BASE + OFFSET_CTRL))
                = ctrl
                | 1 << CTRL_CEN
                | CTRL_KEY_2;

            // Wait for the clock to be enabled
            while (!((*(volatile uint32_t*)(WDT_BASE + OFFSET_CTRL)) & (1 << CTRL_CEN)));

            _useOSC32K = useOSC32K;
        }

        // CTRL (Control Register) : configure then enable the watchdog
        // The WDT must first be configured, then, in a second time, enabled.
        // The CTRL register must be written twice for each operation, the 
        // first time with the first key (0x55), then with the second key (0xAA).
        // cf datasheet p483 (20. WDT / 20.5 Functional Description / 20.5.1 Basic 
        // Mode / 20.5.1.1 WDT Control Register Access)
        uint32_t ctrl = 1 << CTRL_DAR                               // DAR : disable the watchdog after a reset
                      | static_cast<int>(windowMode) << CTRL_MODE   // MODE : Basic or Window mode
                      | static_cast<int>(interruptMode) << CTRL_IM  // IM : Interrupt Mode
                      | 1 << CTRL_FCD                               // FCD : skip flash calibration after reset
                      | psel << CTRL_PSEL                           // PSEL : timeout counter
                      | 1 << CTRL_CEN                               // CEN : enable the clock
                      | static_cast<int>(useOSC32K) << CTRL_CSSEL   // CEN : enable the clock
                      | tban << CTRL_TBAN;                          // TBAN : time ban for window mode
        (*(volatile uint32_t*)(WDT_BASE + OFFSET_CTRL)) // Configure
            = ctrl
            | CTRL_KEY_1;
        (*(volatile uint32_t*)(WDT_BASE + OFFSET_CTRL))
            = ctrl
            | CTRL_KEY_2;
        (*(volatile uint32_t*)(WDT_BASE + OFFSET_CTRL)) // Enable, keeping the same configuration
            = ctrl
            | 1 << CTRL_EN
            | CTRL_KEY_1;
        (*(volatile uint32_t*)(WDT_BASE + OFFSET_CTRL))
            = ctrl
            | 1 << CTRL_EN
            | CTRL_KEY_2;

        // Wait until the WDT is enabled
        while (!isEnabled());

        // Interrupt mode
        if (interruptMode) {
            // IER (Interrupt Enable Register) : enable the watchdog interrupt
            (*(volatile uint32_t*)(WDT_BASE + OFFSET_IER))
                    = 1 << ISR_WINT;

            // Set the handler and enable the module interrupt at the Core level
            Core::setInterruptHandler(Core::Interrupt::WDT, interruptHandlerWrapper);
            Core::enableInterrupt(Core::Interrupt::WDT, INTERRUPT_PRIORITY);
        }
    }

    bool isEnabled() {
        return (*(volatile uint32_t*)(WDT_BASE + OFFSET_CTRL)) & (1 << CTRL_EN);
    }

    void disable() {
        // CTRL (Control Register) : disable the watchdog by setting the EN bit to 0
        uint32_t ctrl = (*(volatile uint32_t*)(WDT_BASE + OFFSET_CTRL)) & 0x00FFFFFF;
        (*(volatile uint32_t*)(WDT_BASE + OFFSET_CTRL))
            = (ctrl & ~(uint32_t)(1 << CTRL_EN))
            | CTRL_KEY_1;
        (*(volatile uint32_t*)(WDT_BASE + OFFSET_CTRL))
            = (ctrl & ~(uint32_t)(1 << CTRL_EN))
            | CTRL_KEY_2;

        // Wait until the WDT is disabled
        while (isEnabled());
    }

    void clear() {
        // SR (Status Register) : wait until the previous clear operation is finished
        while (!((*(volatile uint32_t*)(WDT_BASE + OFFSET_SR)) & (1 << SR_CLEARED)));

        // CLR (Clear Register) : clear the watchdog counter
        // The register must be written twice, first with the key 0x55
        // then with the key 0xAA
        (*(volatile uint32_t*)(WDT_BASE + OFFSET_CLR))
            = 1 << CLR_WDTCLR
            | CLR_KEY_1;
        (*(volatile uint32_t*)(WDT_BASE + OFFSET_CLR))
            = 1 << CLR_WDTCLR
            | CLR_KEY_2;
    }

    void interruptHandlerWrapper() {
        // Call the user interrupt handler
        if (_interruptHandler != nullptr) {
            _interruptHandler();
        }

        // If the handler didn't clear the interrupt with clearInterrupt(), loop
        // indefinitely to prevent the handler from being immediately called again.
        // The watchdog will reset the microcontroller after the next timeout.
        if ((*(volatile uint32_t*)(WDT_BASE + OFFSET_ISR)) & (1 << ISR_WINT)) {
            while (true);
        }
    }

    void clearInterrupt() {
        // ICR (Interrupt Clear Register) : clear the watchdog interrupt
        (*(volatile uint32_t*)(WDT_BASE + OFFSET_ICR))
            = 1 << ISR_WINT;
    }
}