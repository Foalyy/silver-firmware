#include "i2c.h"
#include "core.h"
#include "pm.h"
#include "dma.h"
#include "error.h"

namespace I2C {

    // Channel parameters
    const int BUFFER_SIZE = 64;
    enum class Mode {
        NONE,
        MASTER,
        SLAVE
    };
    struct Channel {
        Mode mode = Mode::NONE;
        uint8_t buffer[BUFFER_SIZE];
        int rxDMAChannel = -1;
        int txDMAChannel = -1;
        unsigned int nBytesToRead = 0;
        unsigned int nBytesToWrite = 0;
    };

    // List of available ports
    struct Channel ports[N_PORTS_M];

    // Interrupt handlers
    extern uint8_t INTERRUPT_PRIORITY;
    uint32_t _interruptHandlers[N_PORTS_M][N_INTERRUPTS];
    Core::Interrupt _interruptChannelsMaster[] = {Core::Interrupt::TWIM0, Core::Interrupt::TWIM1, Core::Interrupt::TWIM2, Core::Interrupt::TWIM3};
    Core::Interrupt _interruptChannelsSlave[] = {Core::Interrupt::TWIS0, Core::Interrupt::TWIS1};
    void interruptHandlerWrapper();

    // Clocks
    const int PM_CLK_M[] = {PM::CLK_I2CM0, PM::CLK_I2CM1, PM::CLK_I2CM2, PM::CLK_I2CM3}; // Master mode
    const int PM_CLK_S[] = {PM::CLK_I2CS0, PM::CLK_I2CS1}; // Slave mode

    // Package-dependant, defined in pins_sam4l_XX.cpp
    extern struct GPIO::Pin PINS_SDA[];
    extern struct GPIO::Pin PINS_SCL[];

    // Registers base address
    const uint32_t I2C_BASE[] = {0x40018000, 0x4001C000, 0x40078000, 0x4007C000};

    // I2C clock frequency
    unsigned int _frequency = 0;



    // MASTER MODE

    bool enableMaster(Port port, unsigned int frequency) {
        if (static_cast<int>(port) > N_PORTS_M) {
            return false;
        }
        const uint32_t REG_BASE = I2C_BASE[static_cast<int>(port)];
        struct Channel* p = &(ports[static_cast<int>(port)]);

        // If this port is already enabled in slave mode, disable it
        if (p->mode == Mode::SLAVE) {
            Error::happened(Error::Module::I2C, WARN_PORT_ALREADY_INITIALIZED, Error::Severity::WARNING);
            (*(volatile uint32_t*)(REG_BASE + OFFSET_S_CR)) = 0;
        }
        p->mode = Mode::MASTER;

        // Initialize the buffer
        for (int i = 0; i < BUFFER_SIZE; i++) {
            p->buffer[i] = 0;
        }

        // Initialize interrupt handlers
        for (int i = 0; i < N_INTERRUPTS; i++) {
            _interruptHandlers[static_cast<int>(port)][i] = (uint32_t)nullptr;
        }

        // Enable the clock
        PM::enablePeripheralClock(PM_CLK_M[static_cast<int>(port)]);

        // CR (Control Register) : reset the interface
        (*(volatile uint32_t*)(REG_BASE + OFFSET_M_CR))
            = 1 << M_CR_SWRST;        // SWRST : software reset

        // CR (Control Register) : enable the master interface
        (*(volatile uint32_t*)(REG_BASE + OFFSET_M_CR))
            = 1 << M_CR_MEN;          // MEN : Master Enable

        // CWGR (Clock Waveform Generator Register) : setup the SCL (clock) line
        _frequency = frequency;
        unsigned long fPrescaler = PM::getModuleClockFrequency(PM_CLK_M[static_cast<int>(port)]) / 2; // Prescaler at EXP=0 divides the freq. by 2
        unsigned int periods = fPrescaler / frequency;
        uint8_t t = periods / 2;
        (*(volatile uint32_t*)(REG_BASE + OFFSET_M_CWGR))
            = t << M_CWGR_LOW      // LOW : low and buffer counter
            | t << M_CWGR_HIGH     // HIGH : high counter
            | t << M_CWGR_STASTO   // STASTO : start/stop counters
            | 1 << M_CWGR_DATA     // DATA : data time counter
            | 0 << M_CWGR_EXP;     // EXP : clock prescaler

        // SRR (Slew Rate Register) : setup the lines
        // See Electrical Characteristics in the datasheet for more details
        (*(volatile uint32_t*)(REG_BASE + OFFSET_M_SRR))
            = 3 << M_SRR_DADRIVEL
            | 0 << M_SRR_DASLEW
            | 3 << M_SRR_CLDRIVEL
            | 0 << M_SRR_CLSLEW
            | 2 << M_SRR_FILTER;

        // Set up the DMA channels and related interrupts
        if (p->rxDMAChannel == -1) {
            p->rxDMAChannel = DMA::newChannel(static_cast<DMA::Device>(static_cast<int>(DMA::Device::I2C0_M_RX) + static_cast<int>(port)), DMA::Size::BYTE);
        }
        if (p->txDMAChannel == -1) {
            p->txDMAChannel = DMA::newChannel(static_cast<DMA::Device>(static_cast<int>(DMA::Device::I2C0_M_TX) + static_cast<int>(port)), DMA::Size::BYTE);
        }

        // Set the pins in peripheral mode
        GPIO::enablePeripheral(PINS_SDA[static_cast<int>(port)]);
        GPIO::enablePeripheral(PINS_SCL[static_cast<int>(port)]);

        return true;
    }

