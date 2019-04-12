#ifndef _SYNC_USB_H_
#define _SYNC_USB_H_

#include <usb.h>
#include "context.h"

namespace SyncUSB {

    const uint16_t USB_VENDOR_ID = USB::DEFAULT_VENDOR_ID;
    const uint16_t USB_PRODUCT_ID = 0xcbd0;
    const char STR_MANUFACTURER[] = "Silica";
    const char STR_PRODUCT[] = "Silver";
    const char STR_SERIALNUMBER[] = "v1";

    const uint8_t REQUEST_START_BOOTLOADER = 0x00;

    void init();
    bool commandAvailable();
    uint8_t getCommand();
    int getPayload(uint8_t* buffer);
    void send(uint8_t command, uint8_t* payload=nullptr, int payloadSize=0);
    bool isConnected();
    void usbConnectedHandler();
    void usbDisconnectedHandler();
    int usbControlHandler(USB::SetupPacket &lastSetupPacket, uint8_t* data, int size);

}

#endif