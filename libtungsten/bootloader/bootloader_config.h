#ifndef _BOOTLOADER_CONFIG_H_

#include <gpio.h>
#include <usart.h>
#include <usb.h>
#include <stdint.h>

// ## Bootloader configuration

// If Input mode is enabled, the bootloader will check the value of the PIN_INPUT pin
// at startup and will be activated only if this pin is in the given PIN_INPUT_STATE.
// An optional pulling can be activated to default to a given state if no voltage is applied
// on the input.
// A common scenario is to set the input on a button wired to short the line to ground when
// pressed, with PIN_INPUT_STATE = GPIO::LOW and PIN_INPUT_PULLING = GPIO::Pulling::PULLUP.
// With this configuration, the pullup will raise the line to HIGH by default, and the
// bootloader will be activated only if the line is forced to LOW by pressing the button
// during startup.
const bool MODE_INPUT = true;
const GPIO::Pin INPUT_PIN = GPIO::PB10;
const GPIO::PinState INPUT_PIN_STATE = GPIO::LOW;
const GPIO::Pulling INPUT_PIN_PULLING = GPIO::Pulling::PULLUP;

// If Timeout mode is enabled, the bootloader will always be activated when the chip
// is powered up, but will automatically exit and reset after the given TIMEOUT_DELAY
// (in milliseconds) if no connection was detected.
const bool MODE_TIMEOUT = false;
const unsigned int TIMEOUT_DELAY = 3000; // ms

// Enable the USB channel to allow the bootloader to register as a USB device on a connected
// host. This is the easiest way to upload programs to the microcontroller using the
// codeuploader tool included with the library.
const bool CHANNEL_USB_ENABLED = true;
const uint16_t USB_VENDOR_ID = USB::DEFAULT_VENDOR_ID;
const uint16_t USB_PRODUCT_ID = 0xcbd0;

// Enable the USART channel to allow the bootloader to receive instructions via a serial
// connection. The port, pins and baudrate can be customized. See pins_sam4l_XX.cpp
// for the list of available pins.
const bool CHANNEL_USART_ENABLED = false;
const USART::Port USART_PORT = USART::Port::USART0;
const int USART_BAUDRATE = 115200;
const GPIO::Pin USART_PIN_RX = {GPIO::Port::A, 11, GPIO::Periph::A};
const GPIO::Pin USART_PIN_TX = {GPIO::Port::A, 12, GPIO::Periph::A};
const int USART_TIMEOUT = 3000;

// The bootloader can flash some LEDs to show its status. These can be enabled/disabled
// and customized here. The LED_POLARITY option specifies the state to set to turn the
// LED on.
const bool LED_BL_ENABLED = true;
const GPIO::Pin PIN_LED_BL = GPIO::PB13;
const bool LED_WRITE_ENABLED = true;
const GPIO::Pin PIN_LED_WRITE = GPIO::PB12;
const bool LED_ERROR_ENABLED = true;
const GPIO::Pin PIN_LED_ERROR = GPIO::PB12;
const GPIO::PinState LED_POLARITY = GPIO::LOW;
const unsigned int LED_BL_BLINK_DELAY_STANDBY = 200; // ms
const unsigned int LED_BL_BLINK_DELAY_CONNECTED = 50; // ms

#endif