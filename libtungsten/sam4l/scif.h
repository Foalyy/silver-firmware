#ifndef _SCIF_H_
#define _SCIF_H_

#include <stdint.h>
#include "gpio.h"

// System Control Interface
// This module manages most of the clock sources : external crystal 
// oscillator, DFLL/PLL, low-power default RCSYS oscillator, the faster
// RC80M and RCFAST oscillators, and the Generic Clocks system.
namespace SCIF {

    // Peripheral memory space base address
    const uint32_t SCIF_BASE = 0x400E0800;

    // Registers addresses
    const uint32_t OFFSET_IER =        0x0000; // Interrupt Enable Register
    const uint32_t OFFSET_IDR =        0x0004; // Interrupt Disable Register
    const uint32_t OFFSET_IMR =        0x0008; // Interrupt Mask Register
    const uint32_t OFFSET_ISR =        0x000C; // Interrupt Status Register
    const uint32_t OFFSET_ICR =        0x0010; // Interrupt Clear Register
    const uint32_t OFFSET_PCLKSR =     0x0014; // Power and Clocks Status Register
    const uint32_t OFFSET_UNLOCK =     0x0018; // Unlock Register
    const uint32_t OFFSET_CSCR =       0x001C; // Chip Specific Configuration Register
    const uint32_t OFFSET_OSCCTRL0 =   0x0020; // Oscillator Control Register
    const uint32_t OFFSET_PLL0 =       0x0024; // PLL0 Control Register
    const uint32_t OFFSET_DFLL0CONF =  0x0028; // DFLL0 Config Register
    const uint32_t OFFSET_DFLL0VAL =   0x002C; // DFLL Value Register
    const uint32_t OFFSET_DFLL0MUL =   0x0030; // DFLL0 Multiplier Register
    const uint32_t OFFSET_DFLL0STEP =  0x0034; // DFLL0 Step Register
    const uint32_t OFFSET_DFLL0SSG =   0x0038; // DFLL0 Spread Spectrum Generator Control Register
    const uint32_t OFFSET_DFLL0RATIO = 0x003C; // DFLL0 Ratio Register
    const uint32_t OFFSET_DFLL0SYNC =  0x0040; // DFLL0 Synchronization Register
    const uint32_t OFFSET_RCCR =       0x0044; // System RC Oscillator Calibration Register
    const uint32_t OFFSET_RCFASTCFG =  0x0048; // 4/8/12MHz RC Oscillator Configuration Register
    const uint32_t OFFSET_RCFASTSR =   0x004C; // 4/8/12MHz RC Oscillator Status Register
    const uint32_t OFFSET_RC80MCR =    0x0050; // 80MHz RC Oscillator Register
    const uint32_t OFFSET_HRPCR =      0x0064; // High Resolution Prescaler Control Register
    const uint32_t OFFSET_FPCR =       0x0068; // Fractional Prescaler Control Register
    const uint32_t OFFSET_FPMUL =      0x006C; // Fractional Prescaler Multiplier Register
    const uint32_t OFFSET_FPDIV =      0x0070; // Fractional Prescaler DIVIDER Register
    const uint32_t OFFSET_GCCTRL0 =    0x0074; // Generic Clock Control 0
    const uint32_t OFFSET_GCCTRL1 =    0x0078; // Generic Clock Control 1
    const uint32_t OFFSET_GCCTRL2 =    0x007C; // Generic Clock Control 2
    const uint32_t OFFSET_GCCTRL3 =    0x0080; // Generic Clock Control 3
    const uint32_t OFFSET_GCCTRL4 =    0x0084; // Generic Clock Control 4
    const uint32_t OFFSET_GCCTRL5 =    0x0088; // Generic Clock Control 5
    const uint32_t OFFSET_GCCTRL6 =    0x008C; // Generic Clock Control 6
    const uint32_t OFFSET_GCCTRL7 =    0x0090; // Generic Clock Control 7
    const uint32_t OFFSET_GCCTRL8 =    0x0094; // Generic Clock Control 8
    const uint32_t OFFSET_GCCTRL9 =    0x0098; // Generic Clock Control 9
    const uint32_t OFFSET_GCCTRL10 =   0x009C; // Generic Clock Control 10
    const uint32_t OFFSET_GCCTRL11 =   0x00A0; // Generic Clock Control 11


