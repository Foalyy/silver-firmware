#include "gpio.h"
#include "core.h"
#include "error.h"
#include <string.h>

namespace GPIO {

    extern uint8_t INTERRUPT_PRIORITY;
    uint32_t _interruptHandlers[N_GPIO_LINES];
    uint32_t _portsState[N_PORTS];

    // Internal functions
    void interruptHandlerWrapper();


    // Internal initialization function. This is called in Core::init() and doesn't have to
    // be called by the user.
    void init() {
        memset(_interruptHandlers, 0, sizeof(_interruptHandlers));
        memset(_portsState, 0, sizeof(_portsState));
    }

    void enableInput(const Pin& pin, Pulling pulling) {
        const uint32_t REG_BASE = GPIO_BASE + static_cast<uint8_t>(pin.port) * PORT_REG_SIZE;

        // ODER (Output Driver Enable Register) : set the pin as input
        ((volatile RSCT_REG*)(REG_BASE + OFFSET_ODER))->CLEAR = 1 << pin.number;

        // STER (Schmitt Trigger Enable Register) : enable the pin input Schmitt trigger (mandatory)
        ((volatile RSCT_REG*)(REG_BASE + OFFSET_STER))->SET = 1 << pin.number;

        // Pulling
        setPulling(pin, pulling);

        // GPER (GPIO Enable Register) : set the pin as driven by the GPIO controller
        ((volatile RSCT_REG*)(REG_BASE + OFFSET_GPER))->SET = 1 << pin.number;

        // Save the current state for polling functions
        PinState state = get(pin);
        if (state == HIGH) {
            _portsState[static_cast<uint8_t>(pin.port)] |= 1 << pin.number;
        } else {
            _portsState[static_cast<uint8_t>(pin.port)] &= ~(uint32_t)(1 << pin.number);
        }
    }

    void enableOutput(const Pin& pin, PinState value) {
        const uint32_t REG_BASE = GPIO_BASE + static_cast<uint8_t>(pin.port) * PORT_REG_SIZE;

        // OVR (Output Value Register) : set the pin output state
        if (value) {
            ((volatile RSCT_REG*)(REG_BASE + OFFSET_OVR))->SET = 1 << pin.number;
        } else {
            ((volatile RSCT_REG*)(REG_BASE + OFFSET_OVR))->CLEAR = 1 << pin.number;
        }

        // ODER (Output Driver Enable Register) : set the pin as output
        ((volatile RSCT_REG*)(REG_BASE + OFFSET_ODER))->SET = 1 << pin.number;

        // GPER (GPIO Enable Register) : set the pin as driven by the GPIO controller
        ((volatile RSCT_REG*)(REG_BASE + OFFSET_GPER))->SET = 1 << pin.number;
    }

    void enablePeripheral(const Pin& pin) {
        const uint32_t REG_BASE = GPIO_BASE + static_cast<uint8_t>(pin.port) * PORT_REG_SIZE;

        // Check if the pin is already used by a peripheral
        if (!(((volatile RSCT_REG*)(REG_BASE + OFFSET_GPER))->RW & (1 << pin.number))) {
            Error::happened(Error::Module::GPIO, ERR_PIN_ALREADY_IN_USE, Error::Severity::CRITICAL);
            return;
        }

        // PMR (Peripheral Mux Register) : set the pin peripheral function
        if (static_cast<uint8_t>(pin.function) & 0b001) {
            ((volatile RSCT_REG*)(REG_BASE + OFFSET_PMR0))->SET = 1 << pin.number;
        } else {
            ((volatile RSCT_REG*)(REG_BASE + OFFSET_PMR0))->CLEAR = 1 << pin.number;
        }
        if (static_cast<uint8_t>(pin.function) & 0b010) {
            ((volatile RSCT_REG*)(REG_BASE + OFFSET_PMR1))->SET = 1 << pin.number;
        } else {
            ((volatile RSCT_REG*)(REG_BASE + OFFSET_PMR1))->CLEAR = 1 << pin.number;
        }
        if (static_cast<uint8_t>(pin.function) & 0b100) {
            ((volatile RSCT_REG*)(REG_BASE + OFFSET_PMR2))->SET = 1 << pin.number;
        } else {
            ((volatile RSCT_REG*)(REG_BASE + OFFSET_PMR2))->CLEAR = 1 << pin.number;
        }

        // GPER (GPIO Enable Register) : set the pin as driven by the peripheral function
        ((volatile RSCT_REG*)(REG_BASE + OFFSET_GPER))->CLEAR = 1 << pin.number;
    }

