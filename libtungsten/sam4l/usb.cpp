#include "usb.h"
#include "core.h"
#include "pm.h"
#include "scif.h"
#include "utils.h"
#include "gpio.h"
#include <string.h>

namespace USB {

    // Current state of the driver during enumeration
    volatile State _state = State::SUSPEND;

    // Endpoints
    // The three least significant bits of UDESC.UDESCA are always zero according to the datasheet,
    // so _epRAMDescriptors must be aligned to an 8-bytes boundary
    struct EndpointConfig _endpoints[N_EP_MAX];
    uint32_t _epRAMDescriptors[N_EP_MAX * EP_DESCRIPTOR_SIZE] __attribute__ ((aligned (0b1000)));
    volatile int _nEndpoints = 0;

    // Bank for EP0
    // This must be large enough to store the whole configuration descriptor :
    // 9 (config desc size) + 9 (iface desc size) + 8 (N_EP_MAX) * 7 (ep desc size) = 74 bytes max
    // and must be a multiple of the endpoint size (64)
    const int BANK_EP0_SIZE = 512;
    uint8_t _bankEP0[BANK_EP0_SIZE];

    // Struct where the parameters of the last Setup request are stored
    SetupPacket _lastSetupPacket;
    volatile bool _setupPacketAvailable = false;


    volatile bool _doEnableAddress = false;

    // Default device descriptor content
    DeviceDescriptor _deviceDescriptor = {
        .bLength = 18,
        .bDescriptorType = 0x01,
        .bcdUSB = 0x0200, // USB version 2.0.0
        .bDeviceClass = 0x00,
        .bDeviceSubClass = 0x00,
        .bDeviceProtocol = 0x00,
        .bMaxPacketSize0 = 64,
        .idVendor = DEFAULT_VENDOR_ID,
        .idProduct = DEFAULT_PRODUCT_ID,
        .bcdDevice = DEFAULT_DEVICE_REVISION,
        .iManufacturer = static_cast<uint8_t>(StringDescriptors::MANUFACTURER) + 1,
        .iProduct = static_cast<uint8_t>(StringDescriptors::PRODUCT) + 1,
        .iSerialNumber = static_cast<uint8_t>(StringDescriptors::SERIALNUMBER) + 1,
        .bNumConfigurations = 1
    };

    // Default configuration descriptor content
    // Full configuration descriptor content, with subordinate interface and endpoint descriptors
    ConfigurationDescriptor _configurationDescriptor = {
        .bLength = 9,
        .bDescriptorType = 0x02,
        .wTotalLength = 18,
        .bNumInterfaces = 1,
        .bConfigurationValue = 1,
        .iConfiguration = 0,
        .bmAttributes = 0xA0, // TODO : bus power settings
        .bMaxPower = 150 // In units of 2mA, therefore 300mA
    };
    InterfaceDescriptor _interfaceDescriptor = {
        .bLength = 9,
        .bDescriptorType = 0x04,
        .bInterfaceNumber = 0,
        .bAlternateSetting = 0,
        .bNumEndpoints = 0,
        .bInterfaceClass = 0xFF, // TODO cf p109
        .bInterfaceSubclass = 0xFF,
        .bInterfaceProtocol = 0xFF,
        .iInterface = 0,
    };

    // String0 descriptor, list of supported languages
    // See http://www.usb.org/developers/docs/USB_LANGIDs.pdf
    String0Descriptor _string0Descriptor = {
        .bLength = 4,
        .bDescriptorType = 0x03,
        .wLANGID0 = 0x0409, // English (United States)
    };

    // List of available String descriptors. This must match the enum StringDescriptors.
    // bLength should be 2 bytes longer to account for bLength and bDescriptorType,
    // but also 2 bytes shorter to remove the null-terminating character at the end
    // of the string : sizeof() + 2 - 2 = sizeof()
    StringDescriptor _stringDescriptors[] = {
        { // Device.iManufacturer
            .bLength = sizeof(DEFAULT_IMANUFACTURER),
            .bDescriptorType = 0x03,
            .bString = DEFAULT_IMANUFACTURER
        },
        { // Device.iProduct
            .bLength = sizeof(DEFAULT_IPRODUCT),
            .bDescriptorType = 0x03,
            .bString = DEFAULT_IPRODUCT
        },
        { // Device.iSerialNumber
            .bLength = sizeof(DEFAULT_SERIALNUMBER),
            .bDescriptorType = 0x03,
            .bString = DEFAULT_SERIALNUMBER
        }
    };
    char16_t _customStringDescriptors[static_cast<int>(StringDescriptors::NUMBER)][MAX_STRING_DESCRIPTOR_SIZE];

