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
        {GPIO::Port::B,  2, GPIO::Periph::A}, // ADC3
        {GPIO::Port::B,  3, GPIO::Periph::A}, // ADC4
        {GPIO::Port::B,  4, GPIO::Periph::A}, // ADC5
        {GPIO::Port::B,  5, GPIO::Periph::A}, // ADC6
        {GPIO::Port::C,  7, GPIO::Periph::A}, // ADC7
        {GPIO::Port::C,  8, GPIO::Periph::A}, // ADC8
        {GPIO::Port::C,  9, GPIO::Periph::A}, // ADC9
        {GPIO::Port::C, 10, GPIO::Periph::A}, // ADC10
        {GPIO::Port::C, 11, GPIO::Periph::A}, // ADC11
        {GPIO::Port::C, 12, GPIO::Periph::A}, // ADC12
        {GPIO::Port::C, 13, GPIO::Periph::A}, // ADC13
        {GPIO::Port::C, 14, GPIO::Periph::A}  // ADC14
    };

}

namespace DAC {

    GPIO::Pin PIN_VOUT = {GPIO::Port::A,  6, GPIO::Periph::A};

}

namespace EIC {

    GPIO::Pin PINS[] = {
        {GPIO::Port::B,  1, GPIO::Periph::C}, // EXTINT0/NMI
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
    //{GPIO::Port::C, 24, GPIO::Periph::B}, // EXTINT1
    //{GPIO::Port::A, 17, GPIO::Periph::C}, // EXTINT2
    //{GPIO::Port::C, 25, GPIO::Periph::B}, // EXTINT2
    //{GPIO::Port::A, 18, GPIO::Periph::C}, // EXTINT3
    //{GPIO::Port::C, 26, GPIO::Periph::B}, // EXTINT3
    //{GPIO::Port::A, 19, GPIO::Periph::C}, // EXTINT4
    //{GPIO::Port::C, 27, GPIO::Periph::B}, // EXTINT4
    //{GPIO::Port::C,  3, GPIO::Periph::B}, // EXTINT5
    //{GPIO::Port::C,  4, GPIO::Periph::B}, // EXTINT6
    //{GPIO::Port::C,  5, GPIO::Periph::B}, // EXTINT7
    //{GPIO::Port::C,  6, GPIO::Periph::B}, // EXTINT8

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
        {GPIO::Port::A,  8, GPIO::Periph::D}, // GLOC0 OUT0
        {GPIO::Port::A, 31, GPIO::Periph::D}  // GLOC1 OUT1
    };

    // Alternatives for GLOC0
    //{GPIO::Port::A, 20, GPIO::Periph::D}, // GLOC0 IN0
    //{GPIO::Port::A, 21, GPIO::Periph::D}, // GLOC0 IN1
    //{GPIO::Port::A, 22, GPIO::Periph::D}, // GLOC0 IN2
    //{GPIO::Port::A, 23, GPIO::Periph::D}  // GLOC0 IN3
    //{GPIO::Port::A, 24, GPIO::Periph::D}, // GLOC0 OUT0

    // Alternatives for GLOC1
    //{GPIO::Port::B,  6, GPIO::Periph::C}, // GLOC1 IN4
    //{GPIO::Port::B,  7, GPIO::Periph::C}, // GLOC1 IN5
    //{GPIO::Port::B,  8, GPIO::Periph::C}, // GLOC1 IN6
    //{GPIO::Port::B,  9, GPIO::Periph::C}  // GLOC1 IN7
    //{GPIO::Port::B, 10, GPIO::Periph::C}  // GLOC1 OUT1

    //{GPIO::Port::C, 15, GPIO::Periph::D}, // GLOC1 IN4
    //{GPIO::Port::C, 16, GPIO::Periph::D}, // GLOC1 IN5
    //{GPIO::Port::C, 17, GPIO::Periph::D}, // GLOC1 IN6
    //{GPIO::Port::C, 18, GPIO::Periph::D}  // GLOC1 IN7
    //{GPIO::Port::C, 19, GPIO::Periph::D}  // GLOC1 OUT1

    //{GPIO::Port::C, 28, GPIO::Periph::C}, // GLOC1 IN4
    //{GPIO::Port::C, 29, GPIO::Periph::C}, // GLOC1 IN5
    //{GPIO::Port::C, 30, GPIO::Periph::C}, // GLOC1 IN6
    //{GPIO::Port::C, 31, GPIO::Periph::C}  // GLOC1 OUT1

}

namespace I2C {