    void disablePeripheral(const Pin& pin) {
        const uint32_t REG_BASE = GPIO_BASE + static_cast<uint8_t>(pin.port) * PORT_REG_SIZE;

        // GPER (GPIO Enable Register) : set the pin as driven by the GPIO controller
        ((volatile RSCT_REG*)(REG_BASE + OFFSET_GPER))->SET = 1 << pin.number;
    }

    void setPulling(const Pin& pin, Pulling pulling) {
        const uint32_t REG_BASE = GPIO_BASE + static_cast<uint8_t>(pin.port) * PORT_REG_SIZE;

        if (pulling == Pulling::PULLUP) {
            // PDER (Pull-Down Enable Register) : disable the pin pull-down resistor
            ((volatile RSCT_REG*)(REG_BASE + OFFSET_PDER))->CLEAR = 1 << pin.number;

            // PUER (Pull-Up Enable Register) : enable the pin pull-up resistor
            ((volatile RSCT_REG*)(REG_BASE + OFFSET_PUER))->SET = 1 << pin.number;

        } else if (pulling == Pulling::PULLDOWN) {
            // PUER (Pull-Up Enable Register) : disable the pin pull-up resistor
            ((volatile RSCT_REG*)(REG_BASE + OFFSET_PUER))->CLEAR = 1 << pin.number;

            // PDER (Pull-Down Enable Register) : enable the pin pull-down resistor
            ((volatile RSCT_REG*)(REG_BASE + OFFSET_PDER))->SET = 1 << pin.number;

        } else if (pulling == Pulling::BUSKEEPER) {
            // See datasheet 23.7.10 Pull-down Enable Register
            // PUER (Pull-Up Enable Register) : enable the pin pull-up resistor
            ((volatile RSCT_REG*)(REG_BASE + OFFSET_PUER))->SET = 1 << pin.number;

            // PDER (Pull-Down Enable Register) : enable the pin pull-down resistor
            ((volatile RSCT_REG*)(REG_BASE + OFFSET_PDER))->SET = 1 << pin.number;

        } else { // Pulling::NONE
            // PUER (Pull-Up Enable Register) : disable the pin pull-up resistor
            ((volatile RSCT_REG*)(REG_BASE + OFFSET_PUER))->CLEAR = 1 << pin.number;

            // PDER (Pull-Down Enable Register) : disable the pin pull-down resistor
            ((volatile RSCT_REG*)(REG_BASE + OFFSET_PDER))->CLEAR = 1 << pin.number;
        }
    }

    void enableInterrupt(const Pin& pin, Trigger trigger) {
        const uint32_t REG_BASE = GPIO_BASE + static_cast<uint8_t>(pin.port) * PORT_REG_SIZE;

        // Select the trigger type : CHANGE = 00, RISING = 01, FALLING = 10
        if (trigger == Trigger::RISING) {
            ((volatile RSCT_REG*)(REG_BASE + OFFSET_IMR0))->SET = 1 << pin.number;
        } else {
            ((volatile RSCT_REG*)(REG_BASE + OFFSET_IMR0))->CLEAR = 1 << pin.number;
        }
        if (trigger == Trigger::FALLING) {
            ((volatile RSCT_REG*)(REG_BASE + OFFSET_IMR1))->SET = 1 << pin.number;
        } else {
            ((volatile RSCT_REG*)(REG_BASE + OFFSET_IMR1))->CLEAR = 1 << pin.number;
        }

        // Enable the interrupt at the GPIO Controller level
        ((volatile RSCT_REG*)(REG_BASE + OFFSET_IER))->SET = 1 << pin.number;

        // Enable the interrupt at the Core level
        Core::enableInterrupt(static_cast<Core::Interrupt>(
                static_cast<int>(Core::Interrupt::GPIO0) 
                + static_cast<uint8_t>(pin.port) * 4 
                + pin.number / 8), INTERRUPT_PRIORITY);
    }

    void enableInterrupt(const Pin& pin, void (*handler)(), Trigger trigger) {
        // Set the interrupt handler
        _interruptHandlers[static_cast<uint8_t>(pin.port) * 32 + pin.number] = (uint32_t) handler;
        Core::setInterruptHandler(static_cast<Core::Interrupt>(
                static_cast<int>(Core::Interrupt::GPIO0)
                + static_cast<uint8_t>(pin.port) * 4 
                + pin.number / 8), &interruptHandlerWrapper);

        // Enable the interrupt with the function above
        enableInterrupt(pin, trigger);
    }

    void disableInterrupt(const Pin& pin) {
        const uint32_t REG_BASE = GPIO_BASE + static_cast<uint8_t>(pin.port) * PORT_REG_SIZE;

        // Disable the interrupt at the GPIO Controller level
        ((volatile RSCT_REG*)(REG_BASE + OFFSET_IER))->CLEAR = 1 << pin.number;

        // Disable the interrupt at the Core level
        Core::disableInterrupt(static_cast<Core::Interrupt>(
                static_cast<int>(Core::Interrupt::GPIO0)
                + static_cast<uint8_t>(pin.port) * 4
                + pin.number / 8));
    }

