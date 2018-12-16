#ifndef _ADC_H_
#define _ADC_H_

#include <stdint.h>
#include "gpio.h"
#include "error.h"

// Analog to Digital Converter
// This module is used to measure an analog voltage on a pin
namespace ADC {

    // Peripheral memory space base address
    const uint32_t ADC_BASE = 0x40038000;

    // Registers addresses
    const uint32_t OFFSET_CR =          0x000; // Control Register
    const uint32_t OFFSET_CFG =         0x004; // Configuration Register
    const uint32_t OFFSET_SR =          0x008; // Status Register
    const uint32_t OFFSET_SCR =         0x00C; // Status Clear Register
    const uint32_t OFFSET_SEQCFG =      0x014; // Sequencer Configuration Register
    const uint32_t OFFSET_CDMA =        0x018; // Configuration Direct Memory Access Register
    const uint32_t OFFSET_TIM =         0x01C; // Timing Configuration Register
    const uint32_t OFFSET_ITIMER =      0x020; // Internal Timer Register
    const uint32_t OFFSET_WCFG =        0x024; // Window Monitor Configuration Register
    const uint32_t OFFSET_WTH =         0x028; // Window Monitor Threshold Configuration Register
    const uint32_t OFFSET_LCV =         0x02C; // Sequencer Last Converted Value Register
    const uint32_t OFFSET_IER =         0x030; // Interrupt Enable Register
    const uint32_t OFFSET_IDR =         0x034; // Interrupt Disable Register
    const uint32_t OFFSET_IMR =         0x038; // Interrupt Mask Register
    const uint32_t OFFSET_CALIB =       0x03C; // Calibration Register

    // Registers fields
    const uint32_t CR_SWRST = 0;
    const uint32_t CR_TSTOP = 1;
    const uint32_t CR_TSTART = 2;
    const uint32_t CR_STRIG = 3;
    const uint32_t CR_REFBUFEN = 4;
    const uint32_t CR_REFBUFDIS = 5;
    const uint32_t CR_EN = 8;
    const uint32_t CR_DIS = 9;
    const uint32_t CR_BGREQEN = 10;
    const uint32_t CR_BGREQDIS = 11;
    const uint32_t CFG_REFSEL = 1;
    const uint32_t CFG_SPEED = 4;
    const uint32_t CFG_CLKSEL = 6;
    const uint32_t CFG_PRESCAL = 8;
    const uint32_t SR_SEOC = 0;
    const uint32_t SR_EN = 24;
    const uint32_t SR_TBUSY = 25;
    const uint32_t SR_SBUSY = 26;
    const uint32_t SR_CBUSY = 27;
    const uint32_t SR_REFBUF = 28;
    const uint32_t SEQCFG_HWLA = 0;
    const uint32_t SEQCFG_BIPOLAR = 2;
    const uint32_t SEQCFG_GAIN = 4;
    const uint32_t SEQCFG_GCOMP = 7;
    const uint32_t SEQCFG_TRGSEL = 8;
    const uint32_t SEQCFG_RES = 12;
    const uint32_t SEQCFG_INTERNAL = 14;
    const uint32_t SEQCFG_MUXPOS = 16;
    const uint32_t SEQCFG_MUXNEG = 20;
    const uint32_t SEQCFG_ZOOMRANGE = 28;


    using Channel = uint8_t;

    enum class AnalogReference {
        INTERNAL_1V = 0b000,
        VCC_0625 = 0b001, // VCC * 0.625
        EXTERNAL_REFERENCE = 0b010,
        DAC_VOUT = 0b011,
        VCC_OVER_2 = 0b100, // VCC / 2
    };

    enum class Gain {
        X1 = 0b000,
        X2 = 0b001,
        X4 = 0b010,
        X8 = 0b011,
        X16 = 0b100,
        X32 = 0b101,
        X64 = 0b110,
        X05 = 0b111, // divided by 2
    };


    // Module API
    void init(AnalogReference analogReference=AnalogReference::VCC_OVER_2, int vref=3300);
    void enable(Channel channel);
    void disable(Channel channel);
    uint16_t readRaw(Channel channel, Gain gain=Gain::X05, Channel relativeTo=0xFF);
    int read(Channel channel, Gain gain=Gain::X05, Channel relativeTo=0xFF);
    void setPin(Channel channel, GPIO::Pin pin);

}


#endif