    // Internal function which checks if the controller has lost the bus arbitration
    // to another master. If no other master is present and this condition arises, 
    // this may be the sign of an electrical problem (short circuit or missing pull-ups).
    bool checkArbitrationLost(Port port) {
        struct Channel* p = &(ports[static_cast<int>(port)]);
        if (p->mode != Mode::MASTER) {
            Error::happened(Error::Module::I2C, ERR_PORT_NOT_INITIALIZED, Error::Severity::CRITICAL);
            return false;
        }

        const uint32_t REG_BASE = I2C_BASE[static_cast<int>(port)];
        if ((*(volatile uint32_t*)(REG_BASE + OFFSET_M_SR)) & (1 << M_SR_ARBLST)) {
            Error::happened(Error::Module::I2C, WARN_ARBITRATION_LOST, Error::Severity::WARNING);
            (*(volatile uint32_t*)(REG_BASE + OFFSET_M_CMDR)) = 0;
            (*(volatile uint32_t*)(REG_BASE + OFFSET_M_SCR)) = 1 << M_SR_ARBLST;
            return true;
        }
        return false;
    }

    // Try to send a read request to the specified address and return true if
    // a slave device has answered
    bool testAddress(Port port, uint8_t address, Dir direction) {
        struct Channel* p = &(ports[static_cast<int>(port)]);
        if (p->mode != Mode::MASTER) {
            Error::happened(Error::Module::I2C, ERR_PORT_NOT_INITIALIZED, Error::Severity::CRITICAL);
            return false;
        }
        const uint32_t REG_BASE = I2C_BASE[static_cast<int>(port)];
        if (checkArbitrationLost(port)) {
            return false;
        }

        // CMDR (Command Register) : initiate a transfer
        (*(volatile uint32_t*)(REG_BASE + OFFSET_M_CMDR))
            = !static_cast<int>(direction) << M_CMDR_READ
            | address << M_CMDR_SADR
            | 1 << M_CMDR_START
            | 1 << M_CMDR_STOP
            | 1 << M_CMDR_VALID
            | 0 << M_CMDR_NBYTES;

        // Wait for the transfer to complete or an Arbitration lost condition to happen
        Core::Time t0 = Core::time();
        while (!( (*(volatile uint32_t*)(REG_BASE + OFFSET_M_SR)) & (1 << M_SR_IDLE)
                || (*(volatile uint32_t*)(REG_BASE + OFFSET_M_SR)) & (1 << M_SR_ARBLST) )) {
            if (Core::time() > t0 + TIMEOUT) {
                Error::happened(Error::Module::I2C, ERR_TIMEOUT, Error::Severity::CRITICAL);
            }
            Core::sleep(1);
        }

        // Check for arbitration lost again now that the transfer is complete
        if (checkArbitrationLost(port)) {
            return false;
        }

        // If the ANAK status flag is set, no slave has answered
        if ((*(volatile uint32_t*)(REG_BASE + OFFSET_M_SR)) & (1 << M_SR_ANAK)) {

            // Clear the ANAK status flag and the command register
            (*(volatile uint32_t*)(REG_BASE + OFFSET_M_CMDR)) = 0;
            (*(volatile uint32_t*)(REG_BASE + OFFSET_M_SCR)) = 1 << M_SR_ANAK;

            return false;
        }
        (*(volatile uint32_t*)(REG_BASE + OFFSET_M_SCR)) = 1 << M_SR_CCOMP;
        return true;
    }

