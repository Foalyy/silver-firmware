#ifndef _WDT_H_
#define _WDT_H_

#include <stdint.h>

// Watchdog Timer
// This module is able to automatically reset the chip after a
// specified delay unless it is periodically serviced. This is
// useful to recover from an unexpected behaviour which leads the
// execution to hang.
namespace WDT {

    // Peripheral memory space base address
    const uint32_t WDT_BASE = 0x400F0C00;


    // Registers addresses
    const uint32_t OFFSET_CTRL =     0x000; // Control Register
    const uint32_t OFFSET_CLR =      0x004; // Clear Register
    const uint32_t OFFSET_SR =       0x008; // Status Register
    const uint32_t OFFSET_IER =      0x00C; // Interrupt Enable Register
    const uint32_t OFFSET_IDR =      0x010; // Interrupt Disable Register
    const uint32_t OFFSET_IMR =      0x014; // Interrupt Mask Register
    const uint32_t OFFSET_ISR =      0x018; // Interrupt Status Register
    const uint32_t OFFSET_ICR =      0x01C; // Interrupt Clear Register


    // Subregisters
    const uint32_t CTRL_EN = 0;     // WDT Enable
    const uint32_t CTRL_DAR = 1;    // WDT Disable After Reset
    const uint32_t CTRL_MODE = 2;   // WDT Mode
    const uint32_t CTRL_SFV = 3;    // WDT Control Register Store Final Value
    const uint32_t CTRL_IM = 4;     // Interrupt Mode
    const uint32_t CTRL_FCD = 7;    // Flash Calibration Done
    const uint32_t CTRL_PSEL = 8;   // Timeout Prescale Select
    const uint32_t CTRL_CEN = 16;   // Clock Enable
    const uint32_t CTRL_CSSEL = 17; // Clock Source Select
    const uint32_t CTRL_TBAN = 18;  // Time Ban Prescale Select
    const uint32_t CTRL_KEY = 24;   // Key
    const uint32_t CLR_WDTCLR = 0;  // Watchdog Clear
    const uint32_t CLR_KEY = 24;    // Key
    const uint32_t SR_WINDOW = 0;   // Counter inside the PSEL period
    const uint32_t SR_CLEARED = 1;  // WDT cleared
    const uint32_t ISR_WINT = 2;    // Watchdog interrupt


    // Constants
    const uint32_t CTRL_KEY_1 = 0x55 << CTRL_KEY;
    const uint32_t CTRL_KEY_2 = 0xAA << CTRL_KEY;
    const uint32_t CLR_KEY_1 = 0x55 << CLR_KEY;
    const uint32_t CLR_KEY_2 = 0xAA << CLR_KEY;


    enum class Unit {
        MILLISECONDS,
        MICROSECONDS
    };

    // Module API
    void enable(unsigned int timeout, Unit unit=Unit::MILLISECONDS, void (*timeoutHandler)()=nullptr, unsigned int windowStart=0, bool useOSC32K=true);
    void disable();
    bool isEnabled();
    void clear();
    void clearInterrupt();

}


#endif