    extern uint8_t INTERRUPT_PRIORITY;

    // User handlers
    void (*_connectedHandler)() = nullptr;
    void (*_disconnectedHandler)() = nullptr;
    void (*_startOfFrameHandler)() = nullptr;
    int (*_controlHandler)(SetupPacket &_lastSetupPacket, uint8_t* data, int size) = nullptr;



    // Initialize the USB controller in Device mode
    void initDevice(uint16_t vendorId, uint16_t productId, uint16_t deviceRevision) {
        // Init state
        _state = State::SUSPEND;

        // Enable the clocks required by the USB
        PM::enablePeripheralClock(PM::CLK_USB_HSB);
        PM::enablePeripheralClock(PM::CLK_USB);
        SCIF::enableRCFAST(SCIF::RCFASTFrequency::RCFAST_12MHZ);
        SCIF::enablePLL(1, 0, SCIF::GCLKSource::RCFAST, 12000000UL);
        SCIF::enableGenericClock(SCIF::GCLKChannel::GCLK7_USB, SCIF::GCLKSource::PLL);

        // Check the clock
        if (!((*(volatile uint32_t*)(USB_BASE + OFFSET_USBSTA)) & (1 << USBSTA_CLKUSABLE))) {
            Error::happened(Error::Module::USB, ERR_CLOCK_NOT_USABLE, Error::Severity::CRITICAL);
        }

        // Enable signal pins
        GPIO::enablePeripheral(PIN_DM);
        GPIO::enablePeripheral(PIN_DP);

        // USBCON (General Control Register) : set USB controller in Device mode
        (*(volatile uint32_t*)(USB_BASE + OFFSET_USBCON))
            = 0 << USBCON_FRZCLK   // FRZCLK : unfreeze input clocks
            | 1 << USBCON_USBE     // USBE : enable controller
            | 1 << USBCON_UIMOD;   // UIMOD : set in device mode

        // Initialize memory buffers
        memset(_endpoints, 0, sizeof(_endpoints));
        memset(_epRAMDescriptors, 0, sizeof(_epRAMDescriptors));

        // UDESC (USB Descriptor Address) : set descriptor vector address
        (*(volatile uint32_t*)(USB_BASE + OFFSET_UDESC)) = (uint32_t)_epRAMDescriptors;

        // UDCON (Device General Control Register) : attach pullup to start device enumeration
        (*(volatile uint32_t*)(USB_BASE + OFFSET_UDCON))
            = 0 << UDCON_LS        // LS : full-speed mode
            | 0 << UDCON_DETACH;   // DETACH : attach device

        // Configure the device descriptor
        _deviceDescriptor.idVendor = vendorId;
        _deviceDescriptor.idProduct = productId;
        _deviceDescriptor.bcdDevice = deviceRevision;

        // Init the first endpoint (mandatory Control Endpoint)
        EndpointConfig* ep = &_endpoints[0];
        ep->enabled = true;
        ep->type = EPType::CONTROL;
        ep->direction = EPDir::OUT;
        ep->nBanks = EPBanks::SINGLE;
        ep->size = EPSize::SIZE64;
        ep->bank0 = _bankEP0;
        setEndpointHandler(0, EPHandlerType::SETUP, ep0SETUPHandler);
        setEndpointHandler(0, EPHandlerType::IN, ep0INHandler);
        setEndpointHandler(0, EPHandlerType::OUT, ep0OUTHandler);
        _nEndpoints = 1;

        // Enable interrupts
        (*(volatile uint32_t*)(USB_BASE + OFFSET_UDINTESET))
            = 1 << UDINT_SUSP
            | 1 << UDINT_EORST
            | 1 << UDINT_WAKEUP;
        Core::setInterruptHandler(Core::Interrupt::USBC, interruptHandler);
        Core::enableInterrupt(Core::Interrupt::USBC, INTERRUPT_PRIORITY);
    }