    PinState get(const Pin& pin) {
        // PVR (Pin Value Register) : get the pin state
        return ((volatile RSCT_REG*)(GPIO_BASE + static_cast<uint8_t>(pin.port) * PORT_REG_SIZE + OFFSET_PVR))->RW & (1 << pin.number);
    }

    void set(const Pin& pin, PinState value) {
        const uint32_t REG_BASE = GPIO_BASE + static_cast<uint8_t>(pin.port) * PORT_REG_SIZE;

        // OVR (Output Value Register) : set the pin output state
        if (value) {
            ((volatile RSCT_REG*)(REG_BASE + OFFSET_OVR))->SET = 1 << pin.number;
        } else {
            ((volatile RSCT_REG*)(REG_BASE + OFFSET_OVR))->CLEAR = 1 << pin.number;
        }
    }

    void blip(const Pin& pin) {
        // Quickly turn the output pin high then low, to produce a blip
        // This can be useful for debug purposes, especially with a logic analyzer
        setHigh(pin);
        setLow(pin);
    }

    bool risingEdge(const Pin& pin) {
        bool result = false;

        // Check the current pin state
        PinState state = get(pin);
        if (state == HIGH) {
            // If it's HIGH, check the previous state
            if ((_portsState[static_cast<uint8_t>(pin.port)] & (1 << pin.number)) == 0) {
                // The previous state was LOW, therefore this is a rising edge
                result = true;
            }

            // Save the current state
            _portsState[static_cast<uint8_t>(pin.port)] |= 1 << pin.number;

        } else {
            // Save the current state
            _portsState[static_cast<uint8_t>(pin.port)] &= ~(uint32_t)(1 << pin.number);
        }

        return result;
    }

    bool fallingEdge(const Pin& pin) {
        bool result = false;

        // Check the current pin state
        PinState state = get(pin);
        if (state == LOW) {
            // If it's LOW, check the previous state
            if ((_portsState[static_cast<uint8_t>(pin.port)] & (1 << pin.number)) != 0) {
                // The previous state was HIGH, therefore this is a falling edge
                result = true;
            }

            // Save the current state
            _portsState[static_cast<uint8_t>(pin.port)] &= ~(uint32_t)(1 << pin.number);

        } else {
            // Save the current state
            _portsState[static_cast<uint8_t>(pin.port)] |= 1 << pin.number;
        }
        
        return result;
    }

    bool changed(const Pin& pin) {
        bool result = false;

        // Check the current pin state
        PinState state = get(pin);

        // Get the old state 
        PinState oldState = _portsState[static_cast<uint8_t>(pin.port)] & (1 << pin.number);
        if (oldState != state) {
            result = true;
        }

        // Save the current state
        if (state == HIGH) {
            _portsState[static_cast<uint8_t>(pin.port)] |= 1 << pin.number;
        } else {
            _portsState[static_cast<uint8_t>(pin.port)] &= ~(uint32_t)(1 << pin.number);
        }
        
        return result;
    }

    // This wrapper is called when any pin generates an interrupt, and calls
    // the user handler according to the current configuration in _interruptHandlers[]
    void interruptHandlerWrapper() {
        // Get the port and pin numbers which called this interrupt
        int channel = static_cast<int>(Core::currentInterrupt()) - static_cast<int>(Core::Interrupt::GPIO0);
        int port = channel / 4; // There are four 8-pin channels in each port. The division int truncate is voluntary.
        int subport = channel - 4 * port; // Equivalent to subport = channel % 4
        const uint32_t REG_BASE = GPIO_BASE + port * PORT_REG_SIZE;

        // For each of the 8 pins in this subport, call the handler if the interrupt is enabled (in IER) and pending (in IFR)
        uint32_t flag = ((volatile RSCT_REG*)(REG_BASE + OFFSET_IFR))->RW & ((volatile RSCT_REG*)(REG_BASE + OFFSET_IER))->RW;
        for (int pin = 0; pin < 8; pin++) {
            if (flag & (1 << (subport * 8 + pin))) {
                // Call the user handler for this interrupt
                void (*handler)() = (void (*)())_interruptHandlers[port * 32 + subport * 8 + pin];
                if (handler == nullptr) {
                    Error::happened(Error::Module::GPIO, ERR_HANDLER_NOT_DEFINED, Error::Severity::CRITICAL);
                } else {
                    handler();
                }

                // Clear the interrupt source
                ((volatile RSCT_REG*)(REG_BASE + OFFSET_IFR))->CLEAR = 1 << (subport * 8 + pin);
            }
        }
    }
}