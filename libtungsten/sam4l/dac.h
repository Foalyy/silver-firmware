#ifndef _DAC_H_
#define _DAC_H_

#include <stdint.h>
#include "gpio.h"

// Digital to Analog Converter
// This module is used to generate an analog voltage on a pin
namespace DAC {

    // Peripheral memory space base address
    const uint32_t DAC_BASE = 0x4003C000;

    // Registers addresses
    const uint32_t OFFSET_CR =      0x00; // Control Register
    const uint32_t OFFSET_MR =      0x04; // Mode Register
    const uint32_t OFFSET_CDR =     0x08; // Conversion Data Register
    const uint32_t OFFSET_IER =     0x0C; // Interrupt Enable Register
    const uint32_t OFFSET_IDR =     0x10; // Interrupt Disable Register
    const uint32_t OFFSET_IMR =     0x14; // Interrupt Mask Register
    const uint32_t OFFSET_ISR =     0x18; // Interrupt Status Register
    const uint32_t OFFSET_WPMR =    0xE4; // Write Protect Mode Register
    const uint32_t OFFSET_WPSR =    0xE8; // Write Protect Status Register

    // Constants
    const uint32_t CR_SWRST = 0;
    const uint32_t MR_TRGEN = 0;
    const uint32_t MR_TRGSEL = 1;
    const uint32_t MR_DACEN = 4;
    const uint32_t MR_WORD = 5;
    const uint32_t MR_STARTUP = 8;
    const uint32_t MR_CLKDIV = 16;
    const uint32_t WPMR_WPEN = 0;
    const uint32_t WPMR_WPKEY = 0x444143 << 8;

    enum class Interrupt {
        RELOAD_EMPTY,
        TRANSFER_FINISHED
    };


    // Module API
    void enable();
    void disable();
    void write(uint16_t value); // 0 <= value <= 1023
    void start(uint16_t* buffer, int n, bool repeat=false);
    void reload(uint16_t* buffer, int n);
    void stop();
    bool setFrequency(unsigned long frequency);
    bool isFinished();
    bool isReloadEmpty();
    void enableInterrupt(void (*handler)(), Interrupt interrupt=Interrupt::TRANSFER_FINISHED);
    void disableInterrupt(Interrupt interrupt=Interrupt::TRANSFER_FINISHED);
    void setPin(GPIO::Pin pin);

}


#endif