    void setStringDescriptor(StringDescriptors descriptor, const char* string, int size) {
        if (size > MAX_STRING_DESCRIPTOR_SIZE) {
            size = MAX_STRING_DESCRIPTOR_SIZE;
        }
        char16_t* customString = _customStringDescriptors[static_cast<int>(descriptor)];
        for (int i = 0; i < size; i++) {
            if (string[i] == 0) {
                size = i;
                break;
            }
            customString[i] = string[i];
        }
        _stringDescriptors[static_cast<int>(descriptor)].bLength = 2 * size + 2;
        _stringDescriptors[static_cast<int>(descriptor)].bString = customString;
    }


    // Initialize an endpoint
    Endpoint newEndpoint(EPType type, EPDir direction, EPBanks nBanks, EPSize size, uint8_t* bank0, uint8_t* bank1) {
        // Check endpoint number
        const int n = _nEndpoints;
        if (n >= N_EP_MAX) {
            return EP_ERROR;
        }
        _nEndpoints++;

        // Internal configuration
        EndpointConfig* ep = &_endpoints[n];
        ep->enabled = true;
        ep->type = type;
        ep->direction = direction;
        ep->nBanks = nBanks;
        ep->size = size;
        ep->bank0 = bank0;
        ep->bank1 = bank1;

        if (_state >= State::DEFAULT) {
            // Reset and enable the endpoint
            (*(volatile uint32_t*)(USB_BASE + OFFSET_UERST))
                &= ~(1 << n);
            (*(volatile uint32_t*)(USB_BASE + OFFSET_UERST))
                |= 1 << n;

            // RAM descriptor configuration
            (*(volatile uint32_t*)(USB_BASE + OFFSET_UECFG0 + n * 4))
                = static_cast<int>(nBanks) << UECFG_EPBK    // EPBK : single/multi-bank endpoint
                | static_cast<int>(size) << UECFG_EPSIZE   // EPSIZE : N-bytes long banks
                | static_cast<int>(direction) << UECFG_EPDIR     // EPDIR : direction IN/OUT
                | static_cast<int>(type) << UECFG_EPTYPE;  // EPTYPE : endpoint type (Control, ...)

            // Descriptor configuration
            ep->descriptor.bLength = 7;
            ep->descriptor.bDescriptorType = 0x05;
            ep->descriptor.bEndpointAddress = n | ((direction == EPDir::IN ? 1 : 0) << 7);
            ep->descriptor.bmAttributes = static_cast<int>(type);
            ep->descriptor.wMaxPacketSize = EP_SIZES[static_cast<int>(size)];
            ep->descriptor.bInterval = 10;

            // Set up descriptor for bank0
            //memset(bank0, 0, EP_SIZES[static_cast<int>(size)]);
            const int offset0 = n * EP_DESCRIPTOR_SIZE;
            _epRAMDescriptors[offset0 + EP_ADDR] = (uint32_t)bank0;
            _epRAMDescriptors[offset0 + EP_PCKSIZE] = 0;
            _epRAMDescriptors[offset0 + EP_CTR_STA] = 0;

            // Set up descriptor for bank1
            if (nBanks == EPBanks::DOUBLE && bank1 != nullptr) {
                //memset(bank1, 0, EP_SIZES[static_cast<int>(size)]);
                const int offset1 = n * EP_DESCRIPTOR_SIZE + EP_DESCRIPTOR_BANK_SIZE;
                _epRAMDescriptors[offset1 + EP_ADDR] = (uint32_t)bank1;
                _epRAMDescriptors[offset1 + EP_PCKSIZE] = 0;
                _epRAMDescriptors[offset1 + EP_CTR_STA] = 0;
            }

            // Enable interrupts
            (*(volatile uint32_t*)(USB_BASE + OFFSET_UDINTESET))
                = 1 << (UDINT_EP0INT + n);
            (*(volatile uint32_t*)(USB_BASE + OFFSET_UECON0SET + n * 4))
                = (type == EPType::CONTROL ? 1 << UECON_RXSTPE : 0)     // SETUP packet
                | 1 << UECON_RXOUTE;   // OUT packet (always enabled )

            // Update the configuration and interface descriptors with the new endpoint
            // -1 because endpoint 0 is not counted
            _configurationDescriptor.wTotalLength
                = _configurationDescriptor.bLength
                + _interfaceDescriptor.bLength
                + (_nEndpoints > 1 ? (_nEndpoints - 1) * ep->descriptor.bLength : 0);
            _interfaceDescriptor.bNumEndpoints = _nEndpoints - 1;
        }
        return n;
    }

