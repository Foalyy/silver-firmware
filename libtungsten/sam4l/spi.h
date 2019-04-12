#ifndef _SPI_H_
#define _SPI_H_

#include <stdint.h>
#include "gpio.h"
#include "error.h"

// Serial Peripheral Interface
// This module allows the chip to communicate through an SPI interface,
// either as a Master or a Slave.
// SPI is faster than I2C, but uses more wires and can connect to less
// peripherals at the same time.
namespace SPI {

    // Peripheral memory space base addresses
    const uint32_t SPI_BASE = 0x40008000;

    // Register offsets
    const uint32_t OFFSET_CR =   0x00; // Control Register
    const uint32_t OFFSET_MR =   0x04; // Mode Register
    const uint32_t OFFSET_RDR =  0x08; // Receive Data Register
    const uint32_t OFFSET_TDR =  0x0C; // Transmit Data Register
    const uint32_t OFFSET_SR =   0x10; // Status Register
    const uint32_t OFFSET_IER =  0x14; // Interrupt Enable Register
    const uint32_t OFFSET_IDR =  0x18; // Interrupt Disable Register
    const uint32_t OFFSET_IMR =  0x1C; // Interrupt Mask Register
    const uint32_t OFFSET_CSR0 = 0x30; // Chip Select Register 0
    const uint32_t OFFSET_CSR1 = 0x34; // Chip Select Register 1
    const uint32_t OFFSET_CSR2 = 0x38; // Chip Select Register 2
    const uint32_t OFFSET_CSR3 = 0x3C; // Chip Select Register 3
    const uint32_t OFFSET_WPCR = 0xE4; // Write Protection Control Register
    const uint32_t OFFSET_WPSR = 0xE8; // Write Protection Status Register

    // Subregisters
    const uint8_t CR_SPIEN = 0;
    const uint8_t CR_SPIDIS = 1;
    const uint8_t CR_SWRST = 7;
    const uint8_t CR_FLUSHFIFO = 8;
    const uint8_t CR_LASTXFER = 24;
    const uint8_t MR_MSTR = 0;
    const uint8_t MR_PS = 1;
    const uint8_t MR_PCSDEC = 2;
    const uint8_t MR_MODFDIS = 4;
    const uint8_t MR_RXFIFOEN = 6;
    const uint8_t MR_LLB = 7;
    const uint8_t MR_PCS = 16;
    const uint8_t MR_DLYBCS = 24;
    const uint8_t TDR_TD = 0;
    const uint8_t TDR_PCS = 16;
    const uint8_t TDR_LASTXFER = 24;
    const uint8_t SR_RDRF = 0;
    const uint8_t SR_TDRE = 1;
    const uint8_t SR_MODF = 2;
    const uint8_t SR_OVRES = 3;
    const uint8_t SR_NSSR = 8;
    const uint8_t SR_TXEMPTY = 9;
    const uint8_t SR_UNDES = 10;
    const uint8_t SR_SPIENS = 16;
    const uint8_t CSR_CPOL = 0;
    const uint8_t CSR_NCPHA = 1;
    const uint8_t CSR_CSNAAT = 2;
    const uint8_t CSR_CSAAT = 3;
    const uint8_t CSR_BITS = 4;
    const uint8_t CSR_SCBR = 8;
    const uint8_t CSR_DLYBS = 16;
    const uint8_t CSR_DLYBCT = 24;
    const uint8_t WPCR_SPIWPEN = 0;
    
    // Constants
    const uint32_t WPCR_KEY = 0x535049 << 8;

    const int N_PERIPHERALS_MAX = 4;

    // Error codes
    const Error::Code ERR_INVALID_PERIPHERAL = 0x0001;
    const Error::Code ERR_PERIPHERAL_ALREADY_ENABLED = 0x0002;
    const Error::Code ERR_NOT_MASTER_MODE = 0x0003;
    const Error::Code ERR_NOT_SLAVE_MODE = 0x0004;
    
    // Static values
    enum class Mode {
        MODE0 = 0, // CPOL=0, CPHA=0
        MODE1 = 1, // CPOL=0, CPHA=1
        MODE2 = 2, // CPOL=1, CPHA=0
        MODE3 = 3  // CPOL=1, CPHA=1
    };

    using Peripheral = uint8_t;

    enum class PinFunction {
        MOSI,
        MISO,
        SCK,
        CS0,
        CS1,
        CS2,
        CS3
    };

    // Master-mode functions
    void enableMaster();
    bool addPeripheral(Peripheral peripheral, Mode mode=Mode::MODE0);
    uint8_t transfer(Peripheral peripheral, uint8_t tx=0, bool next=false);
    void transfer(Peripheral peripheral, uint8_t* txBuffer, int txBufferSize, uint8_t* rxBuffer=nullptr, int rxBufferSize=-1, bool partial=false);

    // Slave-mode functions
    void enableSlave(Mode mode=Mode::MODE0);
    void slaveTransfer(uint8_t* txBuffer=nullptr, int txBufferSize=-1);
    bool isSlaveTransferFinished();
    int slaveGetReceivedData(uint8_t* rxBuffer, int rxBufferSize);
    void enableSlaveTransferFinishedInterrupt(void (*handler)(int nReceivedBytes));
    void disableSlaveTransferFinishedInterrupt();

    // Common functions
    void disable();
    void setPin(PinFunction function, GPIO::Pin pin);

}


#endif