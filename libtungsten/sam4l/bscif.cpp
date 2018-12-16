#include "bscif.h"
#include "bpm.h"

namespace BSCIF {

    unsigned long _osc32kFrequency = 0;
    unsigned long _rc32kFrequency = 0;
    unsigned long _rc1mFrequency = 0;

    void enableOSC32K() {
        // Save the frequency for future use
        _osc32kFrequency = 32768;

        // Unlock the OSCCTRL32 register, which is locked by default as a safety mesure
        (*(volatile uint32_t*)(BSCIF_BASE + OFFSET_UNLOCK))
                = UNLOCK_KEY            // KEY : Magic word (see datasheet)
                | OFFSET_OSCCTRL32;     // ADDR : unlock OSCCTRL32

        // Configure OSC32
        (*(volatile uint32_t*)(BSCIF_BASE + OFFSET_OSCCTRL32))
                = 1 << OSCCTRL32_EN32K      // EN32K : enable 32kHz output
                | 1 << OSCCTRL32_EN1K       // EN1K : enable 1kHz output
                | 3 << OSCCTRL32_MODE       // MODE : crystal
                | 8 << OSCCTRL32_SELCURR    // SELCURR : current driven into the crystal
                | 0 << OSCCTRL32_STARTUP;   // STARTUP : oscillator startup time

        // Enable OSC32
        (*(volatile uint32_t*)(BSCIF_BASE + OFFSET_OSCCTRL32))
                |= 1 << OSCCTRL32_OSC32EN;  // OSC32EN : enable crystal

        // Wait for OSC32K to be ready
        while (!((*(volatile uint32_t*)(BSCIF_BASE + OFFSET_PCLKSR)) & (1 << PCLKSR_OSC32RDY)));

        // Select OSC32K as the 32KHz clock source
        BPM::set32KHzClockSource(BPM::CLK32KSource::OSC32K);
    }

    unsigned long getOSC32KFrequency() {
        return _osc32kFrequency;
    }

    void enableRC32K() {
        // Save the frequency for future use
        _rc32kFrequency = 32768;

        // Unlock the RC32KCR register, which is locked by default as a safety mesure
        (*(volatile uint32_t*)(BSCIF_BASE + OFFSET_UNLOCK))
                = UNLOCK_KEY            // KEY : Magic word (see datasheet)
                | OFFSET_RC32KCR;       // ADDR : unlock RC32KCR

        // Configure RC32K
        (*(volatile uint32_t*)(BSCIF_BASE + OFFSET_RC32KCR))
                = 1 << RC32KCR_TCEN     // TCEN : enable temperature compensation
                | 1 << RC32KCR_EN32K    // EN32K : enable 32kHz output
                | 1 << RC32KCR_EN1K     // EN1K : enable 1kHz output
                | 1 << RC32KCR_MODE     // MODE : closed loop mode
                | 0 << RC32KCR_REF;     // REF : closed loop mode reference : OSC32K

        // Enable RC32K
        (*(volatile uint32_t*)(BSCIF_BASE + OFFSET_RC32KCR))
                |= 1 << RC32KCR_EN;     // EN : enable generic clock source

        // Wait for RC32K to be ready
        while (!((*(volatile uint32_t*)(BSCIF_BASE + OFFSET_PCLKSR)) & ((1 << PCLKSR_RC32KRDY) | (1 << PCLKSR_RC32KLOCK))));

        // Select RC32K as the 32KHz clock source
        BPM::set32KHzClockSource(BPM::CLK32KSource::RC32K);
    }

    unsigned long getRC32KFrequency() {
        return _rc32kFrequency;
    }

    void enableRC1M() {
        // Save the frequency for future use
        _rc1mFrequency = 1000000;

        // Unlock the RC1MCR register, which is locked by default as a safety mesure
        (*(volatile uint32_t*)(BSCIF_BASE + OFFSET_UNLOCK))
                = UNLOCK_KEY         // KEY : Magic word (see datasheet)
                | OFFSET_RC1MCR;     // ADDR : unlock RC1MCR

        // Configure RC1M
        (*(volatile uint32_t*)(BSCIF_BASE + OFFSET_RC1MCR))
                = 1 << RC1MCR_CLKOEN;    // CLKOEN : enable oscillator

        // Wait for RC1M to be ready
        while (!((*(volatile uint32_t*)(BSCIF_BASE + OFFSET_PCLKSR)) & (1 << PCLKSR_RC1MRDY)));
    }

    unsigned long getRC1MFrequency() {
        return _rc1mFrequency;
    }

    // Store data in one of the four 32-bit backup registers
    // This data is kept even in BACKUP low-power mode, when all other
    // registers not in the Backup domain are lost
    void storeBackupData(int n, uint32_t data) {
        // Register offset
        if (n < 0 || n > 3) {
            return;
        }
        const uint32_t offset = OFFSET_BR + n * 4;

        // Unlock the BRn register, which is locked by default as a safety mesure
        (*(volatile uint32_t*)(BSCIF_BASE + OFFSET_UNLOCK))
                = UNLOCK_KEY         // KEY : Magic word (see datasheet)
                | offset;            // ADDR : unlock BRn

        // Store data
        (*(volatile uint32_t*)(BSCIF_BASE + offset)) = data;
    }

    // Read data from one of the four 32-bit backup registers
    uint32_t readBackupData(int n) {
        // Register offset
        if (n < 0 || n > 3) {
            return 0;
        }
        const uint32_t offset = OFFSET_BR + n * 4;

        return (*(volatile uint32_t*)(BSCIF_BASE + offset));
    }

}