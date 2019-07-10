#include <core.h>
#include <gpio.h>
#include <spi.h>
#include "lora.h"

namespace LoRa {

    GPIO::Pin pinReset = GPIO::PA00;
    SPI::Peripheral _spi = 0;
    bool _spiEnabled = false;
    uint32_t _frequency = 0;
    Mode _mode = Mode::STANDBY;
    int _txPowerdBm = 0;
    int _spreadingFactor = 7;
    CodingRate _codingRate = CodingRate::RATE_4_5;
    Bandwidth _bandwidth = Bandwidth::BW_125kHz;
    bool _explicitHeaderEnabled = true;
    bool _isRxEnabled = false;

    // Internal functions
    void writeConfig();


    // Set the GPIOs used by the module
    void setPin(PinFunction pinFunction, GPIO::Pin pin) {
        if (pinFunction == PinFunction::RESET) {
            pinReset = pin;
        }
    }

    void disable() {
        // Turn of the module by forcing it into reset
        // init() must be called to turn it back on
        GPIO::enableOutput(pinReset, GPIO::LOW);
    }

    // Initialize the module with default settings and check it answers correctly
    bool init(SPI::Peripheral slave, uint32_t frequency) {
        _spi = slave;

        // Reset
        GPIO::enableOutput(pinReset, GPIO::LOW);
        Core::sleep(1);
        GPIO::set(pinReset, GPIO::HIGH);
        Core::sleep(5);

        // Enable the SPI controller for this slave
        if (!_spiEnabled) {
            SPI::addPeripheral(_spi);
        }
        _spiEnabled = true;

        // Check that the modules answers on the bus
        uint8_t version = readRegister(REG_VERSION);
        if (version == 0x00 || version == 0xFF) {
            return false;
        }

        // Enable LoRa mode
        setMode(Mode::SLEEP);
        writeRegister(REG_OP_MODE, 0x80);
        setMode(Mode::STANDBY);
        if (readRegister(REG_OP_MODE) != 0x81) {
            return false;
        }

        // Configure the module
        setFrequency(frequency);
        setTxPower(13); // +13dBm ~= 20mW by default
        writeConfig();
        Core::sleep(10);

        return true;
    }

    // Set the current operating mode (sleep, standby, rx, tx...)
    void setMode(Mode mode) {
        _mode = mode;

        uint8_t regOpMode = readRegister(REG_OP_MODE);
        regOpMode &= 0xF8;
        regOpMode |= static_cast<int>(mode);
        writeRegister(REG_OP_MODE, regOpMode);
    }

    // Set the module frequency in Hz, between 862000000L (862MHz) and 915000000L (915MHz)
    void setFrequency(uint32_t frequency) {
        if (frequency < 862000000L || frequency > 915000000L) {
            return;
        }
        _frequency = frequency;
        uint64_t frequency_mHz = (uint64_t)frequency * 1000;
        uint32_t regFr = frequency_mHz / 61035; // Resolution with 32MHz XOSC according to the datasheet
        writeRegister(REG_FR_MSB, (regFr >> 16) & 0xFF);
        writeRegister(REG_FR_MID, (regFr >> 8) & 0xFF);
        writeRegister(REG_FR_LSB, regFr & 0xFF);
    }

    // Set the transmitter power between +2 to +17dBm, or to +20dBm
    // When using the high-power +20dBm setting, do not transmit at a duty cycle
    // higher than 1% to allow the PA to cool down.
    void setTxPower(int dBm) {
        if (dBm > 20 || dBm < 2) {
            return;
        }
        _txPowerdBm = dBm;

        // Special high-power setting at +20dBm
        // Maximum duty-cycle at +20dBm : 1%
        // See datasheet §5.4.3. High Power +20 dBm Operation
        if (dBm > 17) {
            writeRegister(REG_PA_DAC, REG_PA_DAC_HIGH);
            dBm = 17;
        } else {
            writeRegister(REG_PA_DAC, REG_PA_DAC_DEFAULT);
        }

        // Write the power setting
        writeRegister(REG_PA_CONFIG, 
            1 << REG_PA_CONFIG_PA_BOOST | // Always use PA_BOOST, as RFO doesn't appear to be connected
            (dBm - 2) << REG_PA_CONFIG_OUTPUT_POWER
        );
    }

    // Set the Spreading Factor between 6 and 12. A higher SF means a slower but more
    // reliable transfer.
    void setSpreadingFactor(int spreadingFactor) {
        if (spreadingFactor < 6 || spreadingFactor > 12) {
            return;
        }
        _spreadingFactor = spreadingFactor;
    }

    // Set the length of the error-correction code
    void setCodingRate(CodingRate codingRate) {
        if (static_cast<int>(codingRate) < 1 || static_cast<int>(codingRate) > 4) {
            return;
        }
        _codingRate = codingRate;
        writeConfig();
    }

    // Set the radio bandwidth. A higher bandwidth means quicker transfers but
    // be careful to respect your local legislation relative to bandwidth usage.
    void setBandwidth(Bandwidth bandwidth) {
        if (static_cast<int>(bandwidth) < 0 || static_cast<int>(bandwidth) > 9) {
            return;
        }
        _bandwidth = bandwidth;
        writeConfig();
    }

