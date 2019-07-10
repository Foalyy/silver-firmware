#ifndef _SYNC_H_
#define _SYNC_H_

#include <stdint.h>

namespace Sync {

    const uint32_t LORA_FREQUENCY = 868250000L; // 868.25MHz

    const int N_CHANNELS = 255;
    const uint8_t SYNC_PREAMBLE = 0x42;

    const int HEADER_SIZE = 3;
    const uint8_t HEADER_PREAMBLE = 0;
    const uint8_t HEADER_CHANNEL = 1;
    const uint8_t HEADER_COMMAND = 2;
    const int MAX_PAYLOAD_SIZE = 10;

    const uint8_t CMD_GET_GUI_STATE = 0x80;
    const uint8_t CMD_GET_GUI_UPDATE = 0x81;
    const uint8_t CMD_FOCUS = 0x90;
    const uint8_t CMD_FOCUS_HOLD = 0x91;
    const uint8_t CMD_FOCUS_RELEASE = 0x92;
    const uint8_t CMD_TRIGGER = 0x93;
    const uint8_t CMD_TRIGGER_NO_DELAY = 0x94;
    const uint8_t CMD_TRIGGER_HOLD = 0x95;
    const uint8_t CMD_TRIGGER_RELEASE = 0x96;


    bool init();
    bool commandAvailable();
    uint8_t getCommand();
    int getPayload(uint8_t* buffer);
    void send(uint8_t command, uint8_t* payload=nullptr, int payloadSize=0);

}

#endif