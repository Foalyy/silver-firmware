#include "gloc.h"
#include "pm.h"

namespace GLOC {

    // Package-dependant, defined in pins_sam4l_XX.cpp
    extern struct GPIO::Pin PINS_IN[][4];
    extern struct GPIO::Pin PINS_OUT[];

    bool _filterEnabled[2] = {false, false};


    void enable(LUT lut, bool in0, bool in1, bool in2, bool in3) {
        // Enable the clock
        PM::enablePeripheralClock(PM::CLK_GLOC);

        // Configure inputs
        (*(volatile uint32_t*)(GLOC_BASE + static_cast<int>(lut) * GLOC_REG_SIZE + OFFSET_CR0))
            = 0 << CR0_FILTEN       // FILTEN : disable glitch filter
            | in0 << CR0_AEN        // AEN : enable inputs
            | in1 << (CR0_AEN + 1)
            | in2 << (CR0_AEN + 2)
            | in3 << (CR0_AEN + 3);

        // Set pins in peripheral mode
        if (in0) {
            GPIO::enablePeripheral(PINS_IN[static_cast<int>(lut)][0]);
        }
        if (in1) {
            GPIO::enablePeripheral(PINS_IN[static_cast<int>(lut)][1]);
        }
        if (in2) {
            GPIO::enablePeripheral(PINS_IN[static_cast<int>(lut)][2]);
        }
        if (in3) {
            GPIO::enablePeripheral(PINS_IN[static_cast<int>(lut)][3]);
        }
        GPIO::enablePeripheral(PINS_OUT[static_cast<int>(lut)]);
    }

    void disable(LUT lut) {
        // Free pins
        GPIO::disablePeripheral(PINS_IN[static_cast<int>(lut)][0]);
        GPIO::disablePeripheral(PINS_IN[static_cast<int>(lut)][1]);
        GPIO::disablePeripheral(PINS_IN[static_cast<int>(lut)][2]);
        GPIO::disablePeripheral(PINS_IN[static_cast<int>(lut)][3]);
        GPIO::disablePeripheral(PINS_OUT[static_cast<int>(lut)]);
    }

    void set(LUT lut, bool output, bool in0, bool in1, bool in2, bool in3) {
        // Compute the offset corresponding to these inputs in the LUT
        int offset = in0 | (in1 << 1) | (in2 << 2) | (in3 << 3);

        // Update the truth table
        if (output) {
            (*(volatile uint32_t*)(GLOC_BASE + static_cast<int>(lut) * GLOC_REG_SIZE + OFFSET_TRUTH0))
                |= 1 << offset;
        } else {
            (*(volatile uint32_t*)(GLOC_BASE + static_cast<int>(lut) * GLOC_REG_SIZE + OFFSET_TRUTH0))
                &= ~(uint32_t)(1 << offset);
        }
    }

    void setLUT(LUT lut, uint16_t truth) {
        // Set the truth table
        (*(volatile uint32_t*)(GLOC_BASE + static_cast<int>(lut) * GLOC_REG_SIZE + OFFSET_TRUTH0))
            = truth << TRUTH0_TRUTH;
    }

    void enableFilter(LUT lut, SCIF::GCLKSource clock, uint16_t divider) {
        _filterEnabled[static_cast<int>(lut)] = true;

        // Enable the generic clock on which the filter is based
        SCIF::enableGenericClock(SCIF::GCLKChannel::GCLK5_GLOC_TC0, clock, false, divider);

        // Enable the filter for this LUT
        (*(volatile uint32_t*)(GLOC_BASE + static_cast<int>(lut) * GLOC_REG_SIZE + OFFSET_CR0))
            |= 1 << CR0_FILTEN;
    }

    void disableFilter(LUT lut) {
        _filterEnabled[static_cast<int>(lut)] = false;

        // Disable the generic clock on which the filter is based if it is not used anymore
        if (!_filterEnabled[0] && !_filterEnabled[1]) {
            SCIF::disableGenericClock(SCIF::GCLKChannel::GCLK5_GLOC_TC0);
        }

        // Disable the filter for this LUT
        (*(volatile uint32_t*)(GLOC_BASE + static_cast<int>(lut) * GLOC_REG_SIZE + OFFSET_CR0))
            &= ~(uint32_t)(1 << CR0_FILTEN);
    }

    void setPin(LUT lut, PinFunction function, GPIO::Pin pin) {
        switch (function) {
            case PinFunction::IN0:
                PINS_IN[static_cast<int>(lut)][0] = pin;
                break;

            case PinFunction::IN1:
                PINS_IN[static_cast<int>(lut)][1] = pin;
                break;

            case PinFunction::IN2:
                PINS_IN[static_cast<int>(lut)][2] = pin;
                break;

            case PinFunction::IN3:
                PINS_IN[static_cast<int>(lut)][3] = pin;
                break;

            case PinFunction::OUT:
                PINS_OUT[static_cast<int>(lut)] = pin;
                break;
        }
    }

}