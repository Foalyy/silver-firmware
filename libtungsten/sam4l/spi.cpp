#include "spi.h"
#include "pm.h"
#include "dma.h"
#include "gpio.h"
#include "core.h"
#include "error.h"
#include <string.h>

namespace SPI {

    // Package-dependant, defined in pins_sam4l_XX.cpp
    extern struct GPIO::Pin PIN_MOSI;
    extern struct GPIO::Pin PIN_MISO;
    extern struct GPIO::Pin PIN_SCK;
    extern struct GPIO::Pin PIN_NPCS0;
    extern struct GPIO::Pin PIN_NPCS1;
    extern struct GPIO::Pin PIN_NPCS2;
    extern struct GPIO::Pin PIN_NPCS3;

    bool _enabled = false;
    bool _modeMaster = false;
    int _rxDMAChannel = -1;
    int _txDMAChannel = -1;
    bool _enabledPeripherals[N_PERIPHERALS_MAX] = {false, false, false, false};

    // Used for the DMA channel
    const int DUMMY_BYTES_SIZE = 64;
    uint8_t DUMMY_BYTES[DUMMY_BYTES_SIZE];
    int _dummyBytesCounter = 0;

    // Slave mode
    const int SLAVE_BUFFERS_SIZE = 128;
    uint8_t _slaveTXBuffer[SLAVE_BUFFERS_SIZE];
    uint8_t _slaveRXBuffer[SLAVE_BUFFERS_SIZE];
    int _slaveTransferSize = 0;
    bool _slaveTransferFinished = false;
    void (*_slaveTransferFinishedHandler)(int nReceivedBytes) = nullptr;
    extern uint8_t INTERRUPT_PRIORITY;


    // Internal functions
    void txDMAReloadEmptyHandler();
    void interruptHandlerWrapper();


    void disable() {
        _enabled = false;
        
        // Free the pins
        GPIO::disablePeripheral(PIN_MISO);
        GPIO::disablePeripheral(PIN_MOSI);
        GPIO::disablePeripheral(PIN_SCK);
        if (_enabledPeripherals[0]) {
            GPIO::enablePeripheral(PIN_NPCS0);
            _enabledPeripherals[0] = false;
        }
        if (_enabledPeripherals[1]) {
            GPIO::enablePeripheral(PIN_NPCS1);
            _enabledPeripherals[1] = false;
        }
        if (_enabledPeripherals[2]) {
            GPIO::enablePeripheral(PIN_NPCS2);
            _enabledPeripherals[2] = false;
        }
        if (_enabledPeripherals[3]) {
            GPIO::enablePeripheral(PIN_NPCS3);
            _enabledPeripherals[3] = false;
        }

        // MR (Mode Register) : deconfigure the interface
        (*(volatile uint32_t*)(SPI_BASE + OFFSET_MR)) = 0;

        // CR (Control Register) : reset the interface
        (*(volatile uint32_t*)(SPI_BASE + OFFSET_CR))
            = 1 << CR_SWRST;        // SWRST : software reset

        // CR (Control Register) : disable the interface
        (*(volatile uint32_t*)(SPI_BASE + OFFSET_CR))
            = 1 << CR_SPIDIS;       // SPIDIS : SPI Disable

        // Disable the clock
        PM::disablePeripheralClock(PM::CLK_SPI);
    }

    void setPin(PinFunction function, GPIO::Pin pin) {
        switch (function) {
            case PinFunction::MOSI:
                PIN_MOSI = pin;
                break;

            case PinFunction::MISO:
                PIN_MISO = pin;
                break;

            case PinFunction::SCK:
                PIN_SCK = pin;
                break;

            case PinFunction::CS0:
                PIN_NPCS0 = pin;
                break;

            case PinFunction::CS1:
                PIN_NPCS1 = pin;
                break;

            case PinFunction::CS2:
                PIN_NPCS2 = pin;
                break;

            case PinFunction::CS3:
                PIN_NPCS3 = pin;
                break;
        }
    }


