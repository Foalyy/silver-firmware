#include "dma.h"
#include "core.h"
#include "error.h"
#include "pm.h"

namespace DMA {

    // Current number of channels
    int _nChannels = 0;

    ChannelConfig _channels[N_CHANNELS_MAX];

    // Interrupt handlers
    extern uint8_t INTERRUPT_PRIORITY;
    uint32_t _interruptHandlers[N_CHANNELS_MAX][N_INTERRUPTS];
    const int _interruptBits[N_INTERRUPTS] = {ISR_RCZ, ISR_TRC, ISR_TERR};
    void interruptHandlerWrapper();

    int newChannel(Device device, Size size, uint32_t address, uint16_t length, bool ring) {
        // Check that there is an available channel
        if (_nChannels == N_CHANNELS_MAX) {
            Error::happened(Error::Module::DMA, ERR_NO_CHANNEL_AVAILABLE, Error::Severity::CRITICAL);
            return -1;
        }

        // Channel number : take the last channel
        int n = _nChannels;
        const uint32_t REG_BASE = BASE + n * CHANNEL_REG_SIZE;

        // Make sure the clock for the PDCA (Peripheral DMA Controller) is enabled
        PM::enablePeripheralClock(PM::CLK_DMA);

        // Set up the channel
        (*(volatile uint32_t*)(REG_BASE + OFFSET_PSR)) = static_cast<int>(device);                  // Peripheral select
        (*(volatile uint32_t*)(REG_BASE + OFFSET_MAR)) = address;                                   // Buffer memory address
        (*(volatile uint32_t*)(REG_BASE + OFFSET_TCR)) = length;                                    // Buffer length
        (*(volatile uint32_t*)(REG_BASE + OFFSET_MARR)) = 0;                                        // Buffer memory address (reload value)
        (*(volatile uint32_t*)(REG_BASE + OFFSET_TCRR)) = 0;                                        // Buffer length (reload value)
        (*(volatile uint32_t*)(REG_BASE + OFFSET_MR)) = (static_cast<int>(size) & 0b11) << MR_SIZE; // Buffer unit size (byte, half-word or word)
        _channels[n].started = false;
        _channels[n].interruptsEnabled = false;

        // Enable the ring buffer
        if (ring) {
            (*(volatile uint32_t*)(REG_BASE + OFFSET_MARR)) = address;
            (*(volatile uint32_t*)(REG_BASE + OFFSET_TCRR)) = length;
            (*(volatile uint32_t*)(REG_BASE + OFFSET_MR)) |= 1 << MR_RING;
        }

        // Enable transfer
        (*(volatile uint32_t*)(REG_BASE + OFFSET_CR)) = 1;

        _nChannels++;

        return n;
    }

    void enableInterrupt(int channel, void (*handler)(), Interrupt interrupt) {
        // Save the user handler
        _interruptHandlers[channel][static_cast<int>(interrupt)] = (uint32_t)handler;

        // IER (Interrupt Enable Register) : enable the requested interrupt
        (*(volatile uint32_t*)(BASE + channel * CHANNEL_REG_SIZE + OFFSET_IER))
                = 1 << _interruptBits[static_cast<int>(interrupt)];

        // Enable the interrupt in the NVIC
        Core::Interrupt interruptChannel = static_cast<Core::Interrupt>(static_cast<int>(Core::Interrupt::DMA0) + channel);
        Core::setInterruptHandler(interruptChannel, interruptHandlerWrapper);
        Core::enableInterrupt(interruptChannel, INTERRUPT_PRIORITY);
        _channels[channel].interruptsEnabled = true;
    }

    void disableInterrupt(int channel, Interrupt interrupt) {
        // IDR (Interrupt Disable Register) : disable the requested interrupt
        (*(volatile uint32_t*)(BASE + channel * CHANNEL_REG_SIZE + OFFSET_IDR))
                = 1 << _interruptBits[static_cast<int>(interrupt)];

        // If no interrupt is enabled anymore, disable the channel interrupt at the Core level
        if ((*(volatile uint32_t*)(BASE + channel * CHANNEL_REG_SIZE + OFFSET_IMR)) == 0) {
            Core::disableInterrupt(static_cast<Core::Interrupt>(static_cast<int>(Core::Interrupt::DMA0) + channel));
            _channels[channel].interruptsEnabled = false;
        }
    }

    void setupChannel(int channel, uint32_t address, uint16_t length) {
        // Check that this channel exists
        if (channel >= _nChannels) {
            Error::happened(Error::Module::DMA, ERR_CHANNEL_NOT_INITIALIZED, Error::Severity::CRITICAL);
            return;
        }
        
        const uint32_t REG_BASE = BASE + channel * CHANNEL_REG_SIZE;

        // Empty TCR and disable the transfer
        (*(volatile uint32_t*)(REG_BASE + OFFSET_TCR)) = 0;
        (*(volatile uint32_t*)(REG_BASE + OFFSET_CR)) = 1 << CR_TDIS; // Disable transfer

        // Configure this channel
        (*(volatile uint32_t*)(REG_BASE + OFFSET_MAR)) = address;   // Buffer memory address
        (*(volatile uint32_t*)(REG_BASE + OFFSET_TCR)) = length;    // Buffer length
    }