    // Master read
    int read(Port port, uint8_t address, uint8_t* buffer, int n) {
        struct Channel* p = &(ports[static_cast<int>(port)]);
        if (p->mode != Mode::MASTER) {
            Error::happened(Error::Module::I2C, ERR_PORT_NOT_INITIALIZED, Error::Severity::CRITICAL);
            return 0;
        }
        const uint32_t REG_BASE = I2C_BASE[static_cast<int>(port)];
        if (checkArbitrationLost(port)) {
            return 0;
        }

        // Clear every status
        (*(volatile uint32_t*)(REG_BASE + OFFSET_M_SCR)) = 0xFFFFFFFF;

        // Start the DMA RX channel
        DMA::startChannel(p->rxDMAChannel, (uint32_t)(buffer), n);

        // CMDR (Command Register) : initiate a read transfer
        (*(volatile uint32_t*)(REG_BASE + OFFSET_M_CMDR))
            = 1 << M_CMDR_READ
            | address << M_CMDR_SADR
            | 1 << M_CMDR_START
            | 1 << M_CMDR_STOP
            | 1 << M_CMDR_VALID
            | n << M_CMDR_NBYTES;

        // Wait for the transfer to be finished
        Core::Time t0 = Core::time();
        while (!DMA::isFinished(p->rxDMAChannel)
                && !((*(volatile uint32_t*)(REG_BASE + OFFSET_M_SR)) & (1 << M_SR_ANAK | 1 << M_SR_DNAK | 1 << M_SR_ARBLST))) {
            if (Core::time() > t0 + TIMEOUT) {
                Error::happened(Error::Module::I2C, ERR_TIMEOUT, Error::Severity::CRITICAL);
            }
            Core::sleep(1);
        }

        // Check for arbitration lost again now that the transfer is complete
        if (checkArbitrationLost(port)) {
            return false;
        }

        // If the slave has not responded, cancel the read
        if ((*(volatile uint32_t*)(REG_BASE + OFFSET_M_SR)) & 1 << M_SR_ANAK) {
            (*(volatile uint32_t*)(REG_BASE + OFFSET_M_CMDR)) = 0;
            (*(volatile uint32_t*)(REG_BASE + OFFSET_M_SCR)) = 1 << M_SR_ANAK;
            return 0;
        }

        return n - DMA::getCounter(p->rxDMAChannel);
    }

    // Helper function to read a single byte
    uint8_t read(Port port, uint8_t address) {
        uint8_t buffer[] = {0x00};
        read(port, address, buffer, 1);
        return buffer[0];
    }

    // Master write
    bool write(Port port, uint8_t address, const uint8_t* buffer, int n) {
        struct Channel* p = &(ports[static_cast<int>(port)]);
        if (p->mode != Mode::MASTER) {
            Error::happened(Error::Module::I2C, ERR_PORT_NOT_INITIALIZED, Error::Severity::CRITICAL);
            return false;
        }
        const uint32_t REG_BASE = I2C_BASE[static_cast<int>(port)];
        if (checkArbitrationLost(port)) {
            return false;
        }

        // Write at most BUFFER_SIZE characters
        if (n > BUFFER_SIZE) {
            n = BUFFER_SIZE;
        }

        // Copy the user buffer into the port buffer
        for (int i = 0; i < n; i++) {
            p->buffer[i] = buffer[i];
        }

        // Clear every status
        (*(volatile uint32_t*)(REG_BASE + OFFSET_M_SCR)) = 0xFFFFFFFF;

        // Copy the first byte to transmit
        (*(volatile uint32_t*)(REG_BASE + OFFSET_M_THR)) = p->buffer[0];

        // Start the DMA
        if (n >= 2) {
            DMA::startChannel(p->txDMAChannel, (uint32_t)(p->buffer + 1), n - 1);
        }

        // CMDR (Command Register) : initiate a write transfer
        (*(volatile uint32_t*)(REG_BASE + OFFSET_M_CMDR))
            = 0 << M_CMDR_READ
            | address << M_CMDR_SADR
            | 1 << M_CMDR_START
            | 1 << M_CMDR_STOP
            | 1 << M_CMDR_VALID
            | n << M_CMDR_NBYTES;

        // Wait for the transfer to be finished
        if (n >= 2) {
            Core::Time t0 = Core::time();
            while (!(DMA::isFinished(p->txDMAChannel) && (*(volatile uint32_t*)(REG_BASE + OFFSET_M_SR)) & (1 << M_SR_BUSFREE))
                    && !((*(volatile uint32_t*)(REG_BASE + OFFSET_M_SR)) & (1 << M_SR_ANAK | 1 << M_SR_DNAK | 1 << M_SR_ARBLST))) {
                if (Core::time() > t0 + TIMEOUT) {
                    Error::happened(Error::Module::I2C, ERR_TIMEOUT, Error::Severity::CRITICAL);
                }
                Core::sleep(1);
            }

        } else {
            Core::Time t0 = Core::time();
            while (!((*(volatile uint32_t*)(REG_BASE + OFFSET_M_SR)) & (1 << M_SR_IDLE))) {
                if (Core::time() > t0 + TIMEOUT) {
                    Error::happened(Error::Module::I2C, ERR_TIMEOUT, Error::Severity::CRITICAL);
                }
                Core::sleep(1);
            }
        }

        // Check for arbitration lost again now that the transfer is complete
        if (checkArbitrationLost(port)) {
            return false;
        }

        // Return true if the transfer was completed successfully
        return !((*(volatile uint32_t*)(REG_BASE + OFFSET_M_SR)) & (1 << M_SR_ANAK | 1 << M_SR_DNAK));
    }