    void enableMaster() {
        // If the controller is already enabled, disable it first
        if (_enabled) {
            disable();
        }
        _enabled = true;
        _modeMaster = true;

        // Initialize the dummy bytes buffer to 0
        memset(DUMMY_BYTES, 0, sizeof(DUMMY_BYTES));
        
        // Set the pins in peripheral mode
        GPIO::enablePeripheral(PIN_MISO);
        GPIO::enablePeripheral(PIN_MOSI);
        GPIO::enablePeripheral(PIN_SCK);

        // Enable the clock
        PM::enablePeripheralClock(PM::CLK_SPI);

        // CR (Control Register) : reset the interface
        (*(volatile uint32_t*)(SPI_BASE + OFFSET_CR))
            = 1 << CR_SWRST;        // SWRST : software reset

        // MR (Mode Register) : configure the interface in master mode
        (*(volatile uint32_t*)(SPI_BASE + OFFSET_MR))
            = 1 << MR_MSTR          // MSTR : master mode
            | 0 << MR_PS            // PS : fixed peripheral select
            | 0 << MR_PCSDEC        // PCSDEC : no CS decoding
            | 1 << MR_MODFDIS       // MODFDIS : mode fault detection disabled
            | 1 << MR_RXFIFOEN      // RXFIFOEN : reception fifo enabled
            | 0 << MR_LLB           // LLB : local loopback disabled
            | 0 << MR_PCS           // PCS : will be programmed for each transfer
            | 6 << MR_DLYBCS;       // DLYBCS : delay between chip selects : 6 periods of CLK_SPI

        // CR (Control Register) : enable the interface
        (*(volatile uint32_t*)(SPI_BASE + OFFSET_CR))
            = 1 << CR_SPIEN;        // SPIEN : SPI Enable

        // Set up the DMA channels and related interrupts
        if (_rxDMAChannel < 0) {
            _rxDMAChannel = DMA::newChannel(DMA::Device::SPI_RX, DMA::Size::BYTE);
        }
        if (_txDMAChannel < 0) {
            _txDMAChannel = DMA::newChannel(DMA::Device::SPI_TX, DMA::Size::BYTE);
        }
    }

    bool addPeripheral(Peripheral peripheral, Mode mode) {
        // Make sure the controller is enabled in master mode
        if (!_enabled) {
            enableMaster();
        }
        if (!_modeMaster) {
            Error::happened(Error::Module::SPI, ERR_NOT_MASTER_MODE, Error::Severity::CRITICAL);
            return false;
        }

        if (peripheral >= N_PERIPHERALS_MAX) {
            Error::happened(Error::Module::SPI, ERR_INVALID_PERIPHERAL, Error::Severity::CRITICAL);
            return false;
        } else if (_enabledPeripherals[peripheral]) {
            Error::happened(Error::Module::SPI, ERR_PERIPHERAL_ALREADY_ENABLED, Error::Severity::CRITICAL);
            return false;
        }
        _enabledPeripherals[peripheral] = true;

        switch (peripheral) {
            case 0:
                GPIO::enablePeripheral(PIN_NPCS0);
                break;

            case 1:
                GPIO::enablePeripheral(PIN_NPCS1);
                break;

            case 2:
                GPIO::enablePeripheral(PIN_NPCS2);
                break;

            case 3:
                GPIO::enablePeripheral(PIN_NPCS3);
                break;
        }

        // SPI mode
        uint8_t cpol = static_cast<int>(mode) & 0b10;
        uint8_t ncpha = !(static_cast<int>(mode) & 0b01);

        // CSRn (Chip Select Register n) : configure the peripheral-specific settings
        (*(volatile uint32_t*)(SPI_BASE + OFFSET_CSR0 + peripheral * 0x04))
            = cpol << CSR_CPOL      // CPOL : clock polarity
            | ncpha << CSR_NCPHA    // CPHA : clock phase
            | 0 << CSR_CSNAAT       // CSNAAT : CS doesn't rise between two consecutive transfers
            | 0 << CSR_CSAAT        // CSAAT : CS always rises when the last transfer is complete
            | 0b0000 << CSR_BITS    // BITS : 8 bits per transfer
            | 4 << CSR_SCBR         // SCBR : SPI clock = CLK_SPI / 4 (not faster, otherwise the DMA/some code won't be able to keep up)
            | 0 << CSR_DLYBS        // DLYBS : no delay between CS assertion and first clock cycle
            | 0 << CSR_DLYBCT;      // DLYBCT : no delay between consecutive transfers

        return true;
    }