    void startChannel(int channel) {
        // Check that this channel exists
        if (channel >= _nChannels) {
            Error::happened(Error::Module::DMA, ERR_CHANNEL_NOT_INITIALIZED, Error::Severity::CRITICAL);
            return;
        }

        // Enable this channel
        _channels[channel].started = true;
        const uint32_t REG_BASE = BASE + channel * CHANNEL_REG_SIZE;
        (*(volatile uint32_t*)(REG_BASE + OFFSET_CR)) = 1 << CR_TEN;    // Enable transfer

        // Reenable interrupts if necessary
        if (_channels[channel].interruptsEnabled) {
            Core::Interrupt interruptChannel = static_cast<Core::Interrupt>(static_cast<int>(Core::Interrupt::DMA0) + channel);
            Core::enableInterrupt(interruptChannel, INTERRUPT_PRIORITY);
        }
    }

    void startChannel(int channel, uint32_t address, uint16_t length) {
        setupChannel(channel, address, length);
        startChannel(channel);
    }

    void reloadChannel(int channel, uint32_t address, uint16_t length) {
        // Check that this channel exists
        if (channel >= _nChannels) {
            Error::happened(Error::Module::DMA, ERR_CHANNEL_NOT_INITIALIZED, Error::Severity::CRITICAL);
            return;
        }

        // Reload this channel
        const uint32_t REG_BASE = BASE + channel * CHANNEL_REG_SIZE;
        (*(volatile uint32_t*)(REG_BASE + OFFSET_MARR)) = address;  // Buffer memory address
        (*(volatile uint32_t*)(REG_BASE + OFFSET_TCRR)) = length;   // Buffer length

        // Reenable interrupts if necessary
        if (_channels[channel].interruptsEnabled) {
            Core::Interrupt interruptChannel = static_cast<Core::Interrupt>(static_cast<int>(Core::Interrupt::DMA0) + channel);
            Core::enableInterrupt(interruptChannel, INTERRUPT_PRIORITY);
        }
    }

    void stopChannel(int channel) {
        // Check that this channel exists
        if (channel >= _nChannels) {
            Error::happened(Error::Module::DMA, ERR_CHANNEL_NOT_INITIALIZED, Error::Severity::CRITICAL);
            return;
        }
        
        const uint32_t REG_BASE = BASE + channel * CHANNEL_REG_SIZE;

        // Disable the channel interrupt line, it will be reenabled when the channel is started again
        if (_channels[channel].interruptsEnabled) {
            Core::Interrupt interruptChannel = static_cast<Core::Interrupt>(static_cast<int>(Core::Interrupt::DMA0) + channel);
            Core::disableInterrupt(interruptChannel);
        }

        // Disable transfer and empty TCR
        _channels[channel].started = false;
        (*(volatile uint32_t*)(REG_BASE + OFFSET_CR)) = 1 << CR_TDIS;
        (*(volatile uint32_t*)(REG_BASE + OFFSET_TCR)) = 0;
    }

    int getCounter(int channel) {
        // TCR : Transfer Counter Register
        return (*(volatile uint32_t*)(BASE + channel * CHANNEL_REG_SIZE + OFFSET_TCR));
    }

    bool isEnabled(int channel) {
        // SR : Status Register
        return (*(volatile uint32_t*)(BASE + channel * CHANNEL_REG_SIZE + OFFSET_SR)) & (1 << SR_TEN);
    }

    bool isFinished(int channel) {
        // TCR : Transfer Counter Register
        return (*(volatile uint32_t*)(BASE + channel * CHANNEL_REG_SIZE + OFFSET_TCR)) == 0;
    }

    bool isReloadEmpty(int channel) {
        // TCR : Transfer Counter Reload Register
        return (*(volatile uint32_t*)(BASE + channel * CHANNEL_REG_SIZE + OFFSET_TCRR)) == 0;
    }

    void enableRing(int channel) {
        // MR : set the RING bit to keep reloading the channel with the same buffer
        (*(volatile uint32_t*)(BASE + channel * CHANNEL_REG_SIZE + OFFSET_MR)) |= 1 << MR_RING;
    }

    void disableRing(int channel) {
        // MR : reset the RING bit
        (*(volatile uint32_t*)(BASE + channel * CHANNEL_REG_SIZE + OFFSET_MR)) &= ~(uint32_t)(1 << MR_RING);
    }


    void interruptHandlerWrapper() {
        // Get the channel number through the current interrupt number
        int channel = static_cast<int>(Core::currentInterrupt()) - static_cast<int>(Core::Interrupt::DMA0);
        const uint32_t REG_BASE = BASE + channel * CHANNEL_REG_SIZE;

        // Call the user handler of every interrupt that is enabled and pending
        for (int i = 0; i < N_INTERRUPTS; i++) {
            if ((*(volatile uint32_t*)(REG_BASE + OFFSET_IMR)) & (1 << _interruptBits[i]) // Interrupt is enabled
                    && (*(volatile uint32_t*)(REG_BASE + OFFSET_ISR)) & (1 << _interruptBits[i])) { // Interrupt is pending
                void (*handler)() = (void (*)())_interruptHandlers[channel][i];
                if (handler != nullptr) {
                    handler();
                }

                // These interrupts are level-sensitive, they are cleared when their sources are resolved (e.g. 
                // the registers are written with non-zero values)
            }
        }
    }
}