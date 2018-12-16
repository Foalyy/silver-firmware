#include "gloc.h"
#include "pm.h"

namespace GLOC {

    // Package-dependant, defined in pins_sam4l_XX.cpp
    extern struct GPIO::Pin PINS_IN[][4];
    extern struct GPIO::Pin PINS_OUT[];


    void enableLUT(LUT lut, bool in0, bool in1, bool in2, bool in3) {
        // Enable the clock
        PM::enablePeripheralClock(PM::CLK_GLOC);

        // Configure inputs
        (*(volatile uint32_t*)(GLOC_BASE + static_cast<int>(lut) * GLOC_REG_SIZE + OFFSET_CR0))
            = 0 << CR0_FILTEN
            | in0 << CR0_AEN
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

    void disableLUT(LUT lut) {
        // Free pins
        GPIO::disablePeripheral(PINS_IN[static_cast<int>(lut)][0]);
        GPIO::disablePeripheral(PINS_IN[static_cast<int>(lut)][1]);
        GPIO::disablePeripheral(PINS_IN[static_cast<int>(lut)][2]);
        GPIO::disablePeripheral(PINS_IN[static_cast<int>(lut)][3]);
        GPIO::disablePeripheral(PINS_OUT[static_cast<int>(lut)]);
    }

    void setTruth(LUT lut, uint16_t truth) {
        // Set the truth table
        (*(volatile uint32_t*)(GLOC_BASE + static_cast<int>(lut) * GLOC_REG_SIZE + OFFSET_TRUTH0))
            = truth << TRUTH0_TRUTH;
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