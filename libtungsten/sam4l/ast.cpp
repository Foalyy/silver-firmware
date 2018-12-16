#include "ast.h"
#include "core.h"

namespace AST {

    // Represents the highest 32 bits of the 64-bit current time
    // The lowest 32 bits are taken from the Current Value Register (CV)
    // See time()
    Time _currentTimeHighBytes = 0;

    // User handler for the Alarm interrupt
    void (*_alarmHandler)();
    extern uint8_t INTERRUPT_PRIORITY;


    // Internal functions
    void overflowHandler();
    void alarmHandlerWrapper();
    inline void waitWhileBusy() {
        while ((*(volatile uint32_t*)(BASE + OFFSET_SR)) & (1 << SR_BUSY));
    };


    // Note : the CR, CV, SCR, WER, EVE, EVD, ARn, PIRn, and DTR registers
    // cannot be read or written when SR.BUSY is set, because the peripheral
    // is synchronizing them between the two clock domains


    void init() {
        // See datasheet §19.5.1 Initialization

        // Select the clock
        (*(volatile uint32_t*)(BASE + OFFSET_CLOCK))
                = 1 << CLOCK_CSSEL; // 32kHz clock (OSC32 or RC32)
        while ((*(volatile uint32_t*)(BASE + OFFSET_SR)) & (1 << SR_CLKBUSY));

        // Enable the clock
        (*(volatile uint32_t*)(BASE + OFFSET_CLOCK))
                |= 1 << CLOCK_CEN;
        while ((*(volatile uint32_t*)(BASE + OFFSET_SR)) & (1 << SR_CLKBUSY));

        // IER (Interrupt Enable Register) : enable the Overflow interrupt
        (*(volatile uint32_t*)(BASE + OFFSET_IER)) = 1 << SR_OVF;

        // Set the handler and enable the module interrupt at the Core level
        Core::setInterruptHandler(Core::Interrupt::AST_OVF, overflowHandler);
        Core::enableInterrupt(Core::Interrupt::AST_OVF, INTERRUPT_PRIORITY);

        // Enable wake-up for the OVF interrupt
        waitWhileBusy();
        (*(volatile uint32_t*)(BASE + OFFSET_WER))
                = 1 << SR_OVF;

        // Reset the counter value
        waitWhileBusy();
        (*(volatile uint32_t*)(BASE + OFFSET_CV)) = 0;

        // Reset the prescaler value
        (*(volatile uint32_t*)(BASE + OFFSET_CR))
                = 1 << CR_PCLR;

        // Configure the digital tuner to slow down the clock
        // 32.768kHz -> 31.96878kHz (this is the closest it can get to 32.000kHz)
        // See the formula in the datasheet §19.5.7
        waitWhileBusy();
        (*(volatile uint32_t*)(BASE + OFFSET_DTR))
                = 1 << DTR_EXP
                | 0 << DTR_ADD
                | 13 << DTR_VALUE;

        // Configure the prescaler to divide the clock by 32
        // 31.96878kHz -> 0.99902kHz
        // f_out = f_in / 2^(PSEL+1)
        waitWhileBusy();
        (*(volatile uint32_t*)(BASE + OFFSET_CR))
                = 4 << CR_PSEL;

        // Enable the counter
        waitWhileBusy();
        (*(volatile uint32_t*)(BASE + OFFSET_CR))
                |= 1 << CR_EN;
    }

    void enableAlarm(Time time, bool relative, void (*handler)(), bool wake) {
        // Critical section
        Core::disableInterrupts();

        // Set the user handler
        _alarmHandler = handler;

        // Enable wake-up for the ALARM interrupt
        if (wake) {
            waitWhileBusy();
            (*(volatile uint32_t*)(BASE + OFFSET_WER))
                    = 1 << SR_ALARM0;
        }

        // IDR (Interrupt Disable Register) : disable the Alarm interrupt
        (*(volatile uint32_t*)(BASE + OFFSET_IDR)) = 1 << SR_ALARM0;

        // Reset the alarm time
        (*(volatile uint32_t*)(BASE + OFFSET_AR0)) = 0;

        // SCR (Status Clear Register) : clear the interrupt
        waitWhileBusy();
        (*(volatile uint32_t*)(BASE + OFFSET_SCR)) = 1 << SR_ALARM0;
        
        // IER (Interrupt Enable Register) : enable the Alarm interrupt
        (*(volatile uint32_t*)(BASE + OFFSET_IER)) = 1 << SR_ALARM0;

        // Enable the module interrupt at the Core level
        Core::setInterruptHandler(Core::Interrupt::AST_ALARM, alarmHandlerWrapper);
        Core::enableInterrupt(Core::Interrupt::AST_ALARM, INTERRUPT_PRIORITY);
        
        // Set the alarm time
        // The 32-bit truncate is not a problem as long as the alarm is not more than
        // ~ 50 days in the future (2^32=4294967296 ms or ~1193 hours or 49.71 days)
        waitWhileBusy();
        if (relative) {
            time += *(volatile uint32_t*)(BASE + OFFSET_CV) + 1;
        }
        (*(volatile uint32_t*)(BASE + OFFSET_AR0)) = (uint32_t)time;

        // End of critical section
        Core::enableInterrupts();
    }

    void disableAlarm() {
        // IDR (Interrupt Disable Register) : disable the Alarm interrupt
        (*(volatile uint32_t*)(BASE + OFFSET_IDR)) = 1 << SR_ALARM0;
    }

    void overflowHandler() {
        // Increment the high bytes of the 64-bit counter
        _currentTimeHighBytes += (uint64_t)1 << 32;

        // SCR (Status Clear Register) : clear the interrupt
        waitWhileBusy();
        (*(volatile uint32_t*)(BASE + OFFSET_SCR)) = 1 << SR_OVF;
    }

    void alarmHandlerWrapper() {
        // Call the user handler if defined
        if (_alarmHandler != nullptr) {
            _alarmHandler();
        }

        // Disable the alarm so it is not triggered again after
        // the next counter overflow
        disableAlarm();

        // SCR (Status Clear Register) : clear the interrupt
        waitWhileBusy();
        (*(volatile uint32_t*)(BASE + OFFSET_SCR)) = 1 << SR_ALARM0;
    }

    bool alarmPassed() {
        return *(volatile uint32_t*)(BASE + OFFSET_CV) >= *(volatile uint32_t*)(BASE + OFFSET_AR0);
    }

}