    uint8_t transfer(Peripheral peripheral, uint8_t tx, bool next) {
        // Make sure the controller is enabled in master mode
        if (!_enabled || !_modeMaster) {
            Error::happened(Error::Module::SPI, ERR_NOT_MASTER_MODE, Error::Severity::CRITICAL);
            return 0;
        }

        // Select the peripheral
        uint8_t pcs = ~(1 << peripheral) & 0x0F;
        uint32_t mr = (*(volatile uint32_t*)(SPI_BASE + OFFSET_MR));
        mr = mr & ~((uint32_t)(0b1111 << MR_PCS)); // Erase the PCS field
        (*(volatile uint32_t*)(SPI_BASE + OFFSET_MR)) = mr | (pcs << MR_PCS); // Reprogram MR

        // Dummy reads to empty the Rx register
        while ((*(volatile uint32_t*)(SPI_BASE + OFFSET_SR)) >> SR_RDRF & 1) {
            (*(volatile uint32_t*)(SPI_BASE + OFFSET_RDR));
        }

        // Write the data to the transfer register
        (*(volatile uint32_t*)(SPI_BASE + OFFSET_TDR))
            = tx << TDR_TD;         // TD : transmit data

        // If the user has asked the next read byte, write a second byte immediately
        if (next) {
            // Wait for the byte to be transfered to the serializer
            while (!((*(volatile uint32_t*)(SPI_BASE + OFFSET_SR)) >> SR_TDRE & 1));

            // Transmit dummy byte
            (*(volatile uint32_t*)(SPI_BASE + OFFSET_TDR))
                = 0 << TDR_TD;

            // Wait for the end of the transfer
            while (!((*(volatile uint32_t*)(SPI_BASE + OFFSET_SR)) >> SR_RDRF & 1));

            // Dummy read to received data to prevent overrun condition
            (*(volatile uint32_t*)(SPI_BASE + OFFSET_RDR));
        }

        // Wait for the end of the transfer
        while (!((*(volatile uint32_t*)(SPI_BASE + OFFSET_SR)) >> SR_RDRF & 1));

        return (*(volatile uint32_t*)(SPI_BASE + OFFSET_RDR));
    }