    // Main interrupt handler called when an USB-related event happens
    void interruptHandler() {
        uint32_t udint = (*(volatile uint32_t*)(USB_BASE + OFFSET_UDINT));
        uint32_t udinte = (*(volatile uint32_t*)(USB_BASE + OFFSET_UDINTE));

        // Suspend
        if (udinte & udint & (1 << UDINT_SUSP)) {
            // Update the driver state
            _state = State::SUSPEND;

            // Disable this interrupt
            (*(volatile uint32_t*)(USB_BASE + OFFSET_UDINTECLR))
                = 1 << UDINT_SUSP;

            // Clear all interrupts
            (*(volatile uint32_t*)(USB_BASE + OFFSET_UDINTCLR))
                = 0xFFFFFFFF;

            // Enable the WAKEUP interrupt
            (*(volatile uint32_t*)(USB_BASE + OFFSET_UDINTESET))
                = 1 << UDINT_WAKEUP;

            // Freeze clock
            (*(volatile uint32_t*)(USB_BASE + OFFSET_USBCON))
                |= 1 << USBCON_FRZCLK;   // FRZCLK : freeze input clocks

            // Call user handler
            if (_disconnectedHandler != nullptr) {
                _disconnectedHandler();
            }

            // Since clocks are frozen, don't do anything more
            return;
        }

        // Wake up
        if (udinte & udint & (1 << UDINT_WAKEUP)) {
            // Update the driver state
            _state = State::POWERED;

            // Unfreeze clock
            (*(volatile uint32_t*)(USB_BASE + OFFSET_USBCON))
                &= ~(uint32_t)(1 << USBCON_FRZCLK);   // FRZCLK : unfreeze input clocks

            // Disable this interrupt
            (*(volatile uint32_t*)(USB_BASE + OFFSET_UDINTECLR))
                = 1 << UDINT_WAKEUP;

            // Clear interrupt
            (*(volatile uint32_t*)(USB_BASE + OFFSET_UDINTCLR))
                = 1 << UDINT_WAKEUP;

            // Enable the SUSP interrupt
            (*(volatile uint32_t*)(USB_BASE + OFFSET_UDINTCLR))
                = 1 << UDINT_SUSP;
            (*(volatile uint32_t*)(USB_BASE + OFFSET_UDINTESET))
                = 1 << UDINT_SUSP;

            // Call user handler
            if (_connectedHandler != nullptr) {
                _connectedHandler();
            }
        }

        // End of reset
        if (udinte & udint & (1 << UDINT_EORST)) {
            // Clear interrupt
            (*(volatile uint32_t*)(USB_BASE + OFFSET_UDINTCLR))
                = 1 << UDINT_EORST;

            // Reset state
            _state = State::DEFAULT;
            _setupPacketAvailable = false;
            _doEnableAddress = false;

            // Recreate endpoints
            _nEndpoints = 0;
            for (int i = 0; i < N_EP_MAX; i++) {
                if (_endpoints[i].enabled) {
                    newEndpoint(_endpoints[i].type, 
                                _endpoints[i].direction, 
                                _endpoints[i].nBanks, 
                                _endpoints[i].size,
                                _endpoints[i].bank0,
                                _endpoints[i].bank1);
                } else {
                    break;
                }
            }
        } 

        // Start of frame
        if (udinte & udint & (1 << UDINT_SOF)) {
            // Clear interrupt
            (*(volatile uint32_t*)(USB_BASE + OFFSET_UDINTCLR))
                = 1 << UDINT_SOF;

            // Call user handler
            if (_startOfFrameHandler != nullptr) {
                _startOfFrameHandler();
            }
        }

        // Endpoints
        for (int i = 0; i < _nEndpoints; i++) {
            if (_endpoints[i].enabled && (udinte & udint & (1 << (UDINT_EP0INT + i)))) {
                // Check which packet type has been received in UESTA
                volatile uint32_t* uesta = (volatile uint32_t*)(USB_BASE + OFFSET_UESTA0 + i * 4);
                volatile uint32_t* uecon = (volatile uint32_t*)(USB_BASE + OFFSET_UECON0 + i * 4);
                EndpointConfig* ep = &_endpoints[i];

                // SETUP packet
                if (*uecon & *uesta & (1 << UESTA_RXSTPI)) {
                    // Call the handler to read the packet content
                    if (ep->handlers[static_cast<int>(EPHandlerType::SETUP)] != nullptr) {
                        ep->handlers[static_cast<int>(EPHandlerType::SETUP)](0);
                    }

                    // Clear interrupt to acknoledge the packet and free the bank
                    (*(volatile uint32_t*)(USB_BASE + OFFSET_UESTA0CLR + i * 4))
                        = 1 << UESTA_RXSTPI;

                }

                // IN packet
                // Note : this is not called when an IN packet is received, but whenever the bank is ready
                // to receive the content for the next IN packet. This is because the data must be sent 
                // immediately after the IN packet, with no time for a handler call, and must therefore be
                // prepared beforehand.
                if (*uecon & *uesta & (1 << UESTA_TXINI)) {
                    // Call the handler to write the packet content
                    int bytesToSend = 0;
                    if (ep->handlers[static_cast<int>(EPHandlerType::IN)] != nullptr) {
                        bytesToSend = ep->handlers[static_cast<int>(EPHandlerType::IN)](0);
                    }
                    _epRAMDescriptors[i * EP_DESCRIPTOR_SIZE + EP_PCKSIZE] = bytesToSend & PCKSIZE_BYTE_COUNT_MASK;
                    // Multi-packet mode is automatically enabled if BYTE_COUNT (ie bytesToSend) is larger than 
                    // the endpoint size (UECFG.EPSIZE)

                    // Enable AUTO_ZLP
                    //_epRAMDescriptors[i * EP_DESCRIPTOR_SIZE + EP_PCKSIZE] |= 1 << PCKSIZE_AUTO_ZLP;

                    // Clear interrupt (this will send the packet for a Control endpoint)
                    (*(volatile uint32_t*)(USB_BASE + OFFSET_UESTA0CLR + i * 4))
                        = 1 << UESTA_TXINI;

                    // If this is an IN endpoint, clear FIFOCON to free the bank and allow the
                    // hardware to send the data at the next IN packet
                    if (ep->type != EPType::CONTROL && ep->direction == EPDir::IN) {
                        (*(volatile uint32_t*)(USB_BASE + OFFSET_UECON0CLR + i * 4))
                            = 1 << UECON_FIFOCON;
                    }
                }
                
                // OUT packet
                if (*uecon & *uesta & (1 << UESTA_RXOUTI)) {
                    // Number of bytes received
                    int receivedPacketSize = _epRAMDescriptors[i * EP_DESCRIPTOR_SIZE + EP_PCKSIZE] & PCKSIZE_BYTE_COUNT_MASK;

                    // Call the handler to read the packet content
                    if (ep->handlers[static_cast<int>(EPHandlerType::OUT)] != nullptr) {
                        ep->handlers[static_cast<int>(EPHandlerType::OUT)](receivedPacketSize);
                    }

                    // Clear interrupt
                    (*(volatile uint32_t*)(USB_BASE + OFFSET_UESTA0CLR + i * 4))
                        = 1 << UESTA_RXOUTI;

                    // If this is an OUT endpoint, clear FIFOCON to free the bank
                    if (ep->type != EPType::CONTROL && ep->direction == EPDir::OUT) {
                        (*(volatile uint32_t*)(USB_BASE + OFFSET_UECON0CLR + i * 4))
                            = 1 << UECON_FIFOCON;
                    }

                    // If this is an IN or No Data transfer, an OUT packet means the transfer is finished or aborted
                    if (_lastSetupPacket.direction == EPDir::IN || _lastSetupPacket.wLength == 0) {
                        // Disable IN interrupt
                    }
                }
            }
        }
    }


