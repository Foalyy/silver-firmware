#include "trng.h"
#include "core.h"
#include "pm.h"

namespace TRNG {

    // Interrupt handler
    extern uint8_t INTERRUPT_PRIORITY;
    uint32_t _dataReadyHandler = 0;
    void interruptHandlerWrapper();

    void enable() {
        // Enable the clock
        PM::enablePeripheralClock(PM::CLK_TRNG);

        // CR (Control Register) : enable the TRNG
        (*(volatile uint32_t*)(BASE + OFFSET_CR))
            = 1 << CR_ENABLE
            | CR_KEY;
    }

    bool available() {
        // Return true if a new random number is available
        return (*(volatile uint32_t*)(BASE + OFFSET_ISR)) & 1 << ISR_DATRDY;
    }

    uint32_t get() {
        // Clear the DATARDY bit by reading ISR
        available();

        // Return the new random number
        return (*(volatile uint32_t*)(BASE + OFFSET_ODATA));
    }

    void enableInterrupt(void (*handler)(uint32_t)) {
        // Save the user handler
        _dataReadyHandler = (uint32_t)handler;

        // IER (Interrupt Enable Register) : enable the Data Ready interrupt (this is the
        // only interrupt available)
        (*(volatile uint32_t*)(BASE + OFFSET_IER))
                = 1 << ISR_DATRDY;

        // Set the handler and enable the module interrupt at the Core level
        Core::setInterruptHandler(Core::Interrupt::TRNG, interruptHandlerWrapper);
        Core::enableInterrupt(Core::Interrupt::TRNG, INTERRUPT_PRIORITY);
    }

    void disableInterrupt() {
        // IER (Interrupt Disable Register) : disable the interrupt
        (*(volatile uint32_t*)(BASE + OFFSET_IDR))
                = 1 << ISR_DATRDY;

        // Disable the module interrupt at the Core level
        Core::disableInterrupt(Core::Interrupt::TRNG);
    }

    void interruptHandlerWrapper() {
        // Call the user handler
        void (*handler)(uint32_t) = (void (*)(uint32_t))_dataReadyHandler;
        if (handler != nullptr) {
            handler(get());
        }

        // Clear the interrupt by reading ISR
        available();
    }

}