    // Helper function to write a single byte
    bool write(Port port, uint8_t address, uint8_t byte) {
        return write(port, address, &byte, 1);
    }

    // Write then immediately read on the bus on the same transfer, with a Repeated Start condition.
    // This is especially useful for reading registers on devices by writing the register address
    // then reading the value
    bool writeRead(Port port, uint8_t address, const uint8_t* txBuffer, int nTX, uint8_t* rxBuffer, int nRX) {
        struct Channel* p = &(ports[static_cast<int>(port)]);
        if (p->mode != Mode::MASTER) {
            Error::happened(Error::Module::I2C, ERR_PORT_NOT_INITIALIZED, Error::Severity::CRITICAL);
            return false;
        }
        const uint32_t REG_BASE = I2C_BASE[static_cast<int>(port)];
        if (checkArbitrationLost(port)) {
            return false;
        }

        // Write at most BUFFER_SIZE characters
        if (nTX >  BUFFER_SIZE) {
            nTX = BUFFER_SIZE;
        }

        // Copy the user TX buffer into the port buffer
        for (int i = 0; i < nTX; i++) {
            p->buffer[i] = txBuffer[i];
        }

        // Clear every status
        (*(volatile uint32_t*)(REG_BASE + OFFSET_M_SCR)) = 0xFFFFFFFF;

        // Copy the first byte to transmit
        (*(volatile uint32_t*)(REG_BASE + OFFSET_M_THR)) = p->buffer[0];

        // Start the DMA TX channel
        if (nTX >= 2) {
            DMA::startChannel(p->txDMAChannel, (uint32_t)(p->buffer + 1), nTX - 1);
        }

        // Start the DMA RX channel
        DMA::startChannel(p->rxDMAChannel, (uint32_t)(rxBuffer), nRX);

        // CMDR (Command Register) : initiate a write transfer without STOP
        (*(volatile uint32_t*)(REG_BASE + OFFSET_M_CMDR))
            = 0 << M_CMDR_READ
            | address << M_CMDR_SADR
            | 1 << M_CMDR_START
            | 0 << M_CMDR_STOP
            | 1 << M_CMDR_VALID
            | nTX << M_CMDR_NBYTES;

        // NCMDR (Next Command Register) : initiate a read transfer to follow
        (*(volatile uint32_t*)(REG_BASE + OFFSET_M_NCMDR))
            = 1 << M_CMDR_READ
            | address << M_CMDR_SADR
            | 1 << M_CMDR_START
            | 1 << M_CMDR_STOP
            | 1 << M_CMDR_VALID
            | nRX << M_CMDR_NBYTES;

        // Wait for the transfer to be finished
        Core::Time t0 = Core::time();
        while (!DMA::isFinished(p->rxDMAChannel)
                && !((*(volatile uint32_t*)(REG_BASE + OFFSET_M_SR)) & (1 << M_SR_ANAK | 1 << M_SR_DNAK | 1 << M_SR_ARBLST))) {
            if (Core::time() > t0 + TIMEOUT) {
                Error::happened(Error::Module::I2C, ERR_TIMEOUT, Error::Severity::CRITICAL);
            }
            Core::sleep(1);
        }

        // Check for arbitration lost again now that the transfer is complete
        if (checkArbitrationLost(port)) {
            return false;
        }

        // If the slave has not responded, cancel the read
        if ((*(volatile uint32_t*)(REG_BASE + OFFSET_M_SR)) & 1 << M_SR_ANAK) {
            (*(volatile uint32_t*)(REG_BASE + OFFSET_M_CMDR)) = 0;
            (*(volatile uint32_t*)(REG_BASE + OFFSET_M_SCR)) = 1 << M_SR_ANAK;
            return 0;
        }

        return nRX - DMA::getCounter(p->rxDMAChannel);
    }

