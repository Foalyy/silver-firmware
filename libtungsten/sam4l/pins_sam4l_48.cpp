#include "scif.h"
#include "gpio.h"
#include "usb.h"
#include "usart.h"
#include "adc.h"
#include "dac.h"
#include "tc.h"
#include "i2c.h"
#include "spi.h"
#include "gloc.h"
#include "eic.h"

// This file defines the default pin mapping for peripherals.
// Most of the chip peripherals can map their signals on a few different
// pins. The list of available functions for each pin is defined in the
// datasheet, §3.2.1 Multiplexed Signals. Use the setPin() functions of
// each module to modify the default mapping.

namespace ADC {

    GPIO::Pin PINS[] = {
        {GPIO::Port::A,  4, GPIO::Periph::A}, // ADC0
        {GPIO::Port::A,  5, GPIO::Periph::A}, // ADC1
        {GPIO::Port::A,  7, GPIO::Periph::A}, // ADC2
    };

}

namespace DAC {

    GPIO::Pin PIN_VOUT = {GPIO::Port::A,  6, GPIO::Periph::A};

}

namespace EIC {

    GPIO::Pin PINS[] = {
        {}, // EXTINT0/NMI isn't available on this package
        {GPIO::Port::A,  6, GPIO::Periph::C}, // EXTINT1
        {GPIO::Port::A,  4, GPIO::Periph::C}, // EXTINT2
        {GPIO::Port::A,  5, GPIO::Periph::C}, // EXTINT3
        {GPIO::Port::A,  7, GPIO::Periph::C}, // EXTINT4
        {GPIO::Port::A, 20, GPIO::Periph::C}, // EXTINT5
        {GPIO::Port::A, 21, GPIO::Periph::C}, // EXTINT6
        {GPIO::Port::A, 22, GPIO::Periph::C}, // EXTINT7
        {GPIO::Port::A, 23, GPIO::Periph::C}, // EXTINT8
    };

    // Alternatives
    //{GPIO::Port::A, 16, GPIO::Periph::C}, // EXTINT1
    //{GPIO::Port::A, 17, GPIO::Periph::C}, // EXTINT2
    //{GPIO::Port::A, 18, GPIO::Periph::C}, // EXTINT3
    //{GPIO::Port::A, 19, GPIO::Periph::C}, // EXTINT4

}

namespace GLOC {

    GPIO::Pin PINS_IN[][4] = {
        {
            {GPIO::Port::A,  6, GPIO::Periph::D}, // GLOC0 IN0
            {GPIO::Port::A,  4, GPIO::Periph::D}, // GLOC0 IN1
            {GPIO::Port::A,  5, GPIO::Periph::D}, // GLOC0 IN2
            {GPIO::Port::A,  7, GPIO::Periph::D}  // GLOC0 IN3
        },
        {
            {GPIO::Port::A, 27, GPIO::Periph::D}, // GLOC1 IN4
            {GPIO::Port::A, 28, GPIO::Periph::D}, // GLOC1 IN5
            {GPIO::Port::A, 29, GPIO::Periph::D}, // GLOC1 IN6
            {GPIO::Port::A, 30, GPIO::Periph::D}  // GLOC1 IN7
        }
    };

    GPIO::Pin PINS_OUT[] = {
        {GPIO::Port::A,  8, GPIO::Periph::D}, // GLOC0 OUT
        {GPIO::Port::A, 31, GPIO::Periph::D}  // GLOC1 OUT
    };

    // Alternatives for GLOC0
    //{GPIO::Port::A, 20, GPIO::Periph::D}, // GLOC0 IN0
    //{GPIO::Port::A, 21, GPIO::Periph::D}, // GLOC0 IN1
    //{GPIO::Port::A, 22, GPIO::Periph::D}, // GLOC0 IN2
    //{GPIO::Port::A, 23, GPIO::Periph::D}  // GLOC0 IN3
    //{GPIO::Port::A, 24, GPIO::Periph::D}, // GLOC0 OUT

}

namespace I2C {

    // SDA
    GPIO::Pin PINS_SDA[] = {
        {GPIO::Port::A, 23, GPIO::Periph::B}, // I2C0 SDA
        {},                                   // I2C1 isn't available on this packages
        {GPIO::Port::A, 21, GPIO::Periph::E}, // I2C2 SDA
    };

    // SCL
    GPIO::Pin PINS_SCL[] = {
        {GPIO::Port::A, 24, GPIO::Periph::B}, // I2C0 SCL
        {},                                   // I2C1 isn't available on this packages
        {GPIO::Port::A, 22, GPIO::Periph::E}, // I2C2 SCL
    };

}

namespace SCIF {

    GPIO::Pin PINS_GCLK[] = {
        {GPIO::Port::A, 19, GPIO::Periph::E}, // GCLK0
        {GPIO::Port::A, 20, GPIO::Periph::E}, // GCLK1
    };

    GPIO::Pin PINS_GCLK_IN[] = {
        {GPIO::Port::A, 23, GPIO::Periph::E}, // GCLK_IN0
        {GPIO::Port::A, 24, GPIO::Periph::E}  // GCLK_IN1
    };

    // Alternatives for GCLK0
    //{GPIO::Port::A,  2, GPIO::Periph::A}
}

namespace SPI {

