#ifndef _BSCIF_H_
#define _BSCIF_H_

#include <stdint.h>

// Backup System Control Interface
// This module manages low-power clocks, the voltage regulator and 
// some general-purpose backup data registers
namespace BSCIF {

    // Peripheral memory space base address
    const uint32_t BSCIF_BASE = 0x400F0400;

    // Registers addresses
    const uint32_t OFFSET_IER =             0x0000; // Interrupt Enable Register
    const uint32_t OFFSET_IDR =             0x0004; // Interrupt Disable Register
    const uint32_t OFFSET_IMR =             0x0008; // Interrupt Mask Register
    const uint32_t OFFSET_ISR =             0x000C; // Interrupt Status Register
    const uint32_t OFFSET_ICR =             0x0010; // Interrupt Clear Register
    const uint32_t OFFSET_PCLKSR =          0x0014; // Power and Clocks Status Register
    const uint32_t OFFSET_UNLOCK =          0x0018; // Unlock Register
    const uint32_t OFFSET_CSCR =            0x001C; // Chip Specific Configuration Register
    const uint32_t OFFSET_OSCCTRL32 =       0x0020; // Oscillator 32 Control Register
    const uint32_t OFFSET_RC32KCR =         0x0024; // 32kHz RC Oscillator Control Register
    const uint32_t OFFSET_RC32KTUNE =       0x0028; // 32kHz RC Oscillator Tuning Register
    const uint32_t OFFSET_BOD33CTRL =       0x002C; // BOD33 Control Register
    const uint32_t OFFSET_BOD33LEVEL =      0x0030; // BOD33 Level Register
    const uint32_t OFFSET_BOD33SAMPLING =   0x0034; // BOD33 Sampling Control Register
    const uint32_t OFFSET_BOD18CTRL =       0x0038; // BOD18 Control Register
    const uint32_t OFFSET_BOD18LEVEL =      0x003C; // BOD18 Level Register
    const uint32_t OFFSET_BOD18SAMPLING =   0x0040; // BOD18 Sampling Control Register
    const uint32_t OFFSET_VREGCR =          0x0044; // Voltage Regulator Configuration Register
    const uint32_t OFFSET_RC1MCR =          0x0048; // 1MHz RC Clock Configuration Register
    const uint32_t OFFSET_BGCTRL =          0x0060; // Bandgap Control Register
    const uint32_t OFFSET_BGSR =            0x0064; // Bandgap Status Register
    const uint32_t OFFSET_BR =              0x0078; // Backup register

    // Subregisters
    const uint32_t PCLKSR_OSC32RDY = 0;
    const uint32_t PCLKSR_RC32KRDY = 1;
    const uint32_t PCLKSR_RC32KLOCK = 2;
    const uint32_t PCLKSR_RC32KREFE = 3;
    const uint32_t PCLKSR_RC32SAT = 4;
    const uint32_t PCLKSR_BOD33DET = 5;
    const uint32_t PCLKSR_BOD18DET = 6;
    const uint32_t PCLKSR_BOD33SYNRDY = 7;
    const uint32_t PCLKSR_BOD18SYNRDY = 8;
    const uint32_t PCLKSR_SSWRDY = 9;
    const uint32_t PCLKSR_VREGOK = 10;
    const uint32_t PCLKSR_RC1MRDY = 11;
    const uint32_t PCLKSR_LPBGRDY = 12;
    const uint32_t OSCCTRL32_OSC32EN = 0;
    const uint32_t OSCCTRL32_EN32K = 2;
    const uint32_t OSCCTRL32_EN1K = 3;
    const uint32_t OSCCTRL32_MODE = 8;
    const uint32_t OSCCTRL32_SELCURR = 12;
    const uint32_t OSCCTRL32_STARTUP = 16;
    const uint32_t RC32KCR_EN = 0;
    const uint32_t RC32KCR_TCEN = 1;
    const uint32_t RC32KCR_EN32K = 2;
    const uint32_t RC32KCR_EN1K = 3;
    const uint32_t RC32KCR_MODE = 4;
    const uint32_t RC32KCR_REF = 5;
    const uint32_t RC32KCR_FCD = 7;
    const uint32_t RC1MCR_CLKOEN = 0;

    // Constants
    const uint32_t UNLOCK_KEY = 0xAA << 24;


    // Module API
    void enableOSC32K();
    unsigned long getOSC32KFrequency();
    void enableRC32K();
    unsigned long getRC32KFrequency();
    void enableRC1M();
    unsigned long getRC1MFrequency();
    void storeBackupData(int register, uint32_t data);
    uint32_t readBackupData(int register);

}


#endif