    // Helper function which writes a single byte then reads the result
    bool writeRead(Port port, uint8_t address, uint8_t byte, uint8_t* rxBuffer, int nRX) {
        return writeRead(port, address, &byte, 1, rxBuffer, nRX);
    }







    // SLAVE MODE

    bool enableSlave(Port port, uint8_t address) {
        if (static_cast<int>(port) > N_PORTS_S) {
            return false;
        }
        const uint32_t REG_BASE = I2C_BASE[static_cast<int>(port)];
        struct Channel* p = &(ports[static_cast<int>(port)]);

        // If this port is already enabled in master mode, disable it
        if (p->mode == Mode::MASTER) {
            Error::happened(Error::Module::I2C, WARN_PORT_ALREADY_INITIALIZED, Error::Severity::WARNING);
            (*(volatile uint32_t*)(REG_BASE + OFFSET_M_CR))
                = 1 << M_CR_MDIS
                | 1 << M_CR_STOP;
        }
        p->mode = Mode::SLAVE;

        // Initialize the buffer
        for (int i = 0; i < BUFFER_SIZE; i++) {
            p->buffer[i] = 0;
        }

        // Initialize interrupt handlers
        for (int i = 0; i < N_INTERRUPTS; i++) {
            _interruptHandlers[static_cast<int>(port)][i] = (uint32_t)nullptr;
        }

        // Enable the clock
        PM::enablePeripheralClock(PM_CLK_S[static_cast<int>(port)]);

        // CR (Control Register) : reset the interface
        (*(volatile uint32_t*)(REG_BASE + OFFSET_S_CR))
            = 1 << S_CR_SWRST;        // SWRST : software reset

        // CR (Control Register) : enable the slave interface
        (*(volatile uint32_t*)(REG_BASE + OFFSET_S_CR))
            = 1 << S_CR_SEN;          // SEN : Slave Enable

        // CR (Control Register) : configure the interface
        address &= 0x7F;
        (*(volatile uint32_t*)(REG_BASE + OFFSET_S_CR))
            = 1 << S_CR_SEN           // SEN : Slave Enable
            | 1 << S_CR_SMATCH        // SMATCH : Acknowledge the slave address
            | 0 << S_CR_STREN         // STREN : Clock stretch disabled
            | 1 << S_CR_CUP           // CUP : NBYTES Count Up
            | address << S_CR_ADR;    // ADDR : Slave Address

        // TR (Timing Register) : setup bus timings
        (*(volatile uint32_t*)(REG_BASE + OFFSET_S_TR))
            = 1 << S_TR_SUDAT;        // SUDAT : Data setup cycles

        // NBYTES : reset the bytes counter
        (*(volatile uint32_t*)(REG_BASE + OFFSET_S_NBYTES)) = 0;

        // Set up the DMA channels
        p->rxDMAChannel = DMA::newChannel(static_cast<DMA::Device>(static_cast<int>(DMA::Device::I2C0_S_RX) + static_cast<int>(port)), DMA::Size::BYTE);
        p->txDMAChannel = DMA::newChannel(static_cast<DMA::Device>(static_cast<int>(DMA::Device::I2C0_S_TX) + static_cast<int>(port)), DMA::Size::BYTE);

        // SRR (Slew Rate Register) : setup the lines
        // See Electrical Characteristics in the datasheet for more details
        (*(volatile uint32_t*)(REG_BASE + OFFSET_S_SRR))
            = 3 << S_SRR_DADRIVEL
            | 0 << S_SRR_DASLEW
            | 2 << S_SRR_FILTER;

        // IER (Interrupt Enable Register) : enable the Transfer Complete, Overrun and Underrun interrupts
        (*(volatile uint32_t*)(REG_BASE + OFFSET_S_IER))
                = 1 << S_SR_TCOMP
                | 1 << S_SR_URUN
                | 1 << S_SR_ORUN;

        // Enable the interrupt in the NVIC
        Core::Interrupt interruptChannel = _interruptChannelsSlave[static_cast<int>(port)];
        Core::setInterruptHandler(interruptChannel, interruptHandlerWrapper);
        Core::enableInterrupt(interruptChannel, INTERRUPT_PRIORITY);

        // Initialize the slave with an empty write
        (*(volatile uint32_t*)(REG_BASE + OFFSET_S_THR)) = 0xFF;

        // Set the pins in peripheral mode
        GPIO::enablePeripheral(PINS_SDA[static_cast<int>(port)]);
        GPIO::enablePeripheral(PINS_SCL[static_cast<int>(port)]);

        return true;
    }

