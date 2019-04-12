#ifndef _USB_H_
#define _USB_H_

#include <stdint.h>
#include "gpio.h"

// This module allows the chip to communicate on an USB
// bus, either as Device or Host. Only device mode is currently
// supported by the driver.
// References :
// http://www.beyondlogic.org/usbnutshell/usb1.shtml
// "USB Complete Fourth Edition" by Jan Axelson
namespace USB {

    // Peripheral memory space base address
    const uint32_t USB_BASE = 0x400A5000;

    // Registers addresses
    const uint32_t OFFSET_UDCON =      0x0000; // Device General Control Register
    const uint32_t OFFSET_UDINT =      0x0004; // Device Global Interrupt Register
    const uint32_t OFFSET_UDINTCLR =   0x0008; // Device Global Interrupt Clear Register
    const uint32_t OFFSET_UDINTSET =   0x000C; // Device Global Interrupt Set Register
    const uint32_t OFFSET_UDINTE =     0x0010; // Device Global Interrupt Enable Register
    const uint32_t OFFSET_UDINTECLR =  0x0014; // Device Global Interrupt Enable Clear Register
    const uint32_t OFFSET_UDINTESET =  0x0018; // Device Global Interrupt Enable Set Register
    const uint32_t OFFSET_UERST =      0x001C; // Endpoint Enable/Reset Register
    const uint32_t OFFSET_UDFNUM =     0x0020; // Device Frame Number Register
    const uint32_t OFFSET_UECFG0 =     0x0100; // Endpoint n Configuration Register
    const uint32_t OFFSET_UESTA0 =     0x0130; // Endpoint n Status Register
    const uint32_t OFFSET_UESTA0CLR =  0x0160; // Endpoint n Status Clear Register
    const uint32_t OFFSET_UESTA0SET =  0x0190; // Endpoint n Status Set Register
    const uint32_t OFFSET_UECON0 =     0x01C0; // Endpoint n Control Register
    const uint32_t OFFSET_UECON0SET =  0x01F0; // Endpoint n Control Set Register
    const uint32_t OFFSET_UECON0CLR =  0x0220; // Endpoint n Control Clear Register
    const uint32_t OFFSET_UHCON =      0x0400; // Host General Control Register
    const uint32_t OFFSET_UHINT =      0x0404; // Host Global Interrupt Register
    const uint32_t OFFSET_UHINTCLR =   0x0408; // Host Global Interrupt Clear Register
    const uint32_t OFFSET_UHINTSET =   0x040C; // Host Global Interrupt Set Register
    const uint32_t OFFSET_UHINTE =     0x0410; // Host Global Interrupt Enable Register
    const uint32_t OFFSET_UHINTECLR =  0x0414; // Host Global Interrupt Enable Clear Register
    const uint32_t OFFSET_UHINTESET =  0x0418; // Host Global Interrupt Enable Set Register
    const uint32_t OFFSET_UPRST =      0x041C; // Pipe Enable/Reset Register
    const uint32_t OFFSET_UHFNUM =     0x0420; // Host Frame Number Register
    const uint32_t OFFSET_UHSOFC =     0x0424; // Host Start Of Frame Control Register
    const uint32_t OFFSET_UPCFG0 =     0x0500; // Pipe n Configuration Register
    const uint32_t OFFSET_UPSTA0 =     0x0530; // Pipe n Status Register
    const uint32_t OFFSET_UPSTA0CLR =  0x0560; // Pipe n Status Clear Register
    const uint32_t OFFSET_UPSTA0SET =  0x0590; // Pipe n Status Set Register
    const uint32_t OFFSET_UPCON0 =     0x05C0; // Pipe n Control Register
    const uint32_t OFFSET_UPCON0SET =  0x05F0; // Pipe n Control Set Register
    const uint32_t OFFSET_UPCON0CLR =  0x0620; // Pipe n Control Clear Register
    const uint32_t OFFSET_UPINRQ0 =    0x0650; // Pipe n IN Request Register
    const uint32_t OFFSET_USBCON =     0x0800; // General Control Register
    const uint32_t OFFSET_USBSTA =     0x0804; // General Status Register
    const uint32_t OFFSET_USBSTACLR =  0x0808; // General Status Clear Register
    const uint32_t OFFSET_USBSTASET =  0x080C; // General Status Set Register
    const uint32_t OFFSET_UVERS =      0x0818; // IP Version Register
    const uint32_t OFFSET_UFEATURES =  0x081C; // IP Features Register
    const uint32_t OFFSET_UADDRSIZE =  0x0820; // IP PB Address Size Register
    const uint32_t OFFSET_UNAME1 =     0x0824; // IP Name Register 1
    const uint32_t OFFSET_UNAME2 =     0x0828; // IP Name Register 2
    const uint32_t OFFSET_USBFSM =     0x082C; // USB Finite State Machine Status Register
    const uint32_t OFFSET_UDESC =      0x0830; // USB Descriptor address