    void setExplicitHeader(bool explicitHeaderEnabled) {
        _explicitHeaderEnabled = explicitHeaderEnabled;
        writeConfig();
    }

    void writeConfig() {
        writeRegister(REG_MODEM_CONFIG_1, 
            (_explicitHeaderEnabled ? 0 : 1) << REG_MODEM_CONFIG_1_IMPLICIT_HEADER_MODE |
            static_cast<int>(_codingRate) << REG_MODEM_CONFIG_1_CODING_RATE |
            static_cast<int>(_bandwidth) << REG_MODEM_CONFIG_1_BW
        );
        writeRegister(REG_MODEM_CONFIG_2, 
            1 << REG_MODEM_CONFIG_2_RX_PAYLOAD_CRC_ON |
            0 << REG_MODEM_CONFIG_2_TX_CONTINUOUS_MODE |
            _spreadingFactor << REG_MODEM_CONFIG_2_SPREADING_FACTOR
        );
        writeRegister(REG_MODEM_CONFIG_3, 
            1 << REG_MODEM_CONFIG_3_AGC_AUTO_ON
        );
    }

    // Send data
    void tx(uint8_t* payload, unsigned int length) {
        // Check length
        if (length > MAX_TX_LENGTH) {
            length = MAX_TX_LENGTH;
        }
        
        // Clear all flags
        writeRegister(REG_IRQ_FLAGS, 0xFF);

        // Point FIFO to the base TX address
        writeRegister(REG_FIFO_ADDR_PTR, readRegister(REG_FIFO_TX_BASE_ADDR));

        // Set payload length
        writeRegister(REG_PAYLOAD_LENGTH, length);

        // Write payload to the FIFO
        uint8_t buffer[] = {REG_FIFO | 0x80};
        SPI::transfer(_spi, buffer, 1, nullptr, -1, true);
        SPI::transfer(_spi, payload, length);

        // Switch to TX mode to send the payload
        setMode(Mode::TX);

        // Wait for TxDone IRQ
        while (!(readRegister(REG_IRQ_FLAGS) & (1 << IRQ_TX_DONE))) {
            Core::sleep(1);
        }

        // Clear all flags
        writeRegister(REG_IRQ_FLAGS, 0xFF);

        // Re-enable Rx
        if (_isRxEnabled) {
            enableRx();
        }
    }

    void enableRx() {
        _isRxEnabled = true;
        
        // Reset FIFO RX address to the start of the FIFO
        writeRegister(REG_FIFO_RX_BASE_ADDR, 0x00);

        // Start RX mode
        setMode(Mode::RX_CONTINUOUS);
    }

    void disableRx() {
        _isRxEnabled = false;

        // Go back to standby mode
        setMode(Mode::STANDBY);
    }

    bool rxAvailable() {
        return readRegister(REG_IRQ_FLAGS) & (1 << IRQ_RX_DONE);
    }

    // Receive data into a user buffer
    int rx(uint8_t* buffer, unsigned int length) {
        uint8_t irqFlags = readRegister(REG_IRQ_FLAGS);
        if (readRegister(REG_IRQ_FLAGS) & (1 << IRQ_RX_DONE)) {
            // Clear all flags
            writeRegister(REG_IRQ_FLAGS, 0xFF);

            // Check that header is valid
            if (!(irqFlags & (1 << IRQ_VALID_HEADER))) {
                return INVALID_HEADER;
            }

            // Check that payload CRC is valid
            if (irqFlags & (1 << IRQ_PAYLOAD_CRC_ERROR)) {
                return PAYLOAD_CRC_ERROR;
            }

            // Get the length of data received
            unsigned int rxBytesNb = readRegister(REG_FIFO_RX_BYTES_NB);
            if (rxBytesNb > length) {
                rxBytesNb = length;
            }

            // Point FIFO to the address of the last received packet
            writeRegister(REG_FIFO_ADDR_PTR, readRegister(REG_FIFO_RX_CURRENT_ADDR));

            // Read the data from the FIFO
            uint8_t txBuffer[1] = {REG_FIFO};
            SPI::transfer(_spi, txBuffer, 1, nullptr, -1, true);
            SPI::transfer(_spi, nullptr, 0, buffer, rxBytesNb);

            // Clear all flags
            writeRegister(REG_IRQ_FLAGS, 0xFF);
            
            // Reset the RX state machine for the next reception
            // This is the only way to reset REG_FIFO_RX_CURRENT_ADDR
            // and to make sure the buffer will not eventually overflow
            // after multiple transfers
            disableRx();
            enableRx();

            return rxBytesNb;
        }
        return 0;
    }

    // Received Signal Strength Indicator of the last packet, in dBm
    int lastPacketRSSI() {
        return readRegister(REG_PKT_RSSI_VALUE) - 137;
    }

    // Instantaneous Received Signal Strength Indicator, in dBm
    int currentRSSI() {
        return readRegister(REG_RSSI_VALUE) - 137;
    }

    // Read an arbitrary register
    uint8_t readRegister(uint8_t reg) {
        return SPI::transfer(_spi, reg, true);
    }

    // Write an arbitrary
    void writeRegister(uint8_t reg, uint8_t value) {
        uint8_t txBuffer[] = {
            (uint8_t)(reg | 0x80),
            value
        };
        SPI::transfer(_spi, txBuffer, 2);
    }

}