    // Slave read
    int read(Port port, uint8_t* buffer, int n, bool async) {
        struct Channel* p = &(ports[static_cast<int>(port)]);
        if (p->mode != Mode::SLAVE) {
            Error::happened(Error::Module::I2C, ERR_PORT_NOT_INITIALIZED, Error::Severity::CRITICAL);
            return 0;
        }
        const uint32_t REG_BASE = I2C_BASE[static_cast<int>(port)];

        // Clear every status
        (*(volatile uint32_t*)(REG_BASE + OFFSET_S_SCR)) = 0xFFFFFFFF;

        // Dummy read to clear RHR in case of overrun
        (*(volatile uint32_t*)(REG_BASE + OFFSET_S_RHR));

        // NBYTES : reset the bytes counter
        (*(volatile uint32_t*)(REG_BASE + OFFSET_S_NBYTES)) = 0;

        // Save the number of bytes to read
        p->nBytesToRead = n;

        // Start the DMA RX channel
        DMA::startChannel(p->rxDMAChannel, (uint32_t)(buffer), n);

        if (async) {
            // In async mode, do not wait for the read to complete, it will be managed in background
            // by the DMA
            return 0;

        } else {
            // Wait for the transfer to be finished
            Core::Time t0 = Core::time();
            while (!((*(volatile uint32_t*)(REG_BASE + OFFSET_S_SR)) & (1 << S_SR_TCOMP | 1 << S_SR_BUSERR | 1 << S_SR_NAK))) {
                if (Core::time() > t0 + TIMEOUT) {
                    Error::happened(Error::Module::I2C, ERR_TIMEOUT, Error::Severity::CRITICAL);
                }
                Core::sleep(1);
            }

            return n - DMA::getCounter(p->rxDMAChannel);
        }
    }

    // Slave write
    bool write(Port port, const uint8_t* buffer, int n, bool async) {
        struct Channel* p = &(ports[static_cast<int>(port)]);
        if (p->mode != Mode::SLAVE) {
            Error::happened(Error::Module::I2C, ERR_PORT_NOT_INITIALIZED, Error::Severity::CRITICAL);
            return false;
        }
        bool ret = true;
        const uint32_t REG_BASE = I2C_BASE[static_cast<int>(port)];

        // Stop any previous transfer
        DMA::stopChannel(p->txDMAChannel);

        // NBYTES : reset the bytes counter
        (*(volatile uint32_t*)(REG_BASE + OFFSET_S_NBYTES)) = 0;

        // Empty transfer
        if (n == 0 || buffer == nullptr) {
            // Copy a dummy byte to THR to overwrite any previous byte waiting to be sent
            (*(volatile uint32_t*)(REG_BASE + OFFSET_S_THR)) = 0xFF;

            // Clear every status
            (*(volatile uint32_t*)(REG_BASE + OFFSET_S_SCR)) = 0xFFFFFFFF;

        } else {
            // Write at most BUFFER_SIZE - 1 + 1 = BUFFER_SIZE characters into the port buffer : 
            // - 1 to allow room for the 0xFF terminating byte (see below)
            // + 1 because the first byte in the user buffer is written directly to THR (see below)
            if (n >= BUFFER_SIZE) {
                n = BUFFER_SIZE;
                ret = false;
            }

            // Copy the first byte to THR to overwrite any previous byte waiting to be sent
            (*(volatile uint32_t*)(REG_BASE + OFFSET_S_THR)) = buffer[0];

            // Copy the rest of the user buffer into the port buffer
            for (int i = 0; i < n - 1; i++) {
                p->buffer[i] = buffer[i + 1];
            }

            // Save the number of bytes to write
            p->nBytesToWrite = n - 1;

            // Write a 0xFF at the end of the buffer
            // Since the last byte is repeated indefinitely when the master attempts to read
            // more bytes than were available, this forces the hardware to send only 0xFF bytes.
            p->buffer[n - 1] = 0xFF;
            n++;

            // Clear every status
            (*(volatile uint32_t*)(REG_BASE + OFFSET_S_SCR)) = 0xFFFFFFFF;

            // Start the DMA
            DMA::startChannel(p->txDMAChannel, (uint32_t)(p->buffer), n - 1);
        }

        if (async) {
            // In async mode, do not wait for the read to complete, it will be managed in background
            // by the DMA
            return ret;

        } else {
            // Wait for the transfer to be finished
            Core::Time t0 = Core::time();
            while (!((*(volatile uint32_t*)(REG_BASE + OFFSET_S_SR)) & (1 << S_SR_TCOMP | 1 << S_SR_BUSERR | 1 << S_SR_NAK))) {
                if (Core::time() > t0 + TIMEOUT) {
                    Error::happened(Error::Module::I2C, ERR_TIMEOUT, Error::Severity::CRITICAL);
                }
                Core::sleep(1);
            }

            return DMA::isFinished(p->txDMAChannel);
        }
    }