    void transfer(Peripheral peripheral, uint8_t* txBuffer, int txBufferSize, uint8_t* rxBuffer, int rxBufferSize, bool partial) {
        // Make sure the controller is enabled in master mode
        if (!_enabled || !_modeMaster) {
            Error::happened(Error::Module::SPI, ERR_NOT_MASTER_MODE, Error::Severity::CRITICAL);
            return;
        }

        // Select the peripheral
        uint8_t pcs = ~(1 << peripheral) & 0x0F;
        uint32_t mr = (*(volatile uint32_t*)(SPI_BASE + OFFSET_MR));
        mr = mr & ~((uint32_t)(0b1111 << MR_PCS)); // Erase the PCS field
        (*(volatile uint32_t*)(SPI_BASE + OFFSET_MR)) = mr | (pcs << MR_PCS); // Reprogram MR

        // If this is a partial transfer, do not deselect the device
        uint32_t csr = (*(volatile uint32_t*)(SPI_BASE + OFFSET_CSR0 + peripheral * 0x04));
        if (partial) {
            // Enable CSAAT to prevent CS from rising automatically
            (*(volatile uint32_t*)(SPI_BASE + OFFSET_CSR0 + peripheral * 0x04)) = csr | 1 << CSR_CSAAT;
        } else {
            // Disable CSAAT to make CS rise automatically
            (*(volatile uint32_t*)(SPI_BASE + OFFSET_CSR0 + peripheral * 0x04)) = csr & ~(uint32_t)(1 << CSR_CSAAT);
        }

        // Dummy reads to empty the Rx register
        while ((*(volatile uint32_t*)(SPI_BASE + OFFSET_SR)) >> SR_RDRF & 1) {
            (*(volatile uint32_t*)(SPI_BASE + OFFSET_RDR));
        }

        // Enable Rx DMA channel
        if (rxBuffer && rxBufferSize > 0) {
            DMA::startChannel(_rxDMAChannel, (uint32_t)rxBuffer, rxBufferSize);
        } else {
            rxBufferSize = 0;
        }

        if (rxBufferSize == txBufferSize) {
            // Enable Tx DMA channel
            DMA::startChannel(_txDMAChannel, (uint32_t)txBuffer, txBufferSize);

            // Wait for the end of the transfer
            while (!(DMA::isFinished(_rxDMAChannel) && DMA::isFinished(_txDMAChannel)));
            while (!((*(volatile uint32_t*)(SPI_BASE + OFFSET_SR)) >> SR_TDRE & 1));

        // If the user asked to have more bytes received than sent, send dummy bytes
        } else if (rxBufferSize > txBufferSize) {
            // If there is no bytes to send, directly send dummy bytes instead
            if (txBuffer == nullptr || txBufferSize == 0) {
                txBuffer = DUMMY_BYTES;
                txBufferSize = rxBufferSize;
                if (txBufferSize > DUMMY_BYTES_SIZE) {
                    txBufferSize = DUMMY_BYTES_SIZE;
                }
            }

            // Configure Tx DMA channel without starting it
            DMA::setupChannel(_txDMAChannel, (uint32_t)txBuffer, txBufferSize);

            // After the channel has finished, reload it with some dummy bytes
            int dummySize = DUMMY_BYTES_SIZE;
            if (rxBufferSize - txBufferSize < DUMMY_BYTES_SIZE) {
                dummySize = rxBufferSize - txBufferSize;
            }
            DMA::reloadChannel(_txDMAChannel, (uint32_t)DUMMY_BYTES, dummySize);

            // Enable Tx DMA interrupt
            _dummyBytesCounter = rxBufferSize - txBufferSize - dummySize;
            if (_dummyBytesCounter > 0) {
                DMA::enableInterrupt(_txDMAChannel, txDMAReloadEmptyHandler, DMA::Interrupt::RELOAD_EMPTY);
            }

            // Start Tx DMA channel
            DMA::startChannel(_txDMAChannel);

            // Wait for the end of the Rx channel
            while (!(DMA::isFinished(_rxDMAChannel)));

            // Disable Tx DMA interrupt
            DMA::disableInterrupt(_txDMAChannel);

        // If the user asked to have more bytes sent than received, clear the RDR and the Underrun status
        } else if (rxBufferSize < txBufferSize) {
            // Enable Tx DMA channel
            DMA::startChannel(_txDMAChannel, uint32_t(txBuffer), txBufferSize);

            // Wait for the end of the transfer
            while (!DMA::isFinished(_txDMAChannel));
            while (!((*(volatile uint32_t*)(SPI_BASE + OFFSET_SR)) >> SR_TDRE & 1));

            // Dummy reads to reset the registers
            while ((*(volatile uint32_t*)(SPI_BASE + OFFSET_SR)) >> SR_RDRF & 1) {
                (*(volatile uint32_t*)(SPI_BASE + OFFSET_RDR));
            }
            (*(volatile uint32_t*)(SPI_BASE + OFFSET_SR));
        }
    }

    void txDMAReloadEmptyHandler() {
        if (_dummyBytesCounter > 0) {
            // Reload the Tx DMA channel with up to 8 dummy bytes
            int size = DUMMY_BYTES_SIZE;
            if (_dummyBytesCounter < DUMMY_BYTES_SIZE) {
                size = _dummyBytesCounter;
            }
            DMA::reloadChannel(_txDMAChannel, (uint32_t)DUMMY_BYTES, size);
            _dummyBytesCounter -= size;

        } else {
            DMA::disableInterrupt(_txDMAChannel, DMA::Interrupt::RELOAD_EMPTY);
        }
    }

