#ifndef _USB_COM_H_
#define _USB_COM_H_

#include <libusb-1.0/libusb.h>
#include <string>

class USBCom {
private:

    const uint16_t DEFAULT_USB_VENDOR_ID = 0x03eb;
    const uint16_t DEFAULT_USB_PRODUCT_ID = 0xcabd;
    const uint16_t DEFAULT_INTERFACE = 0;

    enum class Direction {
        OUTPUT = 0,
        INPUT = 1,
    };

    libusb_device_handle* _handle = nullptr;

    int sendRequestInternal(uint8_t request, uint16_t value=0, uint16_t index=0, Direction direction=Direction::OUTPUT, uint8_t* buffer=nullptr, uint16_t length=0, int timeout=0);
    int findDevice(uint16_t vid, uint16_t pid);
    void printLibUSBError(std::string message, int r);

public:
    int init();
    void close();
    int read(uint8_t* buffer, int maxLength, int timeout=0);
    int write(const uint8_t* buffer, int length, int timeout=0);
    int sendRequest(uint8_t request, uint16_t value=0, uint16_t index=0, Direction direction=Direction::OUTPUT, uint8_t* buffer=nullptr, uint16_t length=0, int timeout=0);

};

#endif