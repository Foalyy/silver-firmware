#include "sync_usb.h"
#include "sync.h"
#include "context.h"
#include <string.h>

namespace SyncUSB {

    enum class Request {
        FOCUS,
        TRIGGER,
    };

    volatile bool _connected = false;
    bool _commandAvailable = false;
    Request _command = Request::FOCUS;
    uint8_t _payload[Sync::MAX_PAYLOAD_SIZE];
    volatile int _payloadSize = 0;
    uint8_t _sendingBuffer[Sync::MAX_PAYLOAD_SIZE + 1];
    volatile int _sendingBufferSize = 0;


    void init() {
        USB::initDevice(USB_VENDOR_ID, USB_PRODUCT_ID);
        USB::setStringDescriptor(USB::StringDescriptors::MANUFACTURER, STR_MANUFACTURER, sizeof(STR_MANUFACTURER) - 1);
        USB::setStringDescriptor(USB::StringDescriptors::PRODUCT, STR_PRODUCT, sizeof(STR_PRODUCT) - 1);
        USB::setStringDescriptor(USB::StringDescriptors::SERIALNUMBER, STR_SERIALNUMBER, sizeof(STR_SERIALNUMBER) - 1);
        USB::setConnectedHandler(usbConnectedHandler);
        USB::setDisconnectedHandler(usbDisconnectedHandler);
        USB::setControlHandler(usbControlHandler);
    }

    bool commandAvailable() {
        return _commandAvailable;
    }

    uint8_t getCommand() {
        _commandAvailable = false;
        return static_cast<uint8_t>(_command);
    }

    int getPayload(uint8_t* buffer) {
        _commandAvailable = false;
        memcpy(buffer, _payload, _payloadSize);
        return _payloadSize;
    }

    void send(uint8_t command, uint8_t* payload, int payloadSize) {
        for (int i = 0; i < 500; i++) {
            if (_sendingBufferSize == 0) {
                break;
            }
            if (!_connected) {
                return;
            }
            Core::sleep(1);
        }
        if (payloadSize > Sync::MAX_PAYLOAD_SIZE) {
            payloadSize = Sync::MAX_PAYLOAD_SIZE;
        }
        _sendingBuffer[0] = command;
        memcpy(_sendingBuffer + 1, payload, payloadSize);
        _sendingBufferSize = payloadSize + 1;
    }

    bool isConnected() {
        return _connected;
    }

    void resetState() {
        _sendingBufferSize = 0;
        _commandAvailable = false;
    }

    void usbConnectedHandler() {
        resetState();
        _connected = true;
    }

    void usbDisconnectedHandler() {
        resetState();
        _connected = false;
    }

    // Handler called when a CONTROL packet is sent over USB
    int usbControlHandler(USB::SetupPacket &lastSetupPacket, uint8_t* data, int size) {
        // Get command
        _command = static_cast<Request>(lastSetupPacket.bRequest);

        if (lastSetupPacket.direction == USB::EPDir::OUT) {
            // Get payload
            _payloadSize = size;
            if (_payloadSize > Sync::MAX_PAYLOAD_SIZE) {
                _payloadSize = Sync::MAX_PAYLOAD_SIZE;
            }
            memcpy(_payload, data, _payloadSize);

            _commandAvailable = true;
            lastSetupPacket.handled = true;
        } else { // IN
            if (lastSetupPacket.bRequest == Sync::CMD_GET_GUI_STATE) {
                lastSetupPacket.handled = true;
                uint8_t buffer[] = {
                    static_cast<uint8_t>(Context::_submenuFocusHold),
                    static_cast<uint8_t>(Context::_submenuTriggerHold),
                    static_cast<uint8_t>(Context::_triggerSync),
                    static_cast<uint8_t>(Context::_delayMs / 100 >> 16),
                    static_cast<uint8_t>(Context::_delayMs / 100 >> 8),
                    static_cast<uint8_t>(Context::_delayMs / 100),
                    static_cast<uint8_t>(Context::_delaySync),
                    static_cast<uint8_t>(Context::_intervalNShots),
                    static_cast<uint8_t>(Context::_intervalDelayMs / 100 >> 16),
                    static_cast<uint8_t>(Context::_intervalDelayMs / 100 >> 8),
                    static_cast<uint8_t>(Context::_intervalDelayMs / 100),
                    static_cast<uint8_t>(Context::_intervalSync),
                    static_cast<uint8_t>(Context::_inputMode),
                    static_cast<uint8_t>(Context::_inputSync),
                    static_cast<uint8_t>(Context::_syncChannel),
                    static_cast<uint8_t>(Context::_settingsFocusDurationMs / 100 >> 16),
                    static_cast<uint8_t>(Context::_settingsFocusDurationMs / 100 >> 8),
                    static_cast<uint8_t>(Context::_settingsFocusDurationMs / 100),
                    static_cast<uint8_t>(Context::_settingsTriggerDurationMs / 100 >> 16),
                    static_cast<uint8_t>(Context::_settingsTriggerDurationMs / 100 >> 8),
                    static_cast<uint8_t>(Context::_settingsTriggerDurationMs / 100),
                    static_cast<uint8_t>(Context::_settingsSync)
                };
                int payloadSize = sizeof(buffer);
                if (size < payloadSize) {
                    payloadSize = size;
                }
                memcpy(data, buffer, payloadSize);
                return payloadSize;

            } else if (lastSetupPacket.bRequest == Sync::CMD_GET_GUI_UPDATE) {
                lastSetupPacket.handled = true;
                int size = _sendingBufferSize;
                memcpy(data, _sendingBuffer, size);
                _sendingBufferSize = 0;
                return size;
            }
        }

        return 0;
    }

}