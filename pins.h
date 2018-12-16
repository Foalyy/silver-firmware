#ifndef _PINS_H_
#define _PINS_H_

#include <gpio.h>

// SPI
const GPIO::Pin PIN_MISO = {GPIO::Port::A, 27, GPIO::Periph::A};
const GPIO::Pin PIN_MOSI = {GPIO::Port::A, 28, GPIO::Periph::A};
const GPIO::Pin PIN_SCK = {GPIO::Port::A, 29, GPIO::Periph::A};
const GPIO::Pin PIN_CS0 = {GPIO::Port::A, 30, GPIO::Periph::A};
const GPIO::Pin PIN_CS1 = {GPIO::Port::A, 31, GPIO::Periph::A};

// OLED
const GPIO::Pin PIN_OLED_RES = GPIO::PA22;
const GPIO::Pin PIN_OLED_DC = GPIO::PA21;

// Buttons
const GPIO::Pin PIN_BTN_UP = GPIO::PA18;
const GPIO::Pin PIN_BTN_DOWN = GPIO::PB09;
const GPIO::Pin PIN_BTN_LEFT = GPIO::PB08;
const GPIO::Pin PIN_BTN_RIGHT = GPIO::PB11;
const GPIO::Pin PIN_BTN_OK = GPIO::PB10;
const GPIO::Pin PIN_BTN_PW = GPIO::PB14;
const GPIO::Pin PIN_BTN_TRIGGER = GPIO::PA08;

const GPIO::Pin PIN_INPUT = GPIO::PB02;
const GPIO::Pin PIN_FOCUS = GPIO::PA09;
const GPIO::Pin PIN_TRIGGER = GPIO::PA12;
const GPIO::Pin PIN_PW_EN = GPIO::PB15;
const GPIO::Pin PIN_LED_PW = GPIO::PB13;
const GPIO::Pin PIN_LED_TRIGGER = GPIO::PB12;
const GPIO::Pin PIN_VBAT_MEAS = {GPIO::Port::A,  4, GPIO::Periph::A};
const GPIO::Pin PIN_VBAT_MEAS_CMD = GPIO::PA05;

#endif