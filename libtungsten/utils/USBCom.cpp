#include "USBCom.h"
#include <core.h>
#include <RingBuffer.h>
#include <string.h>

namespace USBCom {

    // Bank buffers
    const int BANK_SIZE = 64;
    uint8_t _bankIN[BANK_SIZE];
    uint8_t _bankOUT[BANK_SIZE];

    // Ring buffers
    const int BUFFER_SIZE = 512;
    uint8_t _rxBufferInternal[BUFFER_SIZE];
    RingBuffer _rxBuffer(_rxBufferInternal, BUFFER_SIZE);
    uint8_t _txBufferInternal[BUFFER_SIZE];
    RingBuffer _txBuffer(_txBufferInternal, BUFFER_SIZE);

    // Endpoints
    USB::Endpoint _epIN;
    USB::Endpoint _epOUT;

    // Event handlers
    void (*_eventHandlers[static_cast<int>(Event::N_EVENTS)])();

    volatile bool _usbConnected = false;
    volatile bool _comConnected = false;
    volatile uint8_t _controlRequest = 0;

    // Internal handlers
    int controlHandler(USB::SetupPacket &lastSetupPacket, uint8_t* data, int size);
    int inHandler(int unused);
    int outHandler(int size);
    void connectedHandler();
    void disconnectedHandler();


    void enable(uint16_t vendorId, uint16_t productId, uint16_t deviceRevision) {
        // Initialize the banks
        memset(_bankIN, 0, BANK_SIZE);
        memset(_bankOUT, 0, BANK_SIZE);

        // Initialise the user handlers
        for (int i = 0; i < static_cast<int>(Event::N_EVENTS); i++) {
            _eventHandlers[i] = nullptr;
        }

        // Initialize USB driver
        USB::initDevice(vendorId, productId, deviceRevision);
        USB::setConnectedHandler(connectedHandler);
        USB::setDisconnectedHandler(disconnectedHandler);
        USB::setControlHandler(controlHandler);

        // Add IN and OUT bulk endpoints for communication
        _epIN = USB::newEndpoint(USB::EPType::BULK, USB::EPDir::IN, USB::EPBanks::SINGLE, USB::EPSize::SIZE64, _bankIN);
        USB::setEndpointHandler(_epIN, USB::EPHandlerType::IN, inHandler);
        _epOUT = USB::newEndpoint(USB::EPType::BULK, USB::EPDir::OUT, USB::EPBanks::SINGLE, USB::EPSize::SIZE64, _bankOUT);
        USB::setEndpointHandler(_epOUT, USB::EPHandlerType::OUT, outHandler);
    }

    void setHandler(Event event, void (*handler)()) {
        int e = static_cast<int>(event);
        if (e < static_cast<int>(Event::N_EVENTS)) {
            _eventHandlers[e] = handler;
        }
    }

    int controlHandler(USB::SetupPacket &lastSetupPacket, uint8_t* data, int size) {
        uint8_t request = lastSetupPacket.bRequest;
        if (request == REQ_BOOTLOADER) {
            lastSetupPacket.handled = true;
            Core::resetToBootloader(10);
        } else if (request == REQ_CONNECT) {
            _comConnected = true;
            void (*handler)() = _eventHandlers[static_cast<int>(Event::COM_CONNECTED)];
            if (handler != nullptr) {
                handler();
            }
        } else if (request == REQ_DISCONNECT) {
            _comConnected = false;
            void (*handler)() = _eventHandlers[static_cast<int>(Event::COM_DISCONNECTED)];
            if (handler != nullptr) {
                handler();
            }
        } else if (request & 0x80) {
            _controlRequest = request & 0x7F;
            void (*handler)() = _eventHandlers[static_cast<int>(Event::CONTROL_REQUEST)];
            if (handler != nullptr) {
                handler();
            }
        }
        return 0;
    }

    uint8_t getControlRequest() {
        return _controlRequest;
    }

