#ifndef _PINS_H_
#define _PINS_H_

#include <gpio.h>
#include <i2c.h>
#include <spi.h>

// I2C
const I2C::Port I2C_PORT = I2C::Port::I2C1;
const GPIO::Pin PIN_I2C_SDA = {GPIO::Port::B,  0, GPIO::Periph::A};
const GPIO::Pin PIN_I2C_SCL = {GPIO::Port::B,  1, GPIO::Periph::A};

// SPI
const GPIO::Pin PIN_MISO = {GPIO::Port::A, 27, GPIO::Periph::A};
const GPIO::Pin PIN_MOSI = {GPIO::Port::A, 28, GPIO::Periph::A};
const GPIO::Pin PIN_SCK = {GPIO::Port::A, 29, GPIO::Periph::A};

// OLED
const SPI::Peripheral SPI_SLAVE_OLED = 0;
const GPIO::Pin PIN_OLED_CS = {GPIO::Port::A, 30, GPIO::Periph::A};
const GPIO::Pin PIN_OLED_RES = GPIO::PB02;
const GPIO::Pin PIN_OLED_DC = GPIO::PB03;

// LoRa
const SPI::Peripheral SPI_SLAVE_LORA = 1;
const GPIO::Pin PIN_LORA_CS = {GPIO::Port::A, 31, GPIO::Periph::A};
const GPIO::Pin PIN_LORA_RESET = GPIO::PA09;
const GPIO::Pin PIN_LORA_DIO0 = GPIO::PA07;
const GPIO::Pin PIN_LORA_DIO1 = GPIO::PA06;
const GPIO::Pin PIN_LORA_DIO2 = GPIO::PB05;
const GPIO::Pin PIN_LORA_DIO3 = GPIO::PB06;
const GPIO::Pin PIN_LORA_DIO4 = GPIO::PA08;
const GPIO::Pin PIN_LORA_DIO5 = GPIO::PB07;

// Buttons
const GPIO::Pin PIN_BTN_UP = GPIO::PA19;
const GPIO::Pin PIN_BTN_DOWN = GPIO::PA10;
const GPIO::Pin PIN_BTN_LEFT = GPIO::PA11;
const GPIO::Pin PIN_BTN_RIGHT = GPIO::PA13;
const GPIO::Pin PIN_BTN_OK = GPIO::PA14;
const GPIO::Pin PIN_BTN_PW = GPIO::PA01;
const GPIO::Pin PIN_BTN_FOCUS = GPIO::PB04;
const GPIO::Pin PIN_BTN_TRIGGER = GPIO::PA18;

// LEDs
const GPIO::Pin PIN_LED_FOCUS = GPIO::PA20;
const GPIO::Pin PIN_LED_TRIGGER = GPIO::PB11;
const GPIO::Pin PIN_LED_INPUT = GPIO::PB15;

// Jack ports
const GPIO::Pin PIN_P1_T = GPIO::PA17;
const GPIO::Pin PIN_P1_T_PD = GPIO::PB08;
const GPIO::Pin PIN_P1_R1 = GPIO::PB09;
const GPIO::Pin PIN_P1_R1_PD = GPIO::PB10;
const GPIO::Pin PIN_P1_R2 = GPIO::PA12;
const GPIO::Pin PIN_P1_R2_PD = GPIO::PA16;
const GPIO::Pin PIN_P1_SW = GPIO::PA15;
const GPIO::Pin PIN_P2_T = GPIO::PB13;
const GPIO::Pin PIN_P2_T_PD = GPIO::PB12;
const GPIO::Pin PIN_P2_R1 = GPIO::PA23;
const GPIO::Pin PIN_P2_R1_PD = GPIO::PA24;
const GPIO::Pin PIN_P2_R2 = GPIO::PB14;
const GPIO::Pin PIN_P2_R2_PD = GPIO::PA22;
const GPIO::Pin PIN_P2_SW = GPIO::PA21;

// Power enable command
const GPIO::Pin PIN_PW_EN = GPIO::PA00;

// Battery voltage measurement
const GPIO::Pin PIN_VBAT_MEAS = {GPIO::Port::A,  4, GPIO::Periph::A};
const GPIO::Pin PIN_VBAT_MEAS_CMD = GPIO::PA05;

// Aliases for features of the jack ports
const GPIO::Pin PIN_INPUT = PIN_P2_T;
const GPIO::Pin PIN_FOCUS = PIN_P1_R1_PD;
const GPIO::Pin PIN_TRIGGER = PIN_P1_T_PD;

#endif