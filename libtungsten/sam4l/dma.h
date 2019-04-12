#ifndef _DMA_H_
#define _DMA_H_

#include <stdint.h>
#include "error.h"

// Direct Memory Access
// This module is able to automatically copy data between RAM and peripherals,
// without CPU intervention
namespace DMA {

    // Peripheral memory space base address
    const uint32_t BASE = 0x400A2000;
    const uint32_t CHANNEL_REG_SIZE = 0x40;

    // Registers addresses
    const uint32_t OFFSET_MAR =      0x000; // Memory Address Register
    const uint32_t OFFSET_PSR =      0x004; // Peripheral Select Register
    const uint32_t OFFSET_TCR =      0x008; // Transfer Counter Register
    const uint32_t OFFSET_MARR =     0x00C; // Memory Address Reload Register
    const uint32_t OFFSET_TCRR =     0x010; // Transfer Counter Reload Register
    const uint32_t OFFSET_CR =       0x014; // Control Register
    const uint32_t OFFSET_MR =       0x018; // Mode Register
    const uint32_t OFFSET_SR =       0x01C; // Status Register
    const uint32_t OFFSET_IER =      0x020; // Interrupt Enable Register
    const uint32_t OFFSET_IDR =      0x024; // Interrupt Disable Register
    const uint32_t OFFSET_IMR =      0x028; // Interrupt Mask Register
    const uint32_t OFFSET_ISR =      0x02C; // Interrupt Status Register

    // Subregisters
    const uint8_t MR_SIZE = 0;
    const uint8_t MR_ETRIG = 2;
    const uint8_t MR_RING = 3;
    const uint8_t CR_TEN = 0;
    const uint8_t CR_TDIS = 1;
    const uint8_t ISR_RCZ = 0;
    const uint8_t ISR_TRC = 1;
    const uint8_t ISR_TERR = 2;
    const uint8_t SR_TEN = 0;

    // Error codes
    const Error::Code ERR_NO_CHANNEL_AVAILABLE = 0x0001;
    const Error::Code ERR_CHANNEL_NOT_INITIALIZED = 0x0002;

    // Size constants
    enum class Size {
        BYTE,
        HALFWORD,
        WORD
    };

    // Interrupts
    const int N_INTERRUPTS = 3;
    enum class Interrupt {
        RELOAD_EMPTY,
        TRANSFER_FINISHED,
        TRANSFER_ERROR
    };

    // Device constants
    enum class Device {
        USART0_RX = 0,
        USART1_RX = 1,
        USART2_RX = 2,
        USART3_RX = 3,
        SPI_RX = 4,
        I2C0_M_RX = 5,
        I2C1_M_RX = 6,
        I2C2_M_RX = 7,
        I2C3_M_RX = 8,
        I2C0_S_RX = 9,
        I2C1_S_RX = 10,
        USART0_TX = 18,
        USART1_TX = 19,
        USART2_TX = 20,
        USART3_TX = 21,
        SPI_TX = 22,
        I2C0_M_TX = 23,
        I2C1_M_TX = 24,
        I2C2_M_TX = 25,
        I2C3_M_TX = 26,
        I2C0_S_TX = 27,
        I2C1_S_TX = 28,
        DAC = 35
    };

    struct ChannelConfig {
        bool started;
        bool interruptsEnabled;
    };

    const int N_CHANNELS_MAX = 16;

    // Module API
    int newChannel(Device device, Size size, uint32_t address=0x00000000, uint16_t length=0, bool ring=false);
    void enableInterrupt(int channel, void (*handler)(), Interrupt interrupt=Interrupt::TRANSFER_FINISHED);
    void disableInterrupt(int channel, Interrupt interrupt=Interrupt::TRANSFER_FINISHED);
    void setupChannel(int channel, uint32_t address, uint16_t length);
    void startChannel(int channel);
    void startChannel(int channel, uint32_t address, uint16_t length);
    void reloadChannel(int channel, uint32_t address, uint16_t length);
    void stopChannel(int channel);
    int getCounter(int channel);
    bool isEnabled(int channel);
    bool isFinished(int channel);
    bool isReloadEmpty(int channel);
    void enableRing(int channel);
    void disableRing(int channel);

}

#endif