    // Subregisters
    const uint32_t PCLKSR_OSC0RDY = 0;
    const uint32_t PCLKSR_DFLL0RDY = 3;
    const uint32_t PCLKSR_PLL0LOCK = 6;
    const uint32_t OSCCTRL0_MODE = 0;
    const uint32_t OSCCTRL0_GAIN = 1;
    const uint32_t OSCCTRL0_AGC = 3;
    const uint32_t OSCCTRL0_STARTUP = 8;
    const uint32_t OSCCTRL0_OSCEN = 16;
    const uint32_t PLL0_EN = 0;
    const uint32_t PLL0_PLLOSC = 1;
    const uint32_t PLL0_PLLOPT = 3;
    const uint32_t PLL0_PLLDIV = 8;
    const uint32_t PLL0_PLLMUL = 16;
    const uint32_t PLL0_PLLCOUNT = 24;
    const uint32_t DFLL0CONF_EN = 0;
    const uint32_t DFLL0CONF_MODE = 1;
    const uint32_t DFLL0CONF_RANGE = 16;
    const uint32_t DFLL0STEP_FSTEP = 0;
    const uint32_t DFLL0STEP_CSTEP = 16;
    const uint32_t RCFASTCFG_EN = 0;
    const uint32_t RCFASTCFG_TUNEEN = 1;
    const uint32_t RCFASTCFG_JITMODE = 2;
    const uint32_t RCFASTCFG_NBPERIODS = 4;
    const uint32_t RCFASTCFG_FRANGE = 8;
    const uint32_t RCFASTCFG_LOCKMARGIN = 12;
    const uint32_t RC80MCR_EN = 0;
    const uint32_t GCCTRL_CEN = 0;
    const uint32_t GCCTRL_DIVEN = 1;
    const uint32_t GCCTRL_OSCSEL = 8;
    const uint32_t GCCTRL_DIV = 16;

    // Constants
    const uint32_t UNLOCK_KEY = 0xAA << 24;
    
    // Error codes
    const uint16_t ERR_PLL_OUT_OF_RANGE = 0x0001;
    const uint16_t ERR_DFLL_OUT_OF_RANGE = 0x0002;

    // RCFAST can be configured to operate in any of these frequencies
    enum class RCFASTFrequency {
        RCFAST_4MHZ = 0b00,
        RCFAST_8MHZ = 0b01,
        RCFAST_12MHZ = 0b10
    };

    // Each generic clock channel has one or two specific allocations
    enum class GCLKChannel {
        GCLK0_DFLL,         // Also GCLK0 pin
        GCLK1_DFLLDITHER,   // Also GCLK1 pin
        GCLK2_AST,          // Also GCLK2 pin
        GCLK3_CATB,         // Also GCLK3 pin
        GCLK4_AES,
        GCLK5_GLOC_TC0,
        GCLK6_ABDAC_IIS,
        GCLK7_USB,
        GCLK8_TC1_PEVC0,
        GCLK9_PLL0_PEVC1,
        GCLK10_ADC,
        GCLK11_MASTER
    };

    // Each generic clock can be mapped to any of these clock sources
    enum class GCLKSource {
        RCSYS = 0,
        OSC32K = 1,
        DFLL = 2,
        OSC0 = 3,
        RC80M = 4,
        RCFAST = 5,
        RC1M = 6,
        CLKCPU = 7,
        CLKHSB = 8,
        CLKPBA = 9,
        CLKPBB = 10,
        CLKPBC = 11,
        CLKPBD = 12,
        RC32K = 13,
        CLK1K = 15,
        PLL = 16,
        HRP = 17,
        FP = 18,
        GCLKIN0 = 19,
        GCLKIN1 = 20,
        GCLK11 = 21
    };

    enum class PinFunction {
        GCLK,
        GCLK_IN
    };


    // Module API

    // RCSYS (115kHz) is the default RC oscillator on which the chip operates after reset
    unsigned long getRCSYSFrequency();

    // RCFAST is a faster RC oscillator than RCSYS, which can operate at 4MHz, 8MHz or 12MHz
    void enableRCFAST(RCFASTFrequency frequency, bool enabled=true);
    unsigned long getRCFASTFrequency();

    // OSC0 is an external crystal oscillator, which can operate from 0.6MHz to 30MHz.
    // See datasheet §42.7.1 Oscillator 0 (OSC0) Characteristics for more details on
    // the required characteristics for this oscillator and its load capacitors.
    void enableOSC0(unsigned long frequency, bool enabled=true);
    unsigned long getOSC0Frequency();

    // PLL (Phase Locked Loop) is able to generate a high-frequency clock based on a
    // lower-frequency one. It is used by the USB module.
    void enablePLL(int mul, int div, GCLKSource referenceClock=GCLKSource::RCSYS, unsigned long referenceFrequency=115000UL, bool enabled=true);
    unsigned long getPLLFrequency();

    // DFLL (Digital Frequency Locked Loop) is similar to the PLL
    void enableDFLL(unsigned long frequency, GCLKSource referenceClock=GCLKSource::OSC32K, unsigned long referenceFrequency=32768, bool enabled=true);
    unsigned long getDFLLFrequency();

    // RC80M is the faster RC oscillator available, operating at 80MHz. It can power the
    // main clock if downscaled to at most 48MHz, or be used as a generic clock.
    void enableRC80M(bool enabled=true);
    unsigned long getRC810MFrequency();

    // Generic clocks
    void enableGenericClock(GCLKChannel channel, GCLKSource source, bool output=true, uint32_t divider=0, bool enabled=true);

    // Set the pins used for signal lines
    void setPin(PinFunction function, int channel, GPIO::Pin pin);

}


#endif