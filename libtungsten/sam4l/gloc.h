#ifndef _GLOC_H_
#define _GLOC_H_

#include <stdint.h>
#include <scif.h>
#include "gpio.h"

// Glue Logic Controller
// This module manages truth tables to use some of the chip's pins
// as programmable logic gates
namespace GLOC {

    // Peripheral memory space base address
    const uint32_t GLOC_BASE = 0x40060000;
    const uint32_t GLOC_REG_SIZE = 0x08;
    const int N_GLOC = 2;

    // Registers addresses
    const uint32_t OFFSET_CR0 =    0x00; // Control Register 0
    const uint32_t OFFSET_TRUTH0 = 0x04; // Truth Table Register 0

    // Subregisters
    const uint32_t CR0_AEN = 0;
    const uint32_t CR0_FILTEN = 31;
    const uint32_t TRUTH0_TRUTH = 0;

    enum class LUT {
        LUT0 = 0,
        LUT1 = 1
    };

    enum class PinFunction {
        IN0,
        IN1,
        IN2,
        IN3,
        OUT
    };


    // Module functions
    void enable(LUT lut, bool in0, bool in1=false, bool in2=false, bool in3=false);
    void disable(LUT lut);
    void set(LUT lut, bool output, bool in0, bool in1=false, bool in2=false, bool in3=false);
    void setLUT(LUT lut, uint16_t truth);
    void enableFilter(LUT lut, SCIF::GCLKSource clock=SCIF::GCLKSource::RCSYS, uint16_t divider=10);
    void disableFilter(LUT lut);
    void setPin(LUT lut, PinFunction function, GPIO::Pin pin);

}


#endif