    GPIO::Pin PIN_MISO =  {GPIO::Port::A, 27, GPIO::Periph::A};
    GPIO::Pin PIN_MOSI =  {GPIO::Port::A, 28, GPIO::Periph::A};
    GPIO::Pin PIN_SCK =   {GPIO::Port::A, 29, GPIO::Periph::A};
    GPIO::Pin PIN_NPCS0 = {GPIO::Port::A, 30, GPIO::Periph::A};
    GPIO::Pin PIN_NPCS1 = {GPIO::Port::A, 31, GPIO::Periph::A};
    GPIO::Pin PIN_NPCS2 = {GPIO::Port::A, 14, GPIO::Periph::C};
    GPIO::Pin PIN_NPCS3 = {GPIO::Port::A, 15, GPIO::Periph::C};

    // Alternatives for MISO
    //const GPIO::Pin PIN_MISO =  {GPIO::Port::A,  3, GPIO::Periph::B};
    //const GPIO::Pin PIN_MISO =  {GPIO::Port::A, 21, GPIO::Periph::A};

    // Alternatives for MOSI
    //const GPIO::Pin PIN_MOSI =  {GPIO::Port::A, 22, GPIO::Periph::A};

    // Alternatives for SCK
    //const GPIO::Pin PIN_SCK =   {GPIO::Port::A, 23, GPIO::Periph::A};

    // Alternatives for NPCS0
    //const GPIO::Pin PIN_NPCS0 = {GPIO::Port::A,  2, GPIO::Periph::B};
    //const GPIO::Pin PIN_NPCS0 = {GPIO::Port::A, 24, GPIO::Periph::A};

    // Alternatives for NPCS1
    //const GPIO::Pin PIN_NPCS1 = {GPIO::Port::A, 13, GPIO::Periph::C};

    // No alternatives for NPCS2 ou NPCS3

}

namespace TC {

    const uint8_t N_TC = 1;

    GPIO::Pin PINS[MAX_N_TC][N_COUNTERS_PER_TC * N_CHANNELS_PER_COUNTER] = {
        {
            {GPIO::Port::A,  8, GPIO::Periph::B}, // TC0 A0
            {GPIO::Port::A,  9, GPIO::Periph::B}, // TC0 B0
            {GPIO::Port::A, 10, GPIO::Periph::B}, // TC0 A1
            {GPIO::Port::A, 11, GPIO::Periph::B}, // TC0 B1
            {GPIO::Port::A, 12, GPIO::Periph::B}, // TC0 A2
            {GPIO::Port::A, 13, GPIO::Periph::B}  // TC0 B2
        }
    };

    GPIO::Pin PINS_CLK[MAX_N_TC][N_EXTERNAL_CLOCKS_PER_TC] = {
        {
            {GPIO::Port::A, 14, GPIO::Periph::B}, // TC0 CLK0
            {GPIO::Port::A, 15, GPIO::Periph::B}, // TC0 CLK1
            {GPIO::Port::A, 16, GPIO::Periph::B}, // TC0 CLK2
        }
    };

}

namespace USART {

    // RX
    GPIO::Pin PINS_RX[] = {
        {GPIO::Port::A, 11, GPIO::Periph::A}, // USART0 RX
        {GPIO::Port::A, 15, GPIO::Periph::A}, // USART1 RX
        {GPIO::Port::A, 19, GPIO::Periph::A}, // USART2 RX
        {GPIO::Port::A, 30, GPIO::Periph::E}  // USART3 RX
    };

    // TX
    GPIO::Pin PINS_TX[] = {
        {GPIO::Port::A, 12, GPIO::Periph::A}, // USART0 TX
        {GPIO::Port::A, 16, GPIO::Periph::A}, // USART1 TX
        {GPIO::Port::A, 20, GPIO::Periph::A}, // USART2 TX
        {GPIO::Port::A, 31, GPIO::Periph::E}  // USART3 TX
    };

    // RTS
    GPIO::Pin PINS_RTS[] = {
        {GPIO::Port::A,  8, GPIO::Periph::A}, // USART0 RTS
        {GPIO::Port::A, 13, GPIO::Periph::A}, // USART1 RTS
        {GPIO::Port::A, 17, GPIO::Periph::A}, // USART2 RTS
        {GPIO::Port::A, 27, GPIO::Periph::E}  // USART3 RTS
    };

    // CTS
    GPIO::Pin PINS_CTS[] = {
        {GPIO::Port::A,  9, GPIO::Periph::A}, // USART0 CTS
        {GPIO::Port::A, 21, GPIO::Periph::B}, // USART1 CTS
        {GPIO::Port::A, 22, GPIO::Periph::B}, // USART2 CTS
        {GPIO::Port::A, 28, GPIO::Periph::E}  // USART3 CTS
    };


    // Alternatives for USART0
    // Be careful when using these pins that they are not already used for something else

    //{GPIO::Port::A,  5, GPIO::Periph::B} // RX
    //{GPIO::Port::A,  7, GPIO::Periph::B} // TX
    //{GPIO::Port::A,  6, GPIO::Periph::B} // RTS


    // Alternatives for USART2

    //{GPIO::Port::A, 25, GPIO::Periph::B} // RX
    //{GPIO::Port::A, 26, GPIO::Periph::B} // TX

}

namespace USB {

    const GPIO::Pin PIN_DM = {GPIO::Port::A, 25, GPIO::Periph::A};
    const GPIO::Pin PIN_DP = {GPIO::Port::A, 26, GPIO::Periph::A};

}