    // Handlers for endpoint 0
    int ep0SETUPHandler(int unused) {
        // Read data received in bankEP0
        const int EP_N = 0; // Endpoint number : 0 in this handler
        uint8_t* bank = (uint8_t*)_epRAMDescriptors[EP_N * EP_DESCRIPTOR_SIZE + EP_ADDR];
        _lastSetupPacket = {
            .bmRequestType = bank[0],
            .bRequest = bank[1],
            .wValue = uint16_t(bank[2] | bank[3] << 8),
            .wIndex = uint16_t(bank[4] | bank[5] << 8),
            .wLength = uint16_t(bank[6] | bank[7] << 8),
            .direction = static_cast<EPDir>((bank[0] >> 7) & 0b1),
            .requestType = static_cast<SetupRequestType>((bank[0] >> 5) & 0b11),
            .recipent = static_cast<SetupRecipient>(bank[0] & 0b11111),
            .handled = false
        };

        // Kill any pending IN transfer
        abortINTransfer(0);

        // If this is an IN or No Data transfer
        if (_lastSetupPacket.direction == EPDir::IN || _lastSetupPacket.wLength == 0) {
            // Enable the IN interrupt to answer this request (or ACK it with a ZLP for a No Data transfer)
            enableINInterrupt(0);
        } else { // OUT
            // Configure the MULTI_PACKET_SIZE to allow multi-packet mode for OUT transfers
            _epRAMDescriptors[EP_N * EP_DESCRIPTOR_SIZE + EP_PCKSIZE] = (BANK_EP0_SIZE << PCKSIZE_MULTI_PACKET_SIZE) & PCKSIZE_MULTI_PACKET_SIZE_MASK;

            // Disable the IN interrupt to allow the OUT handler to be called
            disableINInterrupt(0);
        }

        // Mark that a SETUP packet has been received
        _setupPacketAvailable = true;

        return 0;
    }

