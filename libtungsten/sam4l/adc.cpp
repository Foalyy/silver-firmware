#include "adc.h"
#include "pm.h"

namespace ADC {

    // Package-dependant, defined in pins_sam4l_XX.cpp
    extern struct GPIO::Pin PINS[];

    // Bitset representing enabled channels
    uint16_t _enabledChannels = 0x0000;

    // Keep track of the state of the module
    bool _initialized = false;

    // Analog reference selected by the user
    AnalogReference _analogReference = AnalogReference::INTERNAL_1V;

    // Reference voltage value in mV
    // For VCC_0625 and VCC_OVER_2, this is simply the Vcc voltage
    int _vref = 0;


    // Initialize the common ressources of the ADC controller
    void init(AnalogReference analogReference, int vref) {
        // Voltage reference
        if (analogReference == AnalogReference::INTERNAL_1V) {
            vref = 1000;
        }
        _analogReference = analogReference;
        _vref = vref;

        // Enable the clock
        PM::enablePeripheralClock(PM::CLK_ADC);

        // Find a prescaler setting that will bring the module clock frequency below
        // the maximum specified in the datasheet (42.9.4 Analog to Digital Converter
        // Characteristics - ADC clock frequency - Max : 1.5MHz).
        // A frequency too high may otherwise result in incorrect conversions.
        unsigned long frequency = PM::getModuleClockFrequency(PM::CLK_ADC);
        uint8_t prescal = 0;
        for (prescal = 0b000; prescal <= 0b111; prescal++) {
            if ((frequency >> (prescal + 2)) <= 1500000) { // 1.5MHz
                break;
            }
        }

        // CR (Control Register) : enable the ADC
        (*(volatile uint32_t*)(ADC_BASE + OFFSET_CR))
            = 1 << CR_EN        // EN : enable ADC
            | 1 << CR_REFBUFEN  // REFBUFEN : enable reference buffer
            | 1 << CR_BGREQEN;  // BGREQEN : enable bandgap voltage reference

        // CFG (Configuration Register) : set general settings
        (*(volatile uint32_t*)(ADC_BASE + OFFSET_CFG))
            = static_cast<int>(analogReference) << CFG_REFSEL   // REFSEL : voltage reference
            | 0b11 << CFG_SPEED                                 // SPEED : 75ksps
            | 1 << CFG_CLKSEL                                   // CLKSEL : use APB clock
            | prescal << CFG_PRESCAL;                           // PRESCAL : divide clock by 4

        // SR (Status Register) : wait for enabled status flag
        while (!((*(volatile uint32_t*)(ADC_BASE + OFFSET_SR)) & (1 << SR_EN)));

        // Update the module status
        _initialized = true;
    }

    void enable(Channel channel) {
        // Set the pin in peripheral mode
        GPIO::enablePeripheral(PINS[channel]);

        // Initialize and enable the ADC controller if necessary
        if (!_initialized) {
            init();
        }
        _enabledChannels |= 1 << channel;
    }

    void disable(Channel channel) {
        // Disable the peripheral mode on the pin
        GPIO::disablePeripheral(PINS[channel]);

        // Disable the ADC controller if necessary
        _enabledChannels &= ~(uint32_t)(1 << channel);
        if (_enabledChannels == 0x0000) {
            // CR (Control Register) : disable the ADC
            (*(volatile uint32_t*)(ADC_BASE + OFFSET_CR))
                = 1 << CR_DIS;      // DIS : disable ADC
            _initialized = false;
        }
    }

    // Read the current raw value measured by the ADC on the given channel
    uint16_t readRaw(Channel channel, Gain gain, Channel relativeTo) {
        // Enable the channels if they are not already
        if (!(_enabledChannels & 1 << channel)) {
            enable(channel);
        }
        if (relativeTo != 0xFF && !(_enabledChannels & 1 << relativeTo)) {
            enable(relativeTo);
        }

        // SEQCFG (Sequencer Configuration Register) : setup the conversion
        (*(volatile uint32_t*)(ADC_BASE + OFFSET_SEQCFG))
            = 0 << SEQCFG_HWLA                                                    // HWLA : Half Word Left Adjust disabled
            | (relativeTo != 0xFF) << SEQCFG_BIPOLAR                              // BIPOLAR : single-ended or bipolar mode
            | static_cast<int>(gain) << SEQCFG_GAIN                               // GAIN : user-selected gain
            | 1 << SEQCFG_GCOMP                                                   // GCOMP : gain error reduction enabled
            | 0b000 << SEQCFG_TRGSEL                                              // TRGSEL : software trigger
            | 0 << SEQCFG_RES                                                     // RES : 12-bit resolution
            | (relativeTo != 0xFF ? 0b00 : 0b10) << SEQCFG_INTERNAL               // INTERNAL : POS external, NEG internal or external
            | (channel & 0b1111) << SEQCFG_MUXPOS                                 // MUXPOS : selected channel
            | (relativeTo != 0xFF ? relativeTo & 0b111 : 0b111) << SEQCFG_MUXNEG  // MUXNEG : pad ground or neg channel
            | 0b000 << SEQCFG_ZOOMRANGE;                                          // ZOOMRANGE : default

        // CR (Control Register) : start conversion
        (*(volatile uint32_t*)(ADC_BASE + OFFSET_CR))
            = 1 << CR_STRIG;    // STRIG : Sequencer Trigger

        // SR (Status Register) : wait for Sequencer End Of Conversion status flag
        while (!((*(volatile uint32_t*)(ADC_BASE + OFFSET_SR)) & (1 << SR_SEOC)));

        // SCR (Status Clear Register) : clear Sequencer End Of Conversion status flag
        (*(volatile uint32_t*)(ADC_BASE + OFFSET_SCR)) = 1 << SR_SEOC;

        // LCV (Last Converted Value) : conversion result
        return (*(volatile uint32_t*)(ADC_BASE + OFFSET_LCV)) & 0xFFFF;
    }

    // Return the current value on the given channel in mV
    int read(Channel channel, Gain gain, Channel relativeTo) {
        int value = readRaw(channel, gain, relativeTo);

        // Compute reference
        int vref = _vref;
        if (_analogReference == AnalogReference::VCC_0625) {
            vref = (vref * 625) / 1000;
        } else if (_analogReference == AnalogReference::VCC_OVER_2) {
            vref = vref / 2;
        } else if (_analogReference == AnalogReference::INTERNAL_1V) {
            vref = 1000;
        }

        // Convert the result to mV
        // Single-ended : value = gain * voltage / ref * 4095 <=> voltage = value * ref / (gain * 4095)
        // Differential : value = 2047 + gain * voltage / ref * 2047 <=> voltage = (value - 2047) * ref / (gain * 2047)
        if (relativeTo != 0xFF) {
            value -= 2047;
        }
        int gainCoefficients[] = {1, 2, 4, 8, 16, 32, 64};
        if (gain == Gain::X05) {
            value *= 2;
        }
        value *= vref;
        if (relativeTo != 0xFF) {
            value /= 2047;
        } else {
            value /= 4095;
        }
        if (gain != Gain::X05) {
            value /= gainCoefficients[static_cast<int>(gain)];
        }

        return value;
    }

    void setPin(Channel channel, GPIO::Pin pin) {
        PINS[channel] = pin;
    }

}