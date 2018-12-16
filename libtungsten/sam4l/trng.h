#ifndef _TRNG_H_
#define _TRNG_H_

#include <stdint.h>

// True Random Number Generator
// This module provides 32-bit highly random numbers
// based on specified hardware.
namespace TRNG {

    // Peripheral memory space base address
    const uint32_t BASE = 0x40068000;

    // Registers addresses
    const uint32_t OFFSET_CR =      0x00; // Control Register
    const uint32_t OFFSET_IER =     0x10; // Interrupt Enable Register
    const uint32_t OFFSET_IDR =     0x14; // Interrupt Disable Register
    const uint32_t OFFSET_IMR =     0x18; // Interrupt Mask Register
    const uint32_t OFFSET_ISR =     0x1C; // Interrupt Status Register
    const uint32_t OFFSET_ODATA =   0x50; // Output Data Register

    // Constants
    const uint32_t CR_ENABLE = 0;
    const uint32_t CR_KEY = 0x524E47 << 8;
    const uint32_t ISR_DATRDY = 0;


    void enable();
    bool available();
    uint32_t get();
    void enableInterrupt(void (*handler)(uint32_t));
    void disableInterrupt();

}


#endif