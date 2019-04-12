#ifndef _AST_H_
#define _AST_H_

#include <stdint.h>

// Asynchronous Timer
// This module manages the 32-bit asynchronous timer/counter
// which is used as a system time (with a 2-ms resolution) and is able
// to wake up the chip at a specific time
namespace AST {

    // Peripheral memory space base address
    const uint32_t BASE = 0x400F0800;

    // Registers addresses
    const uint32_t OFFSET_CR =          0x000; // Control Register
    const uint32_t OFFSET_CV =          0x004; // Control Value Register
    const uint32_t OFFSET_SR =          0x008; // Status Register
    const uint32_t OFFSET_SCR =         0x00C; // Status Clear Register
    const uint32_t OFFSET_IER =         0x010; // Interrupt Enable Register
    const uint32_t OFFSET_IDR =         0x014; // Interrupt Disable Register
    const uint32_t OFFSET_IMR =         0x018; // Interrupt Mask Register
    const uint32_t OFFSET_WER =         0x01C; // Wake Enable Register
    const uint32_t OFFSET_AR0 =         0x020; // Alarm Register 0
    //const uint32_t OFFSET_AR1 =         0x024; // Alarm Register 1 -- not implemented
    const uint32_t OFFSET_PIR0 =        0x030; // Periodic Interval Register 0
    //const uint32_t OFFSET_PIR1 =        0x034; // Periodic Interval Register 1 -- not implemented
    const uint32_t OFFSET_CLOCK =       0x040; // Clock Control Register
    const uint32_t OFFSET_DTR =         0x044; // Digital Tuner Register
    const uint32_t OFFSET_EVE =         0x048; // Event Enable Register
    const uint32_t OFFSET_EVD =         0x04C; // Event Disable Register
    const uint32_t OFFSET_EVM =         0x050; // Event Mask Register
    const uint32_t OFFSET_CALV =        0x054; // Calendar Value Register

    // Subregisters
    const uint32_t CR_EN = 0;
    const uint32_t CR_PCLR = 1;
    const uint32_t CR_CAL = 2;
    const uint32_t CR_CA0 = 8;
    const uint32_t CR_PSEL = 16;
    const uint32_t SR_OVF = 0;
    const uint32_t SR_ALARM0 = 8;
    const uint32_t SR_PER0 = 16;
    const uint32_t SR_BUSY = 24;
    const uint32_t SR_READY = 25;
    const uint32_t SR_CLKBUSY = 28;
    const uint32_t SR_CLKRDY = 29;
    const uint32_t CLOCK_CEN = 0;
    const uint32_t CLOCK_CSSEL = 8;
    const uint32_t DTR_EXP = 0;
    const uint32_t DTR_ADD = 5;
    const uint32_t DTR_VALUE = 8;

    using Time = volatile uint64_t;
    extern Time _currentTimeHighBytes;


    // Module API
    void init();
    inline Time time() { return ((_currentTimeHighBytes + (*(volatile uint32_t*)(BASE + OFFSET_CV))) * 1000) / (32768/2); };
    void enableAlarm(Time time, bool relative=true, void (*handler)()=nullptr, bool wake=true);
    void disableAlarm();
    bool alarmPassed();

}


#endif