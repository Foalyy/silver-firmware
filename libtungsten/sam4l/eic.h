#ifndef _EIC_H_
#define _EIC_H_

#include <stdint.h>
#include "gpio.h"
#include "error.h"

// External Interrupt Controller
// This module manages synchronous and asynchronous interrupts
// which are able to wake the chip up from power save mode
namespace EIC {

    // Peripheral memory space base address
    const uint32_t BASE = 0x400F1000;


    // Registers addresses
    const uint32_t OFFSET_IER =         0x000; // Interrupt Enable Register
    const uint32_t OFFSET_IDR =         0x004; // Interrupt Disable Register
    const uint32_t OFFSET_IMR =         0x008; // Interrupt Mask Register
    const uint32_t OFFSET_ISR =         0x00C; // Interrupt Status Register
    const uint32_t OFFSET_ICR =         0x010; // Interrupt Clear Register
    const uint32_t OFFSET_MODE =        0x014; // Mode Register
    const uint32_t OFFSET_EDGE =        0x018; // Edge Register
    const uint32_t OFFSET_LEVEL =       0x01C; // Level Register
    const uint32_t OFFSET_FILTER =      0x020; // Filter Register
    const uint32_t OFFSET_TEST =        0x024; // Test Register
    const uint32_t OFFSET_ASYNC =       0x028; // Asynchronous Register
    const uint32_t OFFSET_EN =          0x030; // Enable Register
    const uint32_t OFFSET_DIS =         0x034; // Disable Register
    const uint32_t OFFSET_CTRL =        0x038; // Control Register

    // Constants
    const int NMI = 0; // EIC interrupt 0 is the NMI
    const int N_CHANNELS = 9;

    // Error codes
    const Error::Code ERR_UNKNOWN_CHANNEL = 0x0001;

    enum class Mode {
        EDGE = 0,
        LEVEL = 1,
    };

    enum class Polarity {
        LOW_FALLING = 0,
        HIGH_RISING = 1,
    };

    using Channel = unsigned int;


    // Module API
    void setPin(Channel channel, GPIO::Pin pin);
    void enableInterrupt(Channel channel, Mode mode, Polarity polarity, void (*handler)(int), bool filter=true);
    void enableAsyncInterrupt(Channel channel, Polarity polarity, void (*handler)(int)=nullptr);
    void disableInterrupt(Channel channel);
    void clearInterrupt(Channel channel);


}


#endif