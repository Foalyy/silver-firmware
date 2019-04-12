#include "dac.h"
#include "pm.h"
#include "dma.h"

namespace DAC {

    // Package-dependant, defined in pins_sam4l_XX.cpp
    extern struct GPIO::Pin PIN_VOUT;

    bool _enabled = false;
    int _dmaChannel = 0;

    // Enable the DAC controller
    void enable() {
        // Enable the clock
        PM::enablePeripheralClock(PM::CLK_DAC);

        // Set the pin in peripheral mode
        GPIO::enablePeripheral(PIN_VOUT);

        // CR (Control Register) : issue a software reset
        (*(volatile uint32_t*)(DAC_BASE + OFFSET_CR))
            = 1 << CR_SWRST;

        // WPMR (Write Protect Mode Register) : unlock the MR register
        (*(volatile uint32_t*)(DAC_BASE + OFFSET_WPMR))
            = WPMR_WPKEY
            | 0 << WPMR_WPEN;

        // MR (Mode Register) : setup and enable the DAC
        (*(volatile uint32_t*)(DAC_BASE + OFFSET_MR))
            = 0 << MR_TRGEN         // TRGEN : internal trigger
            | 1 << MR_DACEN         // DACEN : enable DAC
            | 0 << MR_WORD          // WORD : half-word transfer
            | 100 << MR_STARTUP     // STARTUP : startup time ~12µs
            | 0 << MR_CLKDIV;    // CLKDIV : clock divider for internal trigger

        // WPMR (Write Protect Mode Register) : lock the MR register
        (*(volatile uint32_t*)(DAC_BASE + OFFSET_WPMR))
            = WPMR_WPKEY
            | 1 << WPMR_WPEN;

        _dmaChannel = DMA::newChannel(DMA::Device::DAC, DMA::Size::HALFWORD);

        _enabled = true;
    }

    // Disable the DAC controller and free ressources
    void disable() {
        // WPMR (Write Protect Mode Register) : unlock the MR register
        (*(volatile uint32_t*)(DAC_BASE + OFFSET_WPMR))
            = WPMR_WPKEY
            | 0 << WPMR_WPEN;

        // MR (Mode Register) : disable the DAC
        (*(volatile uint32_t*)(DAC_BASE + OFFSET_MR)) = 0;

        // Free the pin
        GPIO::disablePeripheral(PIN_VOUT);

        // Disable the clock
        PM::disablePeripheralClock(PM::CLK_DAC);
    }

    void write(uint16_t value) {
        // Enable the DAC controller if it is not already
        if (!_enabled) {
            enable();
        }

        // Cut high values
        if (value > 1023) {
            value = 1023;
        }

        // CDR (Conversion Data Register) : write new value
        (*(volatile uint32_t*)(DAC_BASE + OFFSET_CDR)) = value;
    }

    void start(uint16_t* buffer, int n, bool repeat) {
        // Enable the DAC controller if it is not already
        if (!_enabled) {
            enable();
        }

        // Stop any previous operation
        DMA::stopChannel(_dmaChannel);

        if (!repeat) {
            // Disable the ring mode of the DMA channel
            DMA::disableRing(_dmaChannel);

            // Start a new operation
            DMA::startChannel(_dmaChannel, (uint32_t)buffer, n);

        } else {
            // Enable the ring mode of the DMA channel
            DMA::enableRing(_dmaChannel);

            // Start a new operation
            DMA::startChannel(_dmaChannel, (uint32_t)buffer, n);

            // Reload the DMA channel with the same buffer, which
            // will be repeated indefinitely
            DMA::reloadChannel(_dmaChannel, (uint32_t)buffer, n);
        }
    }

    void reload(uint16_t* buffer, int n) {
        // Enable the DAC controller if it is not already
        if (!_enabled) {
            enable();
        }

        // Reload the DMA
        DMA::reloadChannel(_dmaChannel, (uint32_t)buffer, n);
    }

    void stop() {
        // Enable the DAC controller if it is not already
        if (!_enabled) {
            enable();
        }
        
        // Stop the DMA
        DMA::stopChannel(_dmaChannel);
    }

    bool isFinished() {
        // Enable the DAC controller if it is not already
        if (!_enabled) {
            enable();
        }
        
        return DMA::isFinished(_dmaChannel);
    }

    bool isReloadEmpty() {
        // Enable the DAC controller if it is not already
        if (!_enabled) {
            enable();
        }
        
        return DMA::isReloadEmpty(_dmaChannel);
    }

    bool setFrequency(unsigned long frequency) {
        // Enable the DAC controller if it is not already
        if (!_enabled) {
            enable();
        }

        // Compute the clock divider for the internal trigger
        if (frequency == 0) {
            return false;
        }
        unsigned long clkdiv = PM::getModuleClockFrequency(PM::CLK_DAC) / frequency;
        if (clkdiv > 0xFFFF) {
            return false;
        }

        // WPMR (Write Protect Mode Register) : unlock the MR register
        (*(volatile uint32_t*)(DAC_BASE + OFFSET_WPMR))
            = WPMR_WPKEY
            | 0 << WPMR_WPEN;

        // MR (Mode Register) : update CLKDIV
        uint32_t mr = (*(volatile uint32_t*)(DAC_BASE + OFFSET_MR));
        (*(volatile uint32_t*)(DAC_BASE + OFFSET_MR))
            = (mr & 0x0000FFFF)
            | clkdiv << MR_CLKDIV;

        // WPMR (Write Protect Mode Register) : lock the MR register
        (*(volatile uint32_t*)(DAC_BASE + OFFSET_WPMR))
            = WPMR_WPKEY
            | 1 << WPMR_WPEN;

        return true;
    }

    void enableInterrupt(void (*handler)(), Interrupt interrupt) {
        // Enable the DAC controller if it is not already
        if (!_enabled) {
            enable();
        }
        
        // Enable the interrupt directly in the DMA
        DMA::enableInterrupt(_dmaChannel, handler, static_cast<DMA::Interrupt>(interrupt));
    }

    void disableInterrupt(Interrupt interrupt) {
        // Enable the DAC controller if it is not already
        if (!_enabled) {
            enable();
        }
        
        // Disable the interrupt in the DMA
        DMA::disableInterrupt(_dmaChannel, static_cast<DMA::Interrupt>(interrupt));
    }

    void setPin(GPIO::Pin pin) {
        PIN_VOUT = pin;
    }

}