    void enableSlave(Mode mode) {
        // If the controller is already enabled, disable it first
        if (_enabled) {
            disable();
        }
        _enabled = true;
        _modeMaster = false;

        // Initialize the dummy bytes buffer to 0
        memset(DUMMY_BYTES, 0, sizeof(DUMMY_BYTES));
        
        // Set the pins in peripheral mode
        GPIO::enablePeripheral(PIN_MISO);
        GPIO::enablePeripheral(PIN_MOSI);
        GPIO::enablePeripheral(PIN_SCK);
        GPIO::enablePeripheral(PIN_NPCS0); // NSS is the NPCS0 pin

        // Enable the clock
        PM::enablePeripheralClock(PM::CLK_SPI);

        // CR (Control Register) : reset the interface
        (*(volatile uint32_t*)(SPI_BASE + OFFSET_CR))
            = 1 << CR_SWRST;        // SWRST : software reset

        // MR (Mode Register) : configure the interface in slave mode
        (*(volatile uint32_t*)(SPI_BASE + OFFSET_MR))
            = 0 << MR_MSTR          // MSTR : slave mode
            | 0 << MR_PS            // PS : fixed peripheral select
            | 0 << MR_PCSDEC        // PCSDEC : no CS decoding
            | 1 << MR_MODFDIS       // MODFDIS : mode fault detection disabled
            | 1 << MR_RXFIFOEN      // RXFIFOEN : reception fifo enabled
            | 0 << MR_LLB           // LLB : local loopback disabled
            | 0 << MR_PCS           // PCS : will be programmed for each transfer
            | 6 << MR_DLYBCS;       // DLYBCS : delay between chip selects : 6 periods of CLK_SPI

        // CR (Control Register) : enable the interface
        (*(volatile uint32_t*)(SPI_BASE + OFFSET_CR))
            = 1 << CR_SPIEN;        // SPIEN : SPI Enable

        // Set up the DMA channels and related interrupts
        if (_rxDMAChannel < 0) {
            _rxDMAChannel = DMA::newChannel(DMA::Device::SPI_RX, DMA::Size::BYTE);
        }
        if (_txDMAChannel < 0) {
            _txDMAChannel = DMA::newChannel(DMA::Device::SPI_TX, DMA::Size::BYTE);
        }

        // SPI mode
        uint8_t cpol = static_cast<int>(mode) & 0b10;
        uint8_t ncpha = !(static_cast<int>(mode) & 0b01);

        // CSR0 (Chip Select Register 0) : configure the slave mode settings
        (*(volatile uint32_t*)(SPI_BASE + OFFSET_CSR0))
            = cpol << CSR_CPOL      // CPOL : clock polarity
            | ncpha << CSR_NCPHA    // CPHA : clock phase
            | 0b0000 << CSR_BITS;    // BITS : 8 bits per transfer
    }