    int inHandler(int unused) {
        // Copy the buffer to the bank
        int size = _txBuffer.size();
        if (size > BANK_SIZE - 1) {
            size = BANK_SIZE - 1;
        }
        if (size > 0) {
            _txBuffer.read(_bankIN, size);
        }
        if (_txBuffer.isEmpty()) {
            USB::disableINInterrupt(_epIN);
        }

        return size;
    }

    int outHandler(int size) {
        // Copy the bank to the buffer
        _rxBuffer.write(_bankOUT, size);

        return 0;
    }

    void connectedHandler() {
        _usbConnected = true;
        void (*handler)() = _eventHandlers[static_cast<int>(Event::USB_CONNECTED)];
        if (handler != nullptr) {
            handler();
        }
    }

    void disconnectedHandler() {
        if (_comConnected) {
            _comConnected = false;
            void (*handler)() = _eventHandlers[static_cast<int>(Event::COM_DISCONNECTED)];
            if (handler != nullptr) {
                handler();
            }
        }
        _usbConnected = false;
        void (*handler)() = _eventHandlers[static_cast<int>(Event::USB_DISCONNECTED)];
        if (handler != nullptr) {
            handler();
        }
    }

    bool isUSBConnected() {
        return _usbConnected;
    }

    bool isComConnected() {
        return _comConnected;
    }

    int available() {
        USB::disableOUTInterrupt(_epOUT);
        int available = _rxBuffer.size();
        USB::enableOUTInterrupt(_epOUT);
        return available;
    }

    int contains(uint8_t byte) {
        USB::disableOUTInterrupt(_epOUT);
        int pos = _rxBuffer.contains(byte);
        USB::enableOUTInterrupt(_epOUT);
        return pos;
    }

    uint8_t read() {
        USB::disableOUTInterrupt(_epOUT);
        return _rxBuffer.read();
        USB::enableOUTInterrupt(_epOUT);
    }

    int read(uint8_t* buffer, unsigned int size) {
        USB::disableOUTInterrupt(_epOUT);
        return _rxBuffer.read(buffer, size);
        USB::enableOUTInterrupt(_epOUT);
    }

    void write(uint8_t byte) {
        USB::disableINInterrupt(_epIN);
        _txBuffer.write(byte);
        USB::enableINInterrupt(_epIN);
    }

    void write(const uint8_t* buffer, unsigned int size) {
        USB::disableINInterrupt(_epIN);
        _txBuffer.write(buffer, size);
        USB::enableINInterrupt(_epIN);
    }

    void write(int number, uint8_t base) {
        // Write a human-readable number in the given base
        const int bufferSize = 32; // Enough to write a 32-bits word in binary
        char buffer[bufferSize];
        int cursor = 0;
        if (base < 2) {
            return;
        }

        // Special case : number = 0
        if (number == 0) {
            write('0');
            return;
        }

        // Minus sign
        if (number < 0) {
            buffer[cursor] = '-';
            cursor++;
            number = -number;
        }

        // Compute the number in reverse
        int start = cursor;
        for (; cursor < bufferSize && number > 0; cursor++) {
            char c = number % base;
            if (c < 10) {
                c += '0';
            } else {
                c += 'A' - 10;
            }
            buffer[cursor] = c;
            number = number / base;
        }

        // Reverse the result
        for (int i = 0; i < (cursor - start) / 2; i++) {
            char c = buffer[start + i];
            buffer[start + i] = buffer[cursor - i - 1];
            buffer[cursor - i - 1] = c;
        }

        buffer[cursor] = 0;
        cursor++;
        write((const uint8_t*)buffer, cursor);
    }

    void write(bool boolean) {
        // Write a boolean value
        if (boolean) {
            write("true");
        } else {
            write("false");
        }
    }

    void writeLine(const char* buffer, int size) {
        write((const uint8_t*)buffer, size);
        write("\r\n");
    }

    void writeLine(char byte) {
        write(byte);
        write("\r\n");
    }

    void writeLine(int number, uint8_t base) {
        write(number, base);
        write("\r\n");
    }

    void writeLine(bool boolean) {
        write(boolean);
        write("\r\n");
    }

    void write(const char* str) {
        write((const uint8_t*)str, strlen(str));
    }
}