    bool isAsyncReadFinished(Port port) {
        struct Channel* p = &(ports[static_cast<int>(port)]);
        if (p->mode != Mode::SLAVE) {
            Error::happened(Error::Module::I2C, ERR_PORT_NOT_INITIALIZED, Error::Severity::CRITICAL);
            return false;
        }
        const uint32_t REG_BASE = I2C_BASE[static_cast<int>(port)];
        uint32_t status = *(volatile uint32_t*)(REG_BASE + OFFSET_S_SR);
        return !(status & 1 << S_SR_TRA) && (status & (1 << S_SR_TCOMP | 1 << S_SR_BUSERR | 1 << S_SR_NAK));
    }

    bool isAsyncWriteFinished(Port port) {
        struct Channel* p = &(ports[static_cast<int>(port)]);
        if (p->mode != Mode::SLAVE) {
            Error::happened(Error::Module::I2C, ERR_PORT_NOT_INITIALIZED, Error::Severity::CRITICAL);
            return false;
        }
        const uint32_t REG_BASE = I2C_BASE[static_cast<int>(port)];
        uint32_t status = *(volatile uint32_t*)(REG_BASE + OFFSET_S_SR);
        return (status & 1 << S_SR_TRA) && (status & (1 << S_SR_TCOMP | 1 << S_SR_BUSERR | 1 << S_SR_NAK));
    }

    // Get the number of bytes which have been read in the current async transfer
    int getAsyncReadCounter(Port port) {
        struct Channel* p = &(ports[static_cast<int>(port)]);
        if (p->mode != Mode::SLAVE) {
            Error::happened(Error::Module::I2C, ERR_PORT_NOT_INITIALIZED, Error::Severity::CRITICAL);
            return 0;
        }
        if (p->nBytesToRead > 0) {
            return p->nBytesToRead - DMA::getCounter(p->rxDMAChannel);
        }
        return 0;
    }

    // Get the number of bytes which have been sent by the master during a read transfer.
    // This will usually be equal to getAsyncReadCounter(), but may be greater in case the
    // master sent more bytes than the buffer allowed. Comparing the two values allows
    // to determine if data was lost during the transfer.
    int getAsyncReadCounterSent(Port port) {
        const uint32_t REG_BASE = I2C_BASE[static_cast<int>(port)];
        return *(volatile uint32_t*)(REG_BASE + OFFSET_S_NBYTES) & 0xFF;
    }

    // Get the number of bytes which have been written in the current async transfer
    int getAsyncWriteCounter(Port port) {
        struct Channel* p = &(ports[static_cast<int>(port)]);
        if (p->mode != Mode::SLAVE) {
            Error::happened(Error::Module::I2C, ERR_PORT_NOT_INITIALIZED, Error::Severity::CRITICAL);
            return 0;
        }
        if (p->nBytesToWrite > 0) {
            return p->nBytesToWrite - DMA::getCounter(p->txDMAChannel);
        }
        return 0;
    }

    void enableInterrupt(Port port, void (*handler)(), Interrupt interrupt) {
        struct Channel* p = &(ports[static_cast<int>(port)]);
        if (p->mode != Mode::SLAVE) {
            Error::happened(Error::Module::I2C, ERR_PORT_NOT_INITIALIZED, Error::Severity::CRITICAL);
            return;
        }

        // Save the user handler
        _interruptHandlers[static_cast<int>(port)][static_cast<int>(interrupt)] = (uint32_t)handler;
    }