    // SDA
    GPIO::Pin PINS_SDA[] = {
        {GPIO::Port::A, 23, GPIO::Periph::B}, // I2C0 SDA
        {GPIO::Port::B,  0, GPIO::Periph::A}, // I2C1 SDA
        {GPIO::Port::A, 21, GPIO::Periph::E}, // I2C2 SDA
        {GPIO::Port::B, 14, GPIO::Periph::C}  // I2C3 SDA
    };

    // SCL
    GPIO::Pin PINS_SCL[] = {
        {GPIO::Port::A, 24, GPIO::Periph::B}, // I2C0 SCL
        {GPIO::Port::B,  1, GPIO::Periph::A}, // I2C1 SCL
        {GPIO::Port::A, 22, GPIO::Periph::E}, // I2C2 SCL
        {GPIO::Port::B, 15, GPIO::Periph::C}  // I2C3 SCL
    };

}

namespace SCIF {

    GPIO::Pin PINS_GCLK[] = {
        {GPIO::Port::B, 10, GPIO::Periph::E}, // GCLK0
        {GPIO::Port::B, 11, GPIO::Periph::E}, // GCLK1
        {GPIO::Port::B, 12, GPIO::Periph::E}, // GCLK2
        {GPIO::Port::B, 13, GPIO::Periph::E}  // GCLK3
    };

    GPIO::Pin PINS_GCLK_IN[] = {
        {GPIO::Port::B, 14, GPIO::Periph::E}, // GCLK_IN0
        {GPIO::Port::B, 15, GPIO::Periph::E}  // GCLK_IN1
    };

    // Alternatives for GCLK0
    //{GPIO::Port::A,  2, GPIO::Periph::A}
    //{GPIO::Port::A, 19, GPIO::Periph::E}
    //{GPIO::Port::C, 26, GPIO::Periph::E}
    
    // Alternatives for GCLK1
    //{GPIO::Port::A, 20, GPIO::Periph::E}
    //{GPIO::Port::C, 27, GPIO::Periph::E}

    // Alternatives for GCLK2
    //{GPIO::Port::C, 28, GPIO::Periph::E}

    // Alternatives for GCLK3
    //{GPIO::Port::C, 29, GPIO::Periph::E}

    // Alternatives for GCLK_IN0
    //{GPIO::Port::A, 23, GPIO::Periph::E}
    //{GPIO::Port::C, 30, GPIO::Periph::E}

    // Alternatives for GCLK_IN1
    //{GPIO::Port::A, 24, GPIO::Periph::E}
    //{GPIO::Port::C, 31, GPIO::Periph::E}

}

namespace SPI {

    GPIO::Pin PIN_MISO =  {GPIO::Port::C, 28, GPIO::Periph::B};
    GPIO::Pin PIN_MOSI =  {GPIO::Port::C, 29, GPIO::Periph::B};
    GPIO::Pin PIN_SCK =   {GPIO::Port::C, 30, GPIO::Periph::B};
    GPIO::Pin PIN_NPCS0 = {GPIO::Port::C, 31, GPIO::Periph::B};
    GPIO::Pin PIN_NPCS1 = {GPIO::Port::A, 13, GPIO::Periph::C};
    GPIO::Pin PIN_NPCS2 = {GPIO::Port::A, 14, GPIO::Periph::C};
    GPIO::Pin PIN_NPCS3 = {GPIO::Port::A, 15, GPIO::Periph::C};

    // Alternatives for MISO
    //GPIO::Pin PIN_MISO =  {GPIO::Port::A,  3, GPIO::Periph::B};
    //GPIO::Pin PIN_MISO =  {GPIO::Port::A, 21, GPIO::Periph::A};
    //GPIO::Pin PIN_MISO =  {GPIO::Port::A, 27, GPIO::Periph::A};
    //GPIO::Pin PIN_MISO =  {GPIO::Port::B, 14, GPIO::Periph::B};
    //GPIO::Pin PIN_MISO =  {GPIO::Port::C,  4, GPIO::Periph::A};

    // Alternatives for MOSI
    //GPIO::Pin PIN_MOSI =  {GPIO::Port::A, 22, GPIO::Periph::A};
    //GPIO::Pin PIN_MOSI =  {GPIO::Port::A, 28, GPIO::Periph::A};
    //GPIO::Pin PIN_MOSI =  {GPIO::Port::B, 15, GPIO::Periph::B};
    //GPIO::Pin PIN_MOSI =  {GPIO::Port::C,  5, GPIO::Periph::A};

    // Alternatives for SCK
    //GPIO::Pin PIN_SCK =   {GPIO::Port::A, 23, GPIO::Periph::A};
    //GPIO::Pin PIN_SCK =   {GPIO::Port::A, 29, GPIO::Periph::A};
    //GPIO::Pin PIN_SCK =   {GPIO::Port::C,  6, GPIO::Periph::A};