    int ep0INHandler(int unused) {
        const int EP_N = 0; // Endpoint number : 0 in this handler

        // Answer the last SETUP packet received
        if (_setupPacketAvailable) {

            // Standard request
            if (_lastSetupPacket.requestType == SetupRequestType::STANDARD) {

                // GET_DESCRIPTOR standard request
                if (_lastSetupPacket.bRequest == USBREQ_GET_DESCRIPTOR) {
                    uint8_t* bank = (uint8_t*)_epRAMDescriptors[EP_N * EP_DESCRIPTOR_SIZE + EP_ADDR];

                    // Select descriptor to send
                    const uint8_t descriptorType = _lastSetupPacket.wValue >> 8;
                    const uint8_t descriptorIndex = _lastSetupPacket.wValue & 0xFF;

                    // Default device descriptor
                    if (descriptorType == USBDESC_DEVICE && descriptorIndex == 0) {
                        _lastSetupPacket.handled = true;

                        // Copy the descriptor to the bank
                        const int length = min(_deviceDescriptor.bLength, _lastSetupPacket.wLength);
                        memcpy(bank, (uint8_t*)&_deviceDescriptor, length);
                        return length;

                    // Default configuration and interface descriptors
                    } else if (descriptorType == USBDESC_CONFIGURATION && descriptorIndex == 0) {
                        _lastSetupPacket.handled = true;

                        // Copy the configuration and interface descriptors to the bank
                        uint8_t* bankCursor = bank;
                        int length = min(_configurationDescriptor.wTotalLength, _lastSetupPacket.wLength);
                        memcpy(bankCursor, (uint8_t*)&_configurationDescriptor, _configurationDescriptor.bLength);
                        bankCursor += _configurationDescriptor.bLength;
                        memcpy(bankCursor, (uint8_t*)&_interfaceDescriptor, _interfaceDescriptor.bLength);
                        bankCursor += _interfaceDescriptor.bLength;
                        for (int i = 1; i < _nEndpoints; i++) {
                            // This loop starts at 1 because the descriptor of endpoint 0 must not be sent
                            memcpy(bankCursor, (uint8_t*)&_endpoints[i].descriptor, _endpoints[i].descriptor.bLength);
                            bankCursor += _endpoints[i].descriptor.bLength;
                        }
                        return length;

                    // String0 descriptor (list of available languages)
                    } else if (descriptorType == USBDESC_STRING && descriptorIndex == 0) {
                        _lastSetupPacket.handled = true;

                        // Copy the descriptor to the bank
                        const int length = min(_string0Descriptor.bLength, _lastSetupPacket.wLength);
                        memcpy(bank, (uint8_t*)&_string0Descriptor, length);
                        return length;

                    // String descriptor
                    } else if (descriptorType == USBDESC_STRING && descriptorIndex <= static_cast<int>(StringDescriptors::NUMBER)) {
                        _lastSetupPacket.handled = true;

                        // Copy the descriptor to the bank
                        const StringDescriptor stringDescriptor = _stringDescriptors[descriptorIndex - 1];
                        const int length = min(stringDescriptor.bLength, _lastSetupPacket.wLength);
                        memcpy(bank, (uint8_t*)&(stringDescriptor), 2); // bLength and bDescriptorType
                        memcpy(bank + 2, stringDescriptor.bString, length - 2); // Actual string content
                        return length;

                    }

                // SET_ADDRESS standard request
                } else if (_lastSetupPacket.bRequest == USBREQ_SET_ADDRESS) {
                    _lastSetupPacket.handled = true;

                    // After a SET_ADDRESS request, the new address is configured (UDCON.UADD) but
                    // not enabled (UDCON.ADDEN) immediately because the device must finish the
                    // current transaction with the default address.
                    // The IN interrupt is triggered for the first just after the SETUP request to
                    // prepare the answer of the next IN transfer (request ACK). During this transfer
                    // the address is written to UDCON but must not be enabled yet. When the ACK transfer
                    // is sent, the bank is free once again and the IN interrupt is triggered a second
                    // time. Since there is no Status Stage in a SET_ADDRESS request, this is a good
                    // time to enable the address.
                    if (!_doEnableAddress) {
                        // Write address (disabled)
                        uint32_t udcon = (*(volatile uint32_t*)(USB_BASE + OFFSET_UDCON));
                        (*(volatile uint32_t*)(USB_BASE + OFFSET_UDCON))
                            = (udcon & ~(uint32_t)(0xFF << UDCON_UADD))
                            | _lastSetupPacket.wValue << UDCON_UADD;   // UADD : USB device address

                        // Enable the flag
                        _doEnableAddress = true;

                    } else {
                        // Enable address
                        (*(volatile uint32_t*)(USB_BASE + OFFSET_UDCON))
                            |= 1 << UDCON_ADDEN;   // ADDEN : enable address

                        // There is no Status phase for a SET_ADDRESS, 
                        // so disable IN interrupt now
                        (*(volatile uint32_t*)(USB_BASE + OFFSET_UECON0CLR))
                            = 1 << UECON_TXINE;
                        
                        // Move through the next enumeration step
                        _state = State::ADDRESS;

                        // Disable the flag
                        _doEnableAddress = false;
                    }

                    // Answer with a ZLP
                    return 0;

                // SET_CONFIGURATION standard request
                } else if (_lastSetupPacket.bRequest == USBREQ_SET_CONFIGURATION) {

                    if (_state == State::ADDRESS || _state == State::CONFIGURED) {

                        const uint8_t configurationId = _lastSetupPacket.wValue & 0xFF;

                        if (configurationId == 0) {
                            _lastSetupPacket.handled = true;
                            
                            // The device is no longer configured, reset to Address state
                            _state = State::ADDRESS;
                        
                            // Answer with a ZLP
                            return 0;

                        // There is only one configuration available
                        } else if (configurationId == 1) {
                            _lastSetupPacket.handled = true;

                            // The device is now configured
                            _state = State::CONFIGURED;
                        
                            // Answer with a ZLP
                            return 0;
                        }
                    }
                }

            } else if (_lastSetupPacket.requestType == SetupRequestType::VENDOR) {
                // Call user handler if this is a IN or No Data request. For an OUT request with data,
                // the user handler will be called by ep0OUTHandler() when the data has been received
                if (_controlHandler != nullptr) {
                    // Make sure the IN handler will not be called again when the IN response is sent
                    disableINInterrupt(0);

                    if (_lastSetupPacket.direction == EPDir::IN || _lastSetupPacket.wLength == 0) {
                        int bytesToSend = _controlHandler(_lastSetupPacket, _bankEP0, _lastSetupPacket.wLength);
                        return min(_lastSetupPacket.wLength, bytesToSend);
                    }
                    // For an OUT request, _lastSetupPacket.handled was set previously by ep0OUTHandler()
                }
            }

            // If the request couldn't be handled, send a STALL
            if (!_lastSetupPacket.handled) {
                (*(volatile uint32_t*)(USB_BASE + OFFSET_UECON0SET))
                    = 1 << UECON_STALLRQ;
                return 0;
            }

        } else {
            // No SETUP packet available
            disableINInterrupt(0);
        }

        return 0;
    }

