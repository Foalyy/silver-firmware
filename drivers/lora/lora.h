#ifndef _LORA_H_
#define _LORA_H_

#include <spi.h>
#include <stdint.h>


namespace LoRa {

    const uint32_t DEFAULT_FREQUENCY = 868000000L; // 868MHz
    const int MAX_TX_LENGTH = 128;

    // Registers
    const uint8_t REG_FIFO = 0x00;
    const uint8_t REG_OP_MODE = 0x01;
    const uint8_t REG_FR_MSB = 0x06;
    const uint8_t REG_FR_MID = 0x07;
    const uint8_t REG_FR_LSB = 0x08;
    const uint8_t REG_PA_CONFIG = 0x09;
    const uint8_t REG_FIFO_ADDR_PTR = 0x0D;
    const uint8_t REG_FIFO_TX_BASE_ADDR = 0x0E;
    const uint8_t REG_FIFO_RX_BASE_ADDR = 0x0F;
    const uint8_t REG_FIFO_RX_CURRENT_ADDR = 0x10;
    const uint8_t REG_IRQ_FLAGS_MASK = 0x11;
    const uint8_t REG_IRQ_FLAGS = 0x12;
    const uint8_t REG_FIFO_RX_BYTES_NB = 0x13;
    const uint8_t REG_PKT_RSSI_VALUE = 0x1A;
    const uint8_t REG_RSSI_VALUE = 0x1B;
    const uint8_t REG_MODEM_CONFIG_1 = 0x1D;
    const uint8_t REG_MODEM_CONFIG_2 = 0x1E;
    const uint8_t REG_PAYLOAD_LENGTH = 0x22;
    const uint8_t REG_MODEM_CONFIG_3 = 0x26;
    const uint8_t REG_FIFO_RX_BYTE_ADDR = 0x25;
    const uint8_t REG_VERSION = 0x42;
    const uint8_t REG_PA_DAC = 0x4D;

    // Subregisters
    const uint8_t REG_MODEM_CONFIG_1_IMPLICIT_HEADER_MODE = 0;
    const uint8_t REG_MODEM_CONFIG_1_CODING_RATE = 1;
    const uint8_t REG_MODEM_CONFIG_1_BW = 4;
    const uint8_t REG_MODEM_CONFIG_2_RX_PAYLOAD_CRC_ON = 2;
    const uint8_t REG_MODEM_CONFIG_2_TX_CONTINUOUS_MODE = 3;
    const uint8_t REG_MODEM_CONFIG_2_SPREADING_FACTOR = 4;
    const uint8_t REG_MODEM_CONFIG_3_AGC_AUTO_ON = 2;
    const uint8_t REG_MODEM_CONFIG_3_MOBILE_NODE = 3;
    const uint8_t REG_PA_CONFIG_OUTPUT_POWER = 0;
    const uint8_t REG_PA_CONFIG_MAX_POWER = 4;
    const uint8_t REG_PA_CONFIG_PA_BOOST = 7;
    const uint8_t REG_PA_DAC_DEFAULT = 0x84;
    const uint8_t REG_PA_DAC_HIGH = 0x87;
    const uint8_t IRQ_CAD_DETECTED = 0;
    const uint8_t IRQ_FHSS_CHANGE_CHANNEL = 1;
    const uint8_t IRQ_CAD_DONE = 2;
    const uint8_t IRQ_TX_DONE = 3;
    const uint8_t IRQ_VALID_HEADER = 4;
    const uint8_t IRQ_PAYLOAD_CRC_ERROR = 5;
    const uint8_t IRQ_RX_DONE = 6;
    const uint8_t IRQ_RX_TIMEOUT = 7;

    // Error codes
    const int INVALID_HEADER = -1;
    const int PAYLOAD_CRC_ERROR = -2;

    // Operating modes
    enum class Mode {
        SLEEP = 0b000,
        STANDBY = 0b001,
        FSTX = 0b010, // Frequency synthesis TX
        TX = 0b011,
        FSRX = 0b100, // Frequency synthesis RX
        RX_CONTINUOUS = 0b101,
        RX_SINGLE = 0b110,
        CAD = 0b111, // Channel activity detection
    };

    // Coding rate
    enum class CodingRate {
        RATE_4_5 = 1,
        RATE_4_6 = 2,
        RATE_4_7 = 3,
        RATE_4_8 = 4,
    };

    // Bandwidth
    enum class Bandwidth {
        BW_7_8kHz,
        BW_10_4kHz,
        BW_15_6kHz,
        BW_20_8kHz,
        BW_31_3kHz,
        BW_41_7kHz,
        BW_62_5kHz,
        BW_125kHz,
        BW_250kHz,
        BW_500kHz
    };

    enum class PinFunction {
        RESET
    };


    void setPin(PinFunction pinFunction, GPIO::Pin pin);
    void disable();
    bool init(SPI::Peripheral slave, uint32_t frequency=DEFAULT_FREQUENCY);
    void setMode(Mode mode);
    void setFrequency(uint32_t frequency);
    void setTxPower(int dBm);
    void setSpreadingFactor(int spreadingFactor);
    void setCodingRate(CodingRate codingRate);
    void setBandwidth(Bandwidth bandwidth);
    void setExplicitHeader(bool explicitHeaderEnabled);
    void tx(uint8_t* payload, unsigned int length);
    void enableRx();
    void disableRx();
    bool rxAvailable();
    int rx(uint8_t* buffer, unsigned int length);
    int lastPacketRSSI();
    int currentRSSI();
    uint8_t readRegister(uint8_t reg);
    void writeRegister(uint8_t reg, uint8_t value);

}

#endif