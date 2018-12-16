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
            | 1000 << MR_CLKDIV;    // CLKDIV : clock divider for internal trigger

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

    void start(uint16_t* buffer, int n) {
        // Enable the DAC controller if it is not already
        if (!_enabled) {
            enable();
        }

        // Stop any previous operation
        DMA::stopChannel(_dmaChannel);

        // Start a new operation
        DMA::startChannel(_dmaChannel, (uint32_t)buffer, n);
    }

    void reload(uint16_t* buffer, int n) {
        // Reload the DMA
        DMA::reloadChannel(_dmaChannel, (uint32_t)buffer, n);
    }

    void stop() {
        // Stop the DMA
        DMA::stopChannel(_dmaChannel);
    }

    bool isFinished() {
        return DMA::isFinished(_dmaChannel);
    }

    bool isReloadEmpty() {
        return DMA::isReloadEmpty(_dmaChannel);
    }

    bool setFrequency(unsigned long frequency) {
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

    void setPin(GPIO::Pin pin) {
        PIN_VOUT = pin;
    }

}