    int ep0OUTHandler(int size) {
        const int EP_N = 0; // Endpoint number : 0 in this handler

        if (_lastSetupPacket.direction == EPDir::IN) {
            // This is an ACK from the host
            _setupPacketAvailable = false;
            disableINInterrupt(0);

        } else {
            // This is an OUT packet containing data
            if (_controlHandler != nullptr) {
                int size = _epRAMDescriptors[EP_N * EP_DESCRIPTOR_SIZE + EP_PCKSIZE] & PCKSIZE_BYTE_COUNT_MASK;
                _controlHandler(_lastSetupPacket, _bankEP0, size);
            }

            // Enable the IN interrupt to ACK the received data
            enableINInterrupt(0);
        }
        return 0;
    }

    // Handlers management
    // Set an endpoint handler
    void setEndpointHandler(Endpoint endpointNumber, EPHandlerType handlerType, int (*handler)(int)) {
        EndpointConfig* ep = &_endpoints[endpointNumber];
        if (ep->enabled) {
            ep->handlers[static_cast<int>(handlerType)] = handler;
        }
    }

    // Enable the IN interrupt on the specified endpoint
    void enableINInterrupt(Endpoint endpointNumber) {
        (*(volatile uint32_t*)(USB_BASE + OFFSET_UECON0SET + endpointNumber * 4))
            = 1 << UECON_TXINE;
    }

