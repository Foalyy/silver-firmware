#ifndef _I2C_H_
#define _I2C_H_

#include <stdint.h>
#include "gpio.h"
#include "error.h"

// This module allows the chip to connect to an I2C bus, either in Master or Slave mode.
// I2C is sometimes called TWI (Two-Wire Interface).
// There are 4 distinct I2C controllers : 2 can be either Master or Slave, and 2 can be only Master.
// The peripheral can also operate in SMBus mode (which is a protocol based on I2C), even though this
// is not currently supported by the driver.
// I2C is slower than SPI, but can connect to a larger bus and uses less wires.
namespace I2C {

    // Peripheral memory space base addresses
    extern const uint32_t I2C_BASE[];

    // Register offsets (Master)
    const uint32_t OFFSET_M_CR =      0x00; // Control Register
    const uint32_t OFFSET_M_CWGR =    0x04; // Clock Waveform Generator Register
    const uint32_t OFFSET_M_SMBTR =   0x08; // SMBus Timing Register
    const uint32_t OFFSET_M_CMDR =    0x0C; // Command Register
    const uint32_t OFFSET_M_NCMDR =   0x10; // Next Command Register
    const uint32_t OFFSET_M_RHR =     0x14; // Receive Holding Register
    const uint32_t OFFSET_M_THR =     0x18; // Transmit Holding Register
    const uint32_t OFFSET_M_SR =      0x1C; // Status Register
    const uint32_t OFFSET_M_IER =     0x20; // Interrupt Enable Register
    const uint32_t OFFSET_M_IDR =     0x24; // Interrupt Disable Register
    const uint32_t OFFSET_M_IMR =     0x28; // Interrupt Mask Register
    const uint32_t OFFSET_M_SCR =     0x2C; // Status Clear Register
    const uint32_t OFFSET_M_PR =      0x30; // Parameter Register
    const uint32_t OFFSET_M_HSCWGR =  0x38; // HS-mode Clock Waveform Generator
    const uint32_t OFFSET_M_SRR =     0x3C; // Slew Rate Register
    const uint32_t OFFSET_M_HSSRR =   0x40; // HS-mode Slew Rate Register

    // Register offsets (Slave)
    const uint32_t OFFSET_S_CR =      0x400; // Control Register
    const uint32_t OFFSET_S_NBYTES =  0x404; // NBYTES Register
    const uint32_t OFFSET_S_TR =      0x408; // Timing Register
    const uint32_t OFFSET_S_RHR =     0x40C; // Receive Holding Register
    const uint32_t OFFSET_S_THR =     0x410; // Transmit Holding Register
    const uint32_t OFFSET_S_PECR =    0x414; // Packet Error Check Register
    const uint32_t OFFSET_S_SR =      0x418; // Status Register
    const uint32_t OFFSET_S_IER =     0x41C; // Interrupt Enable Register
    const uint32_t OFFSET_S_IDR =     0x420; // Interrupt Disable Register
    const uint32_t OFFSET_S_IMR =     0x424; // Interrupt Mask Register
    const uint32_t OFFSET_S_SCR =     0x428; // Status Clear Register
    const uint32_t OFFSET_S_PR =      0x42C; // Parameter Register
    const uint32_t OFFSET_S_HSTR =    0x434; // HS-mode Timing Register
    const uint32_t OFFSET_S_SRR =     0x438; // Slew Rate Register
    const uint32_t OFFSET_S_HSSRR =   0x43C; // HS-mode Slew Rate Register
    

    // Subregisters (Master)
    const uint8_t M_CR_MEN = 0;
    const uint8_t M_CR_MDIS = 1;
    const uint8_t M_CR_SMEN = 4;
    const uint8_t M_CR_SMDIS = 5;
    const uint8_t M_CR_SWRST = 7;
    const uint8_t M_CR_STOP = 8;
    const uint8_t M_CWGR_LOW = 0;
    const uint8_t M_CWGR_HIGH = 8;
    const uint8_t M_CWGR_STASTO = 16;
    const uint8_t M_CWGR_DATA = 24;
    const uint8_t M_CWGR_EXP = 28;
    const uint8_t M_CMDR_READ = 0;
    const uint8_t M_CMDR_SADR = 1;
    const uint8_t M_CMDR_TENBIT = 11;
    const uint8_t M_CMDR_REPSAME = 12;
    const uint8_t M_CMDR_START = 13;
    const uint8_t M_CMDR_STOP = 14;
    const uint8_t M_CMDR_VALID = 15;
    const uint8_t M_CMDR_NBYTES = 16;
    const uint8_t M_CMDR_PECEN = 24;
    const uint8_t M_CMDR_ACKLAST = 25;
    const uint8_t M_CMDR_HS = 26;
    const uint8_t M_CMDR_HSMCODE = 28;
    const uint8_t M_SR_RXRDY = 0;
    const uint8_t M_SR_TXRDY = 1;
    const uint8_t M_SR_CRDY = 2;
    const uint8_t M_SR_CCOMP = 3;
    const uint8_t M_SR_IDLE = 4;
    const uint8_t M_SR_BUSFREE = 5;
    const uint8_t M_SR_ANAK = 8;
    const uint8_t M_SR_DNAK = 9;
    const uint8_t M_SR_ARBLST = 10;
    const uint8_t M_SR_TOUT = 12;
    const uint8_t M_SR_PECERR = 13;
    const uint8_t M_SR_STOP = 14;
    const uint8_t M_SR_MENB = 16;
    const uint8_t M_SR_HSMCACK = 17;
    const uint8_t M_SRR_DADRIVEL = 0;
    const uint8_t M_SRR_DASLEW = 8;
    const uint8_t M_SRR_CLDRIVEL = 16;
    const uint8_t M_SRR_CLSLEW = 24;
    const uint8_t M_SRR_FILTER = 28;