    void slaveTransfer(uint8_t* txBuffer, int txBufferSize) {
        // Make sure the controller is enabled in slave mode
        if (!_enabled || _modeMaster) {
            Error::happened(Error::Module::SPI, ERR_NOT_SLAVE_MODE, Error::Severity::CRITICAL);
            return;
        }

        // Dummy reads to empty the Rx register
        while ((*(volatile uint32_t*)(SPI_BASE + OFFSET_SR)) >> SR_RDRF & 1) {
            (*(volatile uint32_t*)(SPI_BASE + OFFSET_RDR));
        }

        // Reset the transfer finished flag
        _slaveTransferFinished = false;

        if (txBuffer && txBufferSize > 0) {
            // Copy the first byte of data to write directly into TDR to overwrite any previous transfer
            // still waiting to be sent
            (*(volatile uint32_t*)(SPI_BASE + OFFSET_TDR))
                = txBuffer[0] << TDR_TD;

            // Copy the rest of the data into the tx buffer
            _slaveTransferSize = txBufferSize;
            if (_slaveTransferSize > SLAVE_BUFFERS_SIZE) {
                _slaveTransferSize = SLAVE_BUFFERS_SIZE;
            }
            if (txBufferSize >= 2) {
                memcpy(_slaveTXBuffer, txBuffer + 1, _slaveTransferSize - 1);
            }

            // Since the last transfered byte is repeated indefinitely when the master
            // keeps requesting data, make sure this is a null byte
            _slaveTXBuffer[_slaveTransferSize - 1] = 0x00;

            // Enable Tx DMA channel
            // '+ 1' because the null byte at the end of the buffer must be sent
            // '- 1' because the first byte is already written in TDR
            DMA::startChannel(_txDMAChannel, (uint32_t)_slaveTXBuffer, _slaveTransferSize + 1 - 1);

        } else {
            // Rx-only transfer, send dummy bytes
            _slaveTransferSize = 0;
            (*(volatile uint32_t*)(SPI_BASE + OFFSET_TDR))
                = 0x00 << TDR_TD;
        }

        // Enable Rx DMA channel
        DMA::startChannel(_rxDMAChannel, (uint32_t)_slaveRXBuffer, SLAVE_BUFFERS_SIZE);
    }

    bool isSlaveTransferFinished() {
        // Make sure the controller is enabled in slave mode
        if (!_enabled || _modeMaster) {
            Error::happened(Error::Module::SPI, ERR_NOT_SLAVE_MODE, Error::Severity::CRITICAL);
            return false;
        }

        // Check in SR.NSSR if a rising edge on NSS has been detected since the last call to slaveTransfer()
        _slaveTransferFinished = _slaveTransferFinished || ((*(volatile uint32_t*)(SPI_BASE + OFFSET_SR)) >> SR_NSSR & 1);

        return _slaveTransferFinished;
    }

    int slaveGetReceivedData(uint8_t* rxBuffer, int rxBufferSize) {
        // Make sure the controller is enabled in slave mode
        if (!_enabled || _modeMaster) {
            Error::happened(Error::Module::SPI, ERR_NOT_SLAVE_MODE, Error::Severity::CRITICAL);
            return 0;
        }

        // Get the number of bytes received from the DMA counter and copy the received bytes into the user buffer
        int bytesReceived = SLAVE_BUFFERS_SIZE - DMA::getCounter(_rxDMAChannel);
        if (rxBufferSize > bytesReceived) {
            rxBufferSize = bytesReceived;
        }
        memcpy(rxBuffer, _slaveRXBuffer, rxBufferSize);
        return rxBufferSize;
    }

    void enableSlaveTransferFinishedInterrupt(void (*handler)(int nReceivedBytes)) {
        // Save the user handler
        _slaveTransferFinishedHandler = handler;

        // IER (Interrupt Enable Register) : enable the interrupt
        (*(volatile uint32_t*)(SPI_BASE + OFFSET_IER))
                = 1 << SR_NSSR;

        // Enable the interrupt in the NVIC
        Core::setInterruptHandler(Core::Interrupt::SPI, interruptHandlerWrapper);
        Core::enableInterrupt(Core::Interrupt::SPI, INTERRUPT_PRIORITY);
    }

    void disableSlaveTransferFinishedInterrupt() {
        // IDR (Interrupt Disable Register) : disable the interrupt
        (*(volatile uint32_t*)(SPI_BASE + OFFSET_IDR))
                = 1 << SR_NSSR;

        Core::disableInterrupt(Core::Interrupt::SPI);
    }

    void interruptHandlerWrapper() {
        // Call the user handler
        if (_slaveTransferFinishedHandler) {
            _slaveTransferFinishedHandler(SLAVE_BUFFERS_SIZE - DMA::getCounter(_rxDMAChannel));
        }

        // SR (Status Register) : dummy read to clear the interrupt
        (*(volatile uint32_t*)(SPI_BASE + OFFSET_SR));
    }

}