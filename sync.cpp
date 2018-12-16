#include "sync.h"
#include <string.h>
#include "gui.h"
#include "context.h"
#include "drivers/lora/lora.h"

#include <core.h>
#include <gpio.h>

const int BUFFER_RX_SIZE = Sync::HEADER_SIZE + Sync::MAX_PAYLOAD_SIZE;
uint8_t _rxBuffer[BUFFER_RX_SIZE];
int _rxSize = 0;
bool _commandAvailable = false;

bool Sync::init() {
    if (!LoRa::init(LORA_FREQUENCY, 1)) {
        return false;
    }
    LoRa::setTxPower(14); // dBm
    LoRa::setSpreadingFactor(8);
    LoRa::setCodingRate(LoRa::CodingRate::RATE_4_8);
    LoRa::setBandwidth(LoRa::Bandwidth::BW_500kHz);
    LoRa::setExplicitHeader(true);
    LoRa::enableRx();
    return true;
}

void Sync::disable() {
    LoRa::disable();
}

void Sync::enable() {
    LoRa::enable();
}

bool Sync::commandAvailable() {
    // Check if some data has been received in the LoRa's FIFO
    if (LoRa::rxAvailable()) {
        // Retreive this data
        uint8_t rxBuffer2[BUFFER_RX_SIZE];
        int rxSize2 = LoRa::rx(rxBuffer2, BUFFER_RX_SIZE);
        // Check that this looks like a valid frame on the same channel
        if (rxSize2 >= HEADER_SIZE && rxBuffer2[HEADER_PREAMBLE] == SYNC_PREAMBLE && rxBuffer2[HEADER_CHANNEL] == Context::_syncChannel) {
            // Copy the frame into the main buffer
            memcpy(_rxBuffer, rxBuffer2, rxSize2);
            _rxSize = rxSize2;
            _commandAvailable = true;
        }
    }
    return _commandAvailable;
}

uint8_t Sync::getCommand() {
    _commandAvailable = false;
    if (_rxSize >= HEADER_SIZE) {
        return _rxBuffer[HEADER_COMMAND];
    }
    return 0xFF;
}

int Sync::getPayload(uint8_t* buffer) {
    _commandAvailable = false;
    int payloadSize = _rxSize - HEADER_SIZE;
    memcpy(buffer, _rxBuffer + HEADER_SIZE, payloadSize);
    return payloadSize;
}

void Sync::send(uint8_t command, uint8_t* payload, int payloadSize) {
    if (payloadSize > MAX_PAYLOAD_SIZE) {
        payloadSize = MAX_PAYLOAD_SIZE;
    }
    uint8_t buffer[HEADER_SIZE + MAX_PAYLOAD_SIZE];
    buffer[HEADER_PREAMBLE] = SYNC_PREAMBLE;
    buffer[HEADER_CHANNEL] = Context::_syncChannel;
    buffer[HEADER_COMMAND] = command;
    if (payloadSize > 0 && payload != nullptr) {
        memcpy(buffer + HEADER_SIZE, payload, payloadSize);
    }
    LoRa::tx(buffer, HEADER_SIZE + payloadSize);
}