    // Subregisters
    const uint32_t USBCON_FRZCLK = 14;
    const uint32_t USBCON_USBE = 15;
    const uint32_t USBCON_UIMOD = 24; // Datasheet says 25, that's wrong
    const uint32_t USBSTA_VBUSRQ = 9;
    const uint32_t USBSTA_SPEED = 12;
    const uint32_t USBSTA_CLKUSABLE = 14;
    const uint32_t USBSTA_SUSPEND = 16;
    const uint32_t UDCON_UADD = 0;
    const uint32_t UDCON_ADDEN = 7;
    const uint32_t UDCON_DETACH = 8;
    const uint32_t UDCON_RMWKUP = 9;
    const uint32_t UDCON_LS = 12;
    const uint32_t UDCON_GNAK = 17;
    const uint32_t UDINT_SUSP = 0;
    const uint32_t UDINT_SOF = 2;
    const uint32_t UDINT_EORST = 3;
    const uint32_t UDINT_WAKEUP = 4;
    const uint32_t UDINT_EORSM = 5;
    const uint32_t UDINT_UPRSM = 6;
    const uint32_t UDINT_EP0INT = 12;
    const uint32_t UECFG_EPBK = 2;
    const uint32_t UECFG_EPSIZE = 4;
    const uint32_t UECFG_EPDIR = 8;
    const uint32_t UECFG_EPTYPE = 11;
    const uint32_t UECFG_REPNB = 17;
    const uint32_t UECON_TXINE = 0;
    const uint32_t UECON_RXOUTE = 1;
    const uint32_t UECON_RXSTPE = 2;
    const uint32_t UECON_ERRORFE = 2;
    const uint32_t UECON_NAKOUTE = 3;
    const uint32_t UECON_NAKINE = 4;
    const uint32_t UECON_STALLEDE = 6;
    const uint32_t UECON_CRCERRE = 6;
    const uint32_t UECON_NREPLY = 8;
    const uint32_t UECON_RAMACERE = 11;
    const uint32_t UECON_NBUSYBKE = 12;
    const uint32_t UECON_KILLBK = 13;
    const uint32_t UECON_FIFOCON = 14;
    const uint32_t UECON_RSTDT = 18;
    const uint32_t UECON_STALLRQ = 19;
    const uint32_t UECON_BUSY0E = 24;
    const uint32_t UECON_BUSY1E = 25;
    const uint32_t UESTA_TXINI = 0;
    const uint32_t UESTA_RXOUTI = 1;
    const uint32_t UESTA_RXSTPI = 2;
    const uint32_t UESTA_ERRORFI = 2;
    const uint32_t UESTA_NAKOUTI = 3;
    const uint32_t UESTA_NAKINI = 4;
    const uint32_t UESTA_STALLEDI = 6;
    const uint32_t UESTA_CRCERRI = 6;
    const uint32_t UESTA_DTSEQ = 8;
    const uint32_t UESTA_RAMACERI = 11;
    const uint32_t UESTA_NBUSYBK = 12;
    const uint32_t UESTA_CURRBK = 14;
    const uint32_t UESTA_CTRLDIR = 17;

    // Endpoints RAM registers
    const int N_EP_MAX = 8;
    const int EP_DESCRIPTOR_SIZE = 8;
    const int EP_DESCRIPTOR_BANK_SIZE = 4;
    const int EP_ADDR = 0;
    const int EP_PCKSIZE = 1;
    const int PCKSIZE_BYTE_COUNT = 0;
    const int PCKSIZE_BYTE_COUNT_MASK = 0x00007FFFF;
    const int PCKSIZE_MULTI_PACKET_SIZE = 16;
    const int PCKSIZE_MULTI_PACKET_SIZE_MASK = 0x7FFF0000;
    const int PCKSIZE_AUTO_ZLP = 31;
    const int EP_CTR_STA = 2;
    const int CTR_STA_STALLRQ_NEXT = 0;
    const int CTR_STA_CRCERR = 16;
    const int CTR_STA_OVERF = 17;
    const int CTR_STA_UNDERF = 18;

