#ifndef _USB_COM_H
#define _USB_COM_H

#include <usb.h>

namespace USBCom {

    enum class Event {
        USB_CONNECTED,
        USB_DISCONNECTED,
        COM_CONNECTED,
        COM_DISCONNECTED,
        CONTROL_REQUEST,

        N_EVENTS
    };

    const uint8_t REQ_BOOTLOADER = 0x00;
    const uint8_t REQ_CONNECT = 0x01;
    const uint8_t REQ_DISCONNECT = 0x02;

    void enable(uint16_t vendorId=USB::DEFAULT_VENDOR_ID, uint16_t productId=USB::DEFAULT_PRODUCT_ID, uint16_t deviceRevision=USB::DEFAULT_DEVICE_REVISION);
    void setHandler(Event event, void (*handler)());
    bool isUSBConnected();
    bool isComConnected();
    uint8_t getControlRequest();
    int available();
    int contains(uint8_t byte);
    uint8_t read();
    int read(uint8_t* buffer, unsigned int size);
    void write(uint8_t byte);
    void write(const uint8_t* buffer, unsigned int size);
    void write(const char* str);
    void write(int number, uint8_t base=10);
    void write(bool boolean);
    void writeLine(const char* buffer, int size);
    void writeLine(char byte);
    void writeLine(int number, uint8_t base=10);
    void writeLine(bool boolean);

}

#endif