    // Subregisters (Slave)
    const uint8_t S_CR_SEN = 0;
    const uint8_t S_CR_SMEN = 1;
    const uint8_t S_CR_SMATCH = 2;
    const uint8_t S_CR_GCMATCH = 3;
    const uint8_t S_CR_STREN = 4;
    const uint8_t S_CR_SWRST = 7;
    const uint8_t S_CR_SMDA = 9;
    const uint8_t S_CR_SMHH = 10;
    const uint8_t S_CR_PECEN = 11;
    const uint8_t S_CR_ACK = 12;
    const uint8_t S_CR_CUP = 13;
    const uint8_t S_CR_SOAM = 14;
    const uint8_t S_CR_SODR = 15;
    const uint8_t S_CR_ADR = 16;
    const uint8_t S_CR_TENBIT = 26;
    const uint8_t S_TR_TLOWS = 0;
    const uint8_t S_TR_TTOUT = 8;
    const uint8_t S_TR_SUDAT = 16;
    const uint8_t S_TR_EXP = 28;
    const uint8_t S_SR_RXRDY = 0;
    const uint8_t S_SR_TXRDY = 1;
    const uint8_t S_SR_SEN = 2;
    const uint8_t S_SR_TCOMP = 3;
    const uint8_t S_SR_TRA = 5;
    const uint8_t S_SR_URUN = 6;
    const uint8_t S_SR_ORUN = 7;
    const uint8_t S_SR_NAK = 8;
    const uint8_t S_SR_SMBTOUT = 12;
    const uint8_t S_SR_SMBPECERR = 13;
    const uint8_t S_SR_BUSERR = 14;
    const uint8_t S_SR_SAM = 16;
    const uint8_t S_SR_GCM = 17;
    const uint8_t S_SR_SMBHHM = 19;
    const uint8_t S_SR_SMBDAM = 20;
    const uint8_t S_SR_STO = 21;
    const uint8_t S_SR_REP = 22;
    const uint8_t S_SR_BTF = 23;
    const uint8_t S_SRR_DADRIVEL = 0;
    const uint8_t S_SRR_DASLEW = 8;
    const uint8_t S_SRR_FILTER = 28;
    
    // Ports
    const int N_PORTS_M = 4;
    const int N_PORTS_S = 2;
    enum class Port {
        I2C0,
        I2C1,
        I2C2,
        I2C3
    };

    enum Dir {
        READ,
        WRITE
    };

    enum class PinFunction {
        SDA,
        SCL
    };

    // Timeout for transfer operations
    const int TIMEOUT = 1000; // ms

    // Interrupts
    const int N_INTERRUPTS = 2;
    enum class Interrupt {
        ASYNC_READ_FINISHED,
        ASYNC_WRITE_FINISHED,
    };

    // Error codes
    const Error::Code WARN_PORT_ALREADY_INITIALIZED = 1;
    const Error::Code WARN_ARBITRATION_LOST = 2;
    const Error::Code ERR_PORT_NOT_INITIALIZED = 3;
    const Error::Code ERR_TIMEOUT = 4;


    // Common functions
    void disable(Port port);
    uint32_t getStatus(Port port);
    void setPin(Port port, PinFunction function, GPIO::Pin pin);

    // Master-mode functions
    bool enableMaster(Port port, unsigned int frequency=100000);
    int read(Port port, uint8_t address, uint8_t* buffer, int n);
    uint8_t read(Port port, uint8_t address);
    bool write(Port port, uint8_t address, const uint8_t* buffer, int n);
    bool write(Port port, uint8_t address, uint8_t byte);
    bool writeRead(Port port, uint8_t address, const uint8_t* txBuffer, int nTX, uint8_t* rxBuffer, int nRX);
    bool writeRead(Port port, uint8_t address, uint8_t byte, uint8_t* rxBuffer, int nRX);
    bool testAddress(Port port, uint8_t address, Dir direction);

    // Slave-mode functions
    bool enableSlave(Port port, uint8_t address);
    int read(Port port, uint8_t* buffer, int n, bool async=false);
    bool write(Port port, const uint8_t* buffer, int n, bool async=false);
    bool isAsyncReadFinished(Port port);
    bool isAsyncWriteFinished(Port port);
    int getAsyncReadCounter(Port port);
    int getAsyncReadBytesSent(Port port);
    int getAsyncWriteCounter(Port port);
    void enableInterrupt(Port port, void (*handler)(), Interrupt interrupt);

}


#endif