    // Clear the bank of the specified endpoint. This is useful if the bank data was prepared
    // by an IN handler, but no IN packet was sent by the host yet, and this data is now obsolete
    void abortINTransfer(Endpoint endpointNumber) {
        // Disable the IN interrupt
        disableINInterrupt(endpointNumber);

        _epRAMDescriptors[endpointNumber * EP_DESCRIPTOR_SIZE + EP_PCKSIZE] = 0;
        
        // Kill the bank
        (*(volatile uint32_t*)(USB_BASE + OFFSET_UECON0SET + endpointNumber * 4))
            = 1 << UECON_KILLBK;

        // Wait for the end of the procedure
        while ((*(volatile uint32_t*)(USB_BASE + OFFSET_UECON0 + endpointNumber * 4)) & 1 << UECON_KILLBK);
    }

    // Disable the IN interrupt on the specified endpoint
    void disableINInterrupt(Endpoint endpointNumber) {
        (*(volatile uint32_t*)(USB_BASE + OFFSET_UECON0CLR + endpointNumber * 4))
            = 1 << UECON_TXINE;
    }

    // Return true if the IN interrupt is enabled on the specified endpoint
    bool isINInterruptEnabled(Endpoint endpointNumber) {
        return (*(volatile uint32_t*)(USB_BASE + OFFSET_UECON0 + endpointNumber * 4)) & (1 << UECON_TXINE);
    }

    // Enable the OUT interrupt on the specified endpoint
    void enableOUTInterrupt(Endpoint endpointNumber) {
        (*(volatile uint32_t*)(USB_BASE + OFFSET_UECON0SET + endpointNumber * 4))
            = 1 << UECON_RXOUTE;
    }

    // Disable the OUT interrupt on the specified endpoint
    void disableOUTInterrupt(Endpoint endpointNumber) {
        (*(volatile uint32_t*)(USB_BASE + OFFSET_UECON0CLR + endpointNumber * 4))
            = 1 << UECON_RXOUTE;
    }

    // Return true if the OUT interrupt is enabled on the specified endpoint
    bool isOUTInterruptEnabled(Endpoint endpointNumber) {
        return (*(volatile uint32_t*)(USB_BASE + OFFSET_UECON0 + endpointNumber * 4)) & (1 << UECON_RXOUTE);
    }

    // User handlers
    void setConnectedHandler(void (*handler)()) {
        _connectedHandler = handler;
    }

    void setDisconnectedHandler(void (*handler)()) {
        _disconnectedHandler = handler;
    }

    void setStartOfFrameHandler(void (*handler)()) {
        _startOfFrameHandler = handler;
    }

    void setControlHandler(int (*handler)(SetupPacket &_lastSetupPacket, uint8_t* data, int size)) {
        _controlHandler = handler;
    }


    // Misc functions
    void remoteWakeup() {
        // TODO
        if (_state == State::SUSPEND) {
            (*(volatile uint32_t*)(USB_BASE + OFFSET_UDCON))
                |= 1 << UDCON_RMWKUP;
        }
    }
}