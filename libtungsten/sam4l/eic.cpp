#include "eic.h"
#include "pm.h"
#include "bpm.h"
#include "core.h"
#include "gpio.h"
#include "error.h"
#include <string.h>

namespace EIC {

    // Package-dependant, defined in pins_sam4l_XX.cpp
    extern struct GPIO::Pin PINS[];

    // Handlers defined by the user
    extern uint8_t INTERRUPT_PRIORITY;
    bool _initialized = false;
    uint32_t _interruptHandlers[N_CHANNELS - 1]; // -1 because channel 0 is the NMI

    // Internal functions
    void init();
    void handlerWrapper();


    void setPin(Channel channel, GPIO::Pin pin) {
        PINS[channel] = pin;
    }

    void init() {
        memset(_interruptHandlers, 0, sizeof(_interruptHandlers));
    }

    void enableInterrupt(Channel channel, Mode mode, Polarity polarity, void (*handler)(int), bool filter) {
        // Init the module if necessary
        if (!_initialized) {
            init();
            _initialized = true;
        }

        // Check channel number
        if (channel >= N_CHANNELS) {
            Error::happened(Error::Module::EIC, ERR_UNKNOWN_CHANNEL, Error::Severity::WARNING);
            return;
        }

        if (mode == Mode::EDGE) {
            // Mode
            (*(volatile uint32_t*)(BASE + OFFSET_MODE)) &= ~(uint32_t)(1 << channel);

            // Polarity : rising or falling edge
            if (polarity == Polarity::LOW_FALLING) {
                (*(volatile uint32_t*)(BASE + OFFSET_EDGE)) &= ~(uint32_t)(1 << channel);

            } else if (polarity == Polarity::HIGH_RISING) {
                (*(volatile uint32_t*)(BASE + OFFSET_EDGE)) |= 1 << channel;
            }

        } else if (mode == Mode::LEVEL) {
            // Mode
            (*(volatile uint32_t*)(BASE + OFFSET_MODE)) |= 1 << channel;

            // Polarity : high or low level
            if (polarity == Polarity::LOW_FALLING) {
                (*(volatile uint32_t*)(BASE + OFFSET_LEVEL)) &= ~(uint32_t)(1 << channel);

            } else if (polarity == Polarity::HIGH_RISING) {
                (*(volatile uint32_t*)(BASE + OFFSET_LEVEL)) |= 1 << channel;
            }
        }

        // Filter
        if (filter) {
            (*(volatile uint32_t*)(BASE + OFFSET_FILTER)) |= 1 << channel;
        } else {
            (*(volatile uint32_t*)(BASE + OFFSET_FILTER)) &= ~(uint32_t)(1 << channel);
        }

        // IER (Interrupt Enable Register) : enable the requested interrupt
        (*(volatile uint32_t*)(BASE + OFFSET_IER)) = 1 << channel;

        // Set the handler and enable the module interrupt at the Core level, except for the NMI
        if (channel > 0) {
            _interruptHandlers[channel - 1] = (uint32_t)handler;
            Core::Interrupt interrupt = static_cast<Core::Interrupt>(static_cast<int>(Core::Interrupt::EIC1) + channel - 1);
            Core::setInterruptHandler(interrupt, handlerWrapper);
            Core::enableInterrupt(interrupt, INTERRUPT_PRIORITY);
        }

        // Input pin
        GPIO::enablePeripheral(PINS[channel]);

        // EN (Enable Register) : enable the requested channel
        (*(volatile uint32_t*)(BASE + OFFSET_EN)) = 1 << channel;
    }

    void enableAsyncInterrupt(Channel channel, Polarity polarity, void (*handler)(int)) {
        // Init the module if necessary
        if (!_initialized) {
            init();
            _initialized = true;
        }

        // Check channel number
        if (channel >= N_CHANNELS) {
            Error::happened(Error::Module::EIC, ERR_UNKNOWN_CHANNEL, Error::Severity::WARNING);
            return;
        }

        // Mode : only level in async mode
        (*(volatile uint32_t*)(BASE + OFFSET_MODE)) |= 1 << channel;

        // Polarity : high or low level
        if (polarity == Polarity::LOW_FALLING) {
            (*(volatile uint32_t*)(BASE + OFFSET_LEVEL)) &= ~(uint32_t)(1 << channel);

        } else if (polarity == Polarity::HIGH_RISING) {
            (*(volatile uint32_t*)(BASE + OFFSET_LEVEL)) |= 1 << channel;
        }

        // Filter disabled in async mode
        (*(volatile uint32_t*)(BASE + OFFSET_FILTER)) &= ~(uint32_t)(1 << channel);

        // IER (Interrupt Enable Register) : enable the requested interrupt
        (*(volatile uint32_t*)(BASE + OFFSET_IER)) = 1 << channel;

        // Set the handler and enable the module interrupt at the Core level, except for the NMI
        if (channel > 0) {
            _interruptHandlers[channel - 1] = (uint32_t)handler;
            Core::Interrupt interrupt = static_cast<Core::Interrupt>(static_cast<int>(Core::Interrupt::EIC1) + channel - 1);
            Core::setInterruptHandler(interrupt, handlerWrapper);
            Core::enableInterrupt(interrupt, INTERRUPT_PRIORITY);
        }

        // Wake up logic
        BPM::enableBackupWakeUpSource(BPM::BackupWakeUpSource::EIC);

        // Input pin
        GPIO::enablePeripheral(PINS[channel]);
        BPM::enableBackupPin(static_cast<int>(channel));

        // EN (Enable Register) : enable the requested channel
        (*(volatile uint32_t*)(BASE + OFFSET_EN)) = 1 << channel;
    }

    void disableInterrupt(Channel channel) {
        // Check channel number
        if (channel >= N_CHANNELS) {
            Error::happened(Error::Module::EIC, ERR_UNKNOWN_CHANNEL, Error::Severity::WARNING);
            return;
        }

        // IDR (Interrupt Disable Register) : disable the requested interrupt
        (*(volatile uint32_t*)(BASE + OFFSET_IDR)) = 1 << channel;

        // DIS (Disable Register) : disable the requested channel
        (*(volatile uint32_t*)(BASE + OFFSET_DIS)) = 1 << channel;

        // Free the input pin
        GPIO::disablePeripheral(PINS[channel]);
    }

    void clearInterrupt(Channel channel) {
        // Check channel number
        if (channel >= N_CHANNELS) {
            Error::happened(Error::Module::EIC, ERR_UNKNOWN_CHANNEL, Error::Severity::WARNING);
            return;
        }

        // ICR (Interrupt Clear Register) : clear the interrupt
        (*(volatile uint32_t*)(BASE + OFFSET_ICR)) = 1 << channel;
    }

    // This wrapper is called when an interrupt is triggered and is used to clear the interrupt
    // and call a user handler if defined
    void handlerWrapper() {
        // Channel that triggered the interrupt
        int channel = static_cast<int>(Core::currentInterrupt()) - static_cast<int>(Core::Interrupt::EIC1) + 1;

        // Call the user handler for this interrupt
        void (*handler)(int) = (void (*)(int))_interruptHandlers[channel - 1];
        if (handler != nullptr) {
            handler(channel);
        }
        
        // ICR (Interrupt Clear Register) : clear the interrupt that was triggered
        (*(volatile uint32_t*)(BASE + OFFSET_ICR)) = 1 << channel;
    }

}