    // Error codes
    const Error::Code ERR_CLOCK_NOT_USABLE = 0x0001;


    // USB Device states
    enum class State {
        SUSPEND,
        POWERED,
        DEFAULT,
        ADDRESS,
        CONFIGURED
    };


    // USB requests codes
    const int USBREQ_GET_STATUS = 0;
    const int USBREQ_CLEAR_FAILURE = 1;
    const int USBREQ_SET_FEATURE = 3;
    const int USBREQ_SET_ADDRESS = 5;
    const int USBREQ_GET_DESCRIPTOR = 6;
    const int USBREQ_SET_DESCRIPTOR = 7;
    const int USBREQ_GET_CONFIGURATION = 8;
    const int USBREQ_SET_CONFIGURATION = 9;
    const int USBREQ_GET_INTERFACE = 10;
    const int USBREQ_SET_INTERFACE = 11;
    const int USBREQ_SYNC_FRAME = 12;


    // USB descriptors
    const int USBDESC_DEVICE = 1;
    const int USBDESC_CONFIGURATION = 2;
    const int USBDESC_STRING = 3;
    const int USBDESC_INTERFACE = 4;
    const int USBDESC_ENDPOINT = 5;
    const int USBDESC_DEVICE_QUALIFIER = 6;
    const int USBDESC_OTHER_SPEED_CONFIGURATION = 7;
    const int USBDESC_INTERFACE_POWER = 8;
    struct DeviceDescriptor {
        uint8_t bLength;
        uint8_t bDescriptorType;
        uint16_t bcdUSB;
        uint8_t bDeviceClass;
        uint8_t bDeviceSubClass;
        uint8_t bDeviceProtocol;
        uint8_t bMaxPacketSize0;
        uint16_t idVendor;
        uint16_t idProduct;
        uint16_t bcdDevice;
        uint8_t iManufacturer;
        uint8_t iProduct;
        uint8_t iSerialNumber;
        uint8_t bNumConfigurations;
    };
    struct ConfigurationDescriptor {
        uint8_t bLength;
        uint8_t bDescriptorType;
        uint16_t wTotalLength;
        uint8_t bNumInterfaces;
        uint8_t bConfigurationValue;
        uint8_t iConfiguration;
        uint8_t bmAttributes;
        uint8_t bMaxPower;
    };
    struct InterfaceDescriptor {
        uint8_t bLength;
        uint8_t bDescriptorType;
        uint8_t bInterfaceNumber;
        uint8_t bAlternateSetting;
        uint8_t bNumEndpoints;
        uint8_t bInterfaceClass;
        uint8_t bInterfaceSubclass;
        uint8_t bInterfaceProtocol;
        uint8_t iInterface;
    };
    struct EndpointDescriptor {
        uint8_t bLength;
        uint8_t bDescriptorType;
        uint8_t bEndpointAddress;
        uint8_t bmAttributes;
        uint16_t wMaxPacketSize;
        uint8_t bInterval;
    };
    struct String0Descriptor {
        uint8_t bLength;
        uint8_t bDescriptorType;
        uint16_t wLANGID0;
    };
    struct StringDescriptor {
        uint8_t bLength;
        uint8_t bDescriptorType;
        const char16_t* bString;
    };
    extern DeviceDescriptor _deviceDescriptor;
    extern ConfigurationDescriptor _configurationDescriptor;
    extern InterfaceDescriptor _interfaceDescriptor;

    // String descriptors
    enum class StringDescriptors {
        MANUFACTURER,
        PRODUCT,
        SERIALNUMBER,

        NUMBER // Number of string descriptors
    };
    const int MAX_STRING_DESCRIPTOR_SIZE = 30;
    const char16_t DEFAULT_IMANUFACTURER[] = u"libtungsten";
    const char16_t DEFAULT_IPRODUCT[] = u"Carbide";
    const char16_t DEFAULT_SERIALNUMBER[] = u"beta";
    extern String0Descriptor _string0Descriptor;
    extern StringDescriptor _stringDescriptors[];

