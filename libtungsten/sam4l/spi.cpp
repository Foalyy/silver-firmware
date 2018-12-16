#include "spi.h"
#include "pm.h"
#include "dma.h"
#include "gpio.h"
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

    int _rxDMAChannel = -1;
    int _txDMAChannel = -1;
    const int N_SLAVES_MAX = 4;
    bool _enabledSlaves[N_SLAVES_MAX] = {false, false, false, false};

    // Used for the DMA channel
    const int DUMMY_BYTES_SIZE = 64;
    uint8_t DUMMY_BYTES[DUMMY_BYTES_SIZE];
    int _dummyBytesCounter = 0;

    volatile bool _txDMAChannelFinished = false;

    // Internal functions
    void txDMAReloadEmptyHandler();


    void enableMaster() {
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
        _rxDMAChannel = DMA::newChannel(DMA::Device::SPI_RX, DMA::Size::BYTE);
        _txDMAChannel = DMA::newChannel(DMA::Device::SPI_TX, DMA::Size::BYTE);
    }

    void disableMaster() {
        // Free the pins
        GPIO::disablePeripheral(PIN_MISO);
        GPIO::disablePeripheral(PIN_MOSI);
        GPIO::disablePeripheral(PIN_SCK);

        // MR (Mode Register) : deconfigure the interface
        (*(volatile uint32_t*)(SPI_BASE + OFFSET_MR)) = 0;

        // CR (Control Register) : disable the interface
        (*(volatile uint32_t*)(SPI_BASE + OFFSET_CR))
            = 1 << CR_SPIDIS;       // SPIDIS : SPI Disable

        // Disable the clock
        PM::disablePeripheralClock(PM::CLK_SPI);
    }

    bool enableSlave(Slave slave, Mode mode) {
        if (slave >= N_SLAVES_MAX) {
            Error::happened(Error::Module::SPI, ERR_INVALID_SLAVE, Error::Severity::CRITICAL);
            return false;
        } else if (_enabledSlaves[slave]) {
            Error::happened(Error::Module::SPI, ERR_SLAVE_ALREADY_ENABLED, Error::Severity::CRITICAL);
            return false;
        }
        _enabledSlaves[slave] = true;

        switch (slave) {
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

        // CSRn (Chip Select Register n) : configure the slave-specific settings
        (*(volatile uint32_t*)(SPI_BASE + OFFSET_CSR0 + slave * 0x04))
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

    uint8_t transfer(Slave slave, uint8_t tx, bool next) {
        // Select the slave
        uint8_t pcs = ~(1 << slave) & 0x0F;
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

    void transfer(Slave slave, uint8_t* txBuffer, uint8_t* rxBuffer, int size, int sizeRx, bool partial) {
        // Select the slave
        uint8_t pcs = ~(1 << slave) & 0x0F;
        uint32_t mr = (*(volatile uint32_t*)(SPI_BASE + OFFSET_MR));
        mr = mr & ~((uint32_t)(0b1111 << MR_PCS)); // Erase the PCS field
        (*(volatile uint32_t*)(SPI_BASE + OFFSET_MR)) = mr | (pcs << MR_PCS); // Reprogram MR

        // If this is a partial transfer, do not deselect the device
        uint32_t csr = (*(volatile uint32_t*)(SPI_BASE + OFFSET_CSR0 + slave * 0x04));
        if (partial) {
            // Enable CSAAT to prevent CS from rising automatically
            (*(volatile uint32_t*)(SPI_BASE + OFFSET_CSR0 + slave * 0x04)) = csr | 1 << CSR_CSAAT;
        } else {
            // Disable CSAAT to make CS rise automatically
            (*(volatile uint32_t*)(SPI_BASE + OFFSET_CSR0 + slave * 0x04)) = csr & ~(uint32_t)(1 << CSR_CSAAT);
        }

        // By default, receive as many bytes as was sent
        if (sizeRx == -1) {
            sizeRx = size;
        }

        // Dummy reads to empty the Rx register
        while ((*(volatile uint32_t*)(SPI_BASE + OFFSET_SR)) >> SR_RDRF & 1) {
            (*(volatile uint32_t*)(SPI_BASE + OFFSET_RDR));
        }

        // Enable Rx DMA channel
        if (rxBuffer && sizeRx > 0) {
            DMA::startChannel(_rxDMAChannel, (uint32_t)rxBuffer, sizeRx);
        } else {
            sizeRx = 0;
        }

        if (sizeRx == size) {
            // Enable Tx DMA channel
            DMA::startChannel(_txDMAChannel, (uint32_t)txBuffer, size);

            // Wait for the end of the transfer
            while (!(DMA::isFinished(_rxDMAChannel) && DMA::isFinished(_txDMAChannel)));
            while (!((*(volatile uint32_t*)(SPI_BASE + OFFSET_SR)) >> SR_TDRE & 1));

        // If the user asked to have more bytes received than sent, send dummy bytes
        } else if (sizeRx > size) {
            // Configure Tx DMA channel without starting it
            DMA::setupChannel(_txDMAChannel, (uint32_t)txBuffer, size);

            // After the channel has finished, reload it with some dummy bytes
            int dummySize = DUMMY_BYTES_SIZE;
            if (sizeRx - size < DUMMY_BYTES_SIZE) {
                dummySize = sizeRx - size;
            }
            DMA::reloadChannel(_txDMAChannel, (uint32_t)DUMMY_BYTES, dummySize);

            // Enable Tx DMA interrupt
            _dummyBytesCounter = sizeRx - size - dummySize;
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
        } else if (sizeRx < size) {
            // Enable Tx DMA channel
            DMA::startChannel(_txDMAChannel, uint32_t(txBuffer), size);

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

}