    void interruptHandlerWrapper() {
        // Get the port through the current interrupt number
        Port port;
        Core::Interrupt currentInterrupt = Core::currentInterrupt();
        if (currentInterrupt == Core::Interrupt::TWIS0) {
            port = Port::I2C0;
        } else if (currentInterrupt == Core::Interrupt::TWIS1) {
            port = Port::I2C1;
        } else {
            return;
        }
        const uint32_t REG_BASE = I2C_BASE[static_cast<int>(port)];

        // URUN : underrun
        if ((*(volatile uint32_t*)(REG_BASE + OFFSET_S_SR)) & (1 << S_SR_URUN)) {
            // Write a dummy byte in THR to make sure the peripheral will not block
            // the bus waiting for data to transmit
            (*(volatile uint32_t*)(REG_BASE + OFFSET_S_THR)) = 0xFB;

            // SCR (Status Clear Register) : clear the interrupt
            (*(volatile uint32_t*)(REG_BASE + OFFSET_S_SCR))
                = 1 << S_SR_URUN;
        }

        // ORUN : overrun
        if ((*(volatile uint32_t*)(REG_BASE + OFFSET_S_SR)) & (1 << S_SR_ORUN)) {
            // Dummy read from RHR to make sure the peripheral will not block
            // the bus waiting for the received data to be handled
            (*(volatile uint32_t*)(REG_BASE + OFFSET_S_RHR));

            // SCR (Status Clear Register) : clear the interrupt
            (*(volatile uint32_t*)(REG_BASE + OFFSET_S_SCR))
                = 1 << S_SR_ORUN;
        }

        // TCOMP : Transfer Complete
        if ((*(volatile uint32_t*)(REG_BASE + OFFSET_S_SR)) & (1 << S_SR_TCOMP)) {
            // Call the user handler corresponding to this interrupt
            void (*handler)() = nullptr;
            if ((*(volatile uint32_t*)(REG_BASE + OFFSET_S_SR)) & (1 << S_SR_TRA)) {
                // Write finished
                handler = (void (*)())_interruptHandlers[static_cast<int>(port)][static_cast<int>(Interrupt::ASYNC_WRITE_FINISHED)];
                if (handler != nullptr) {
                    handler();
                }

            } else {
                // Read finished
                handler = (void (*)())_interruptHandlers[static_cast<int>(port)][static_cast<int>(Interrupt::ASYNC_READ_FINISHED)];
                if (handler != nullptr) {
                    handler();
                }
            }
            // SCR (Status Clear Register) : clear the interrupt
            (*(volatile uint32_t*)(REG_BASE + OFFSET_S_SCR))
                = 1 << S_SR_TCOMP;
        }
    }


    // COMMON TO MASTER AND SLAVE MODE

    // Disable the port and release its ressources
    void disable(Port port) {
        const uint32_t REG_BASE = I2C_BASE[static_cast<int>(port)];
        struct Channel* p = &(ports[static_cast<int>(port)]);
        
        // Free the pins in peripheral mode
        GPIO::disablePeripheral(PINS_SDA[static_cast<int>(port)]);
        GPIO::disablePeripheral(PINS_SCL[static_cast<int>(port)]);

        // Stop the DMA channels
        if (p->txDMAChannel > -1) {
            DMA::stopChannel(p->txDMAChannel);
        }
        if (p->rxDMAChannel > -1) {
            DMA::stopChannel(p->rxDMAChannel);
        }

        if (p->mode == Mode::MASTER) {
            // Disable the interrupt in the NVIC
            Core::Interrupt interruptChannel = _interruptChannelsMaster[static_cast<int>(port)];
            Core::disableInterrupt(interruptChannel);

            // CR (Control Register) : disable the master interface
            (*(volatile uint32_t*)(REG_BASE + OFFSET_M_CR)) = 0;

            // Disable the clock
            PM::disablePeripheralClock(PM_CLK_M[static_cast<int>(port)]);

        } else if (p->mode == Mode::SLAVE) {
            // Disable the interrupt in the NVIC
            Core::Interrupt interruptChannel = _interruptChannelsSlave[static_cast<int>(port)];
            Core::disableInterrupt(interruptChannel);

            // CR (Control Register) : disable the slave interface
            (*(volatile uint32_t*)(REG_BASE + OFFSET_S_CR)) = 0;

            // Disable the clock
            PM::disablePeripheralClock(PM_CLK_S[static_cast<int>(port)]);
        }

        p->mode = Mode::NONE;
    }

    // Advanced function which returns the raw Status Register.
    // See the datasheet §27.9.8 for more details.
    uint32_t getStatus(Port port) {
        const uint32_t REG_BASE = I2C_BASE[static_cast<int>(port)];
        struct Channel* p = &(ports[static_cast<int>(port)]);
        if (p->mode == Mode::MASTER) {
            return (*(volatile uint32_t*)(REG_BASE + OFFSET_M_SR));
        } else if (p->mode == Mode::SLAVE) {
            return (*(volatile uint32_t*)(REG_BASE + OFFSET_S_SR));
        }
        return 0;
    }

    void setPin(Port port, PinFunction function, GPIO::Pin pin) {
        switch (function) {
            case PinFunction::SDA:
                PINS_SDA[static_cast<int>(port)] = pin;
                break;

            case PinFunction::SCL:
                PINS_SCL[static_cast<int>(port)] = pin;
                break;
        }
    }

}