    // Default descriptor ids
    const uint16_t DEFAULT_VENDOR_ID = 0x03eb; // Atmel Corp vendor ID
    const uint16_t DEFAULT_PRODUCT_ID = 0xcabd;
    const uint16_t DEFAULT_DEVICE_REVISION = 0x0001;


    // Endpoints
    enum class EPBanks {
        SINGLE = 0,
        DOUBLE = 1
    };
    enum class EPSize {
        SIZE8 = 0b000,
        SIZE16 = 0b001,
        SIZE32 = 0b010,
        SIZE64 = 0b011,
        SIZE128 = 0b100, // /!\ EP sizes above 64 are only possible for
        SIZE256 = 0b101, // isochronous transfers in low- and full-speed,
        SIZE512 = 0b110, // according to the USB specification
        SIZE1024 = 0b111
    };
    const int EP_SIZES[] = {
        8,
        16,
        32,
        64,
        128,
        256,
        512,
        1024
    };
    enum class EPDir {
        OUT = 0,
        IN = 1
    };
    enum class EPType {
        CONTROL = 0b00,
        ISOCHRONOUS = 0b01,
        BULK = 0b10,
        INTERRUPT = 0b11
    };
    enum class EPHandlerType {
        SETUP,
        IN,
        OUT,

        NUMBER
    };
    struct EndpointConfig {
        bool enabled;
        EPType type;
        EPDir direction;
        EPBanks nBanks;
        EPSize size;
        uint8_t* bank0;
        uint8_t* bank1;
        int (*handlers[static_cast<int>(EPHandlerType::NUMBER)])(int); // Array of function pointers of EPHandlerType
        EndpointDescriptor descriptor;
    };
    using Endpoint = int; // Helper type to manage endpoints, created by newEndpoint()
    const Endpoint EP_ERROR = -1;
    extern const int BANK_EP0_SIZE;


    // Packets
    enum class SetupRequestType {
        STANDARD =  0b00,
        CLASS =     0b01,
        VENDOR =    0b10
    };
    enum class SetupRecipient {
        DEVICE =    0b00000,
        INTERFACE = 0b00001,
        ENDPOINT =  0b00010,
        OTHER =     0b00011
    };
    struct SetupPacket {
        uint8_t bmRequestType;
        uint8_t bRequest;
        uint16_t wValue;
        uint16_t wIndex;
        uint16_t wLength;
        EPDir direction;
        SetupRequestType requestType;
        SetupRecipient recipent;
        bool handled;
    };


    // Package-dependant, defined in pins_sam4l_XX.cpp
    extern const struct GPIO::Pin PIN_DM;
    extern const struct GPIO::Pin PIN_DP;


    // Module API
    void initDevice(uint16_t vendorId=DEFAULT_VENDOR_ID, uint16_t productId=DEFAULT_PRODUCT_ID, uint16_t deviceRevision=DEFAULT_DEVICE_REVISION);
    void setStringDescriptor(StringDescriptors descriptor, const char* string, int size);
    Endpoint newEndpoint(EPType type, EPDir direction, EPBanks nBanks, EPSize size, uint8_t* bank0, uint8_t* bank1=nullptr);
    void setConnectedHandler(void (*handler)());
    void setDisconnectedHandler(void (*handler)());
    void setStartOfFrameHandler(void (*handler)());
    void setControlHandler(int (*handler)(SetupPacket &_lastSetupPacket, uint8_t* data, int size));
    void setEndpointHandler(Endpoint endpointNumber, EPHandlerType handlerType, int (*handler)(int));
    void enableINInterrupt(Endpoint endpointNumber);
    void abortINTransfer(Endpoint endpointNumber);
    void disableINInterrupt(Endpoint endpointNumber);
    bool isINInterruptEnabled(Endpoint endpointNumber);
    void enableOUTInterrupt(Endpoint endpointNumber);
    void disableOUTInterrupt(Endpoint endpointNumber);
    bool isOUTInterruptEnabled(Endpoint endpointNumber);
    void remoteWakeup();
    void interruptHandler();
    int ep0SETUPHandler(int unused);
    int ep0INHandler(int unused);
    int ep0OUTHandler(int size);
}

#endif