    // Alternatives for NPCS0
    //GPIO::Pin PIN_NPCS0 = {GPIO::Port::A,  2, GPIO::Periph::B};
    //GPIO::Pin PIN_NPCS0 = {GPIO::Port::A, 24, GPIO::Periph::A};
    //GPIO::Pin PIN_NPCS0 = {GPIO::Port::A, 30, GPIO::Periph::A};
    //GPIO::Pin PIN_NPCS0 = {GPIO::Port::C,  3, GPIO::Periph::A};

    // Alternatives for NPCS1
    //GPIO::Pin PIN_NPCS1 = {GPIO::Port::A, 31, GPIO::Periph::A};
    //GPIO::Pin PIN_NPCS1 = {GPIO::Port::B, 13, GPIO::Periph::B};
    //GPIO::Pin PIN_NPCS1 = {GPIO::Port::C,  2, GPIO::Periph::A};

    // Alternatives for NPCS2
    //GPIO::Pin PIN_NPCS2 = {GPIO::Port::B, 11, GPIO::Periph::B};
    //GPIO::Pin PIN_NPCS2 = {GPIO::Port::C,  0, GPIO::Periph::A};

    // Alternatives for NPCS3
    //GPIO::Pin PIN_NPCS3 = {GPIO::Port::B, 12, GPIO::Periph::B};
    //GPIO::Pin PIN_NPCS3 = {GPIO::Port::C,  1, GPIO::Periph::A};

}

namespace TC {

    const uint8_t N_TC = 2;

    GPIO::Pin PINS[MAX_N_TC][N_COUNTERS_PER_TC * N_CHANNELS_PER_COUNTER] = {
        {
            {GPIO::Port::B,  7, GPIO::Periph::D}, // TC0 A0
            {GPIO::Port::B,  8, GPIO::Periph::D}, // TC0 B0
            {GPIO::Port::B,  9, GPIO::Periph::D}, // TC0 A1
            {GPIO::Port::B, 10, GPIO::Periph::D}, // TC0 B1
            {GPIO::Port::B, 11, GPIO::Periph::D}, // TC0 A2
            {GPIO::Port::B, 12, GPIO::Periph::D}  // TC0 B2
        },
        {
            {GPIO::Port::C,  0, GPIO::Periph::D}, // TC1 A0
            {GPIO::Port::C,  1, GPIO::Periph::D}, // TC1 B0
            {GPIO::Port::C,  2, GPIO::Periph::D}, // TC1 A1
            {GPIO::Port::C,  3, GPIO::Periph::D}, // TC1 B1
            {GPIO::Port::C,  4, GPIO::Periph::D}, // TC1 A2
            {GPIO::Port::C,  5, GPIO::Periph::D}  // TC1 B2
        }
    };

    GPIO::Pin PINS_CLK[MAX_N_TC][N_EXTERNAL_CLOCKS_PER_TC] = {
        {
            {GPIO::Port::B, 13, GPIO::Periph::D}, // TC0 CLK0
            {GPIO::Port::B, 14, GPIO::Periph::D}, // TC0 CLK1
            {GPIO::Port::B, 15, GPIO::Periph::D}, // TC0 CLK2
        },
        {
            {GPIO::Port::C,  6, GPIO::Periph::D}, // TC1 CLK0
            {GPIO::Port::C,  7, GPIO::Periph::D}, // TC1 CLK1
            {GPIO::Port::C,  8, GPIO::Periph::D}  // TC1 CLK2
        }
    };

    // Alternatives
    //{GPIO::Port::A,  8, GPIO::Periph::B}, // TC0 A0
    //{GPIO::Port::A,  9, GPIO::Periph::B}, // TC0 B0
    //{GPIO::Port::A, 10, GPIO::Periph::B}, // TC0 A1
    //{GPIO::Port::A, 11, GPIO::Periph::B}, // TC0 B1
    //{GPIO::Port::A, 12, GPIO::Periph::B}, // TC0 A2
    //{GPIO::Port::A, 13, GPIO::Periph::B}, // TC0 B2
    //{GPIO::Port::A, 14, GPIO::Periph::B}, // TC0 CLK0
    //{GPIO::Port::A, 15, GPIO::Periph::B}, // TC0 CLK1
    //{GPIO::Port::A, 16, GPIO::Periph::B}, // TC0 CLK2
    //{GPIO::Port::C, 15, GPIO::Periph::A}, // TC1 A0
    //{GPIO::Port::C, 16, GPIO::Periph::A}, // TC1 B0
    //{GPIO::Port::C, 17, GPIO::Periph::A}, // TC1 A1
    //{GPIO::Port::C, 18, GPIO::Periph::A}, // TC1 B1
    //{GPIO::Port::C, 19, GPIO::Periph::A}, // TC1 A2
    //{GPIO::Port::C, 20, GPIO::Periph::A}, // TC1 B2
    //{GPIO::Port::C, 21, GPIO::Periph::A}, // TC1 CLK0
    //{GPIO::Port::C, 22, GPIO::Periph::A}, // TC1 CLK1
    //{GPIO::Port::C, 23, GPIO::Periph::A}, // TC1 CLK2

}

namespace USART {

    // RX
    GPIO::Pin PINS_RX[] = {
        {GPIO::Port::A, 11, GPIO::Periph::A}, // USART0 RX
        {GPIO::Port::C, 26, GPIO::Periph::A}, // USART1 RX
        {GPIO::Port::A, 19, GPIO::Periph::A}, // USART2 RX
        {GPIO::Port::C, 28, GPIO::Periph::A}  // USART3 RX
    };

    // TX
    GPIO::Pin PINS_TX[] = {
        {GPIO::Port::A, 12, GPIO::Periph::A}, // USART0 TX
        {GPIO::Port::C, 27, GPIO::Periph::A}, // USART1 TX
        {GPIO::Port::A, 20, GPIO::Periph::A}, // USART2 TX
        {GPIO::Port::C, 29, GPIO::Periph::A}  // USART3 TX
    };

    // RTS
    GPIO::Pin PINS_RTS[] = {
        {GPIO::Port::A,  8, GPIO::Periph::A}, // USART0 RTS
        {GPIO::Port::B,  2, GPIO::Periph::B}, // USART1 RTS
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

    //{GPIO::Port::B,  0, GPIO::Periph::B} // RX
    //{GPIO::Port::B,  1, GPIO::Periph::B} // TX

    //{GPIO::Port::B, 14, GPIO::Periph::A} // RX
    //{GPIO::Port::B, 15, GPIO::Periph::A} // TX
    //{GPIO::Port::B, 12, GPIO::Periph::A} // RTS
    //{GPIO::Port::B, 11, GPIO::Periph::A} // CTS

    //{GPIO::Port::C,  2, GPIO::Periph::C} // RX
    //{GPIO::Port::C,  3, GPIO::Periph::C} // TX
    //{GPIO::Port::C,  1, GPIO::Periph::B} // RTS
    //{GPIO::Port::C,  2, GPIO::Periph::B} // CTS


    // Alternatives for USART1

    //{GPIO::Port::A, 15, GPIO::Periph::A} // RX
    //{GPIO::Port::A, 16, GPIO::Periph::A} // TX
    //{GPIO::Port::A, 13, GPIO::Periph::A} // RTS

    //{GPIO::Port::B,  4, GPIO::Periph::B} // RX
    //{GPIO::Port::B,  5, GPIO::Periph::B} // TX

    //{GPIO::Port::C, 24, GPIO::Periph::A} // RTS


    // Alternatives for USART2

    //{GPIO::Port::A, 25, GPIO::Periph::B} // RX
    //{GPIO::Port::A, 26, GPIO::Periph::B} // TX

    //{GPIO::Port::C, 11, GPIO::Periph::B} // RX
    //{GPIO::Port::C, 12, GPIO::Periph::B} // TX

    //{GPIO::Port::C,  7, GPIO::Periph::B} // RTS
    //{GPIO::Port::C,  8, GPIO::Periph::E} // CTS


    // Alternatives for USART3

    //{GPIO::Port::A, 30, GPIO::Periph::E} // RX
    //{GPIO::Port::A, 31, GPIO::Periph::E} // TX

    //{GPIO::Port::C,  9, GPIO::Periph::B} // RX
    //{GPIO::Port::C, 10, GPIO::Periph::B} // TX
    //{GPIO::Port::C, 13, GPIO::Periph::B} // RTS

    //{GPIO::Port::B,  9, GPIO::Periph::A} // RX
    //{GPIO::Port::B, 10, GPIO::Periph::A} // TX
    //{GPIO::Port::B,  6, GPIO::Periph::A} // RTS
    //{GPIO::Port::B,  7, GPIO::Periph::A} // CTS

    //{GPIO::Port::C, 30, GPIO::Periph::A} // RTS

}

namespace USB {

    const GPIO::Pin PIN_DM = {GPIO::Port::A, 25, GPIO::Periph::A};
    const GPIO::Pin PIN_DP = {GPIO::Port::A, 26, GPIO::Periph::A};

}