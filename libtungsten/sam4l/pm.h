#ifndef _PM_H_
#define _PM_H_

#include <stdint.h>

// Power Manager
// This module controls the clock gating from clock sources to peripherals
// as well as the reset logic.
namespace PM {

    // Peripheral memory space base address
    const uint32_t BASE = 0x400E0000;


    // Registers addresses
    const uint32_t OFFSET_MCCTRL =     0x000; // Main Clock Control
    const uint32_t OFFSET_CPUSEL =     0x004; // CPU Clock Select
    const uint32_t OFFSET_PBASEL =     0x00C; // PBA Clock Select
    const uint32_t OFFSET_PBBSEL =     0x010; // PBB Clock Select
    const uint32_t OFFSET_PBCSEL =     0x014; // PBC Clock Select
    const uint32_t OFFSET_PBDSEL =     0x018; // PBD Clock Select
    const uint32_t OFFSET_CPUMASK =    0x020; // CPU Mask
    const uint32_t OFFSET_HSBMASK =    0x024; // HSB Mask
    const uint32_t OFFSET_PBAMASK =    0x028; // PBA Mask
    const uint32_t OFFSET_PBBMASK =    0x02C; // PBB Mask
    const uint32_t OFFSET_PBCMASK =    0x030; // PBC Mask
    const uint32_t OFFSET_PBDMASK =    0x034; // PBD Mask
    const uint32_t OFFSET_PBADIVMASK = 0x040; // PBA Divided Mask
    const uint32_t OFFSET_CFDCTRL =    0x054; // Clock Failure Detector Control
    const uint32_t OFFSET_UNLOCK =     0x058; // Unlock Register
    const uint32_t OFFSET_IER =        0x0C0; // Interrupt Enable Register
    const uint32_t OFFSET_IDR =        0x0C4; // Interrupt Disable Register
    const uint32_t OFFSET_IMR =        0x0C8; // Interrupt Mask Register
    const uint32_t OFFSET_ISR =        0x0CC; // Interrupt Status Register
    const uint32_t OFFSET_ICR =        0x0D0; // Interrupt Clear Register
    const uint32_t OFFSET_SR =         0x0D4; // Status Register
    const uint32_t OFFSET_PPCR =       0x160; // Peripheral Power Control Register
    const uint32_t OFFSET_RCAUSE =     0x180; // Reset Cause Register
    const uint32_t OFFSET_WCAUSE =     0x184; // Wake Cause Register
    const uint32_t OFFSET_AWEN =       0x188; // Asynchronous Wake Enable
    const uint32_t OFFSET_PROTCTRL =   0x18C; // Protection Control Register
    const uint32_t OFFSET_FASTSLEEP =  0x194; // Fast Sleep Register
    const uint32_t OFFSET_CONFIG =     0x3F8; // Configuration Register


    // Subregisters
    const uint32_t CPUSEL_CPUSEL = 0;
    const uint32_t CPUSEL_CPUDIV = 7;
    const uint32_t MCCTRL_MCSEL = 0;
    const uint32_t SR_CFD = 0;
    const uint32_t SR_CKRDY = 5;
    const uint32_t SR_WAKE = 8;


    // Constants
    const uint32_t UNLOCK_KEY = 0xAA << 24;
    const uint32_t MCCTRL_MCSEL_RCSYS = 0;
    const uint32_t MCCTRL_MCSEL_OSC0 = 1;
    const uint32_t MCCTRL_MCSEL_PLL = 2;
    const uint32_t MCCTRL_MCSEL_DFLL = 3;
    const uint32_t MCCTRL_MCSEL_RC80M = 4;
    const uint32_t MCCTRL_MCSEL_RCFAST = 5;
    const uint32_t MCCTRL_MCSEL_RC1M = 6;

    const uint32_t HSBMASK = 0;
    const uint32_t HSBMASK_PDCA = 0;
    const uint32_t HSBMASK_FLASHCALW = 1;
    const uint32_t HSBMASK_FLASHCALW_PICOCACHE = 2;
    const uint32_t HSBMASK_USBC = 3;
    const uint32_t HSBMASK_CRCCU = 4;
    const uint32_t HSBMASK_AESA = 9;
    const uint32_t PBAMASK = 32;
    const uint32_t PBAMASK_IISC = 0;
    const uint32_t PBAMASK_SPI = 1;
    const uint32_t PBAMASK_TC0 = 2;
    const uint32_t PBAMASK_TC1 = 3;
    const uint32_t PBAMASK_TWIM0 = 4;
    const uint32_t PBAMASK_TWIS0 = 5;
    const uint32_t PBAMASK_TWIM1 = 6;
    const uint32_t PBAMASK_TWIS1 = 7;
    const uint32_t PBAMASK_USART0 = 8;
    const uint32_t PBAMASK_USART1 = 9;
    const uint32_t PBAMASK_USART2 = 10;
    const uint32_t PBAMASK_USART3 = 11;
    const uint32_t PBAMASK_ADCIFE = 12;
    const uint32_t PBAMASK_DACC = 13;
    const uint32_t PBAMASK_ACIFC = 14;
    const uint32_t PBAMASK_GLOC = 15;
    const uint32_t PBAMASK_ABDACB = 16;
    const uint32_t PBAMASK_TRNG = 17;
    const uint32_t PBAMASK_PARC = 18;
    const uint32_t PBAMASK_CATB = 19;
    const uint32_t PBAMASK_TWIM2 = 21;
    const uint32_t PBAMASK_TWIM3 = 22;
    const uint32_t PBAMASK_LCDCA = 23;
    const uint32_t PBBMASK = 64;
    const uint32_t PBBMASK_HRAMC1 = 1;
    const uint32_t PBBMASK_HMATRIX = 2;
    const uint32_t PBBMASK_PDCA = 3;
    const uint32_t PBBMASK_CRCCU = 4;
    const uint32_t PBBMASK_USBC = 5;
    const uint32_t PBBMASK_PEVC = 6;

    // Default clock frequencies
    const unsigned long RCSYS_FREQUENCY = 115000UL;
    const unsigned long RC80M_FREQUENCY = 80000000UL;
    const unsigned long PBA_MAX_FREQUENCY = 8000000UL;

    // Peripheral clocks
    const int CLK_DMA = HSBMASK + HSBMASK_PDCA;
    const int CLK_USB_HSB = HSBMASK + HSBMASK_USBC; // The USB has 2 clocks
    const int CLK_CRC = HSBMASK + HSBMASK_CRCCU;
    const int CLK_AES = HSBMASK + HSBMASK_AESA;
    const int CLK_IIS = PBAMASK + PBAMASK_IISC;
    const int CLK_SPI = PBAMASK + PBAMASK_SPI;
    const int CLK_TC0 = PBAMASK + PBAMASK_TC0;
    const int CLK_TC1 = PBAMASK + PBAMASK_TC1;
    const int CLK_I2CM0 = PBAMASK + PBAMASK_TWIM0;
    const int CLK_I2CS0 = PBAMASK + PBAMASK_TWIS0;
    const int CLK_I2CM1 = PBAMASK + PBAMASK_TWIM1;
    const int CLK_I2CS1 = PBAMASK + PBAMASK_TWIS1;
    const int CLK_I2CM2 = PBAMASK + PBAMASK_TWIM2;
    const int CLK_I2CM3 = PBAMASK + PBAMASK_TWIM3;
    const int CLK_USART0 = PBAMASK + PBAMASK_USART0;
    const int CLK_USART1 = PBAMASK + PBAMASK_USART1;
    const int CLK_USART2 = PBAMASK + PBAMASK_USART2;
    const int CLK_USART3 = PBAMASK + PBAMASK_USART3;
    const int CLK_ADC = PBAMASK + PBAMASK_ADCIFE;
    const int CLK_DAC = PBAMASK + PBAMASK_DACC;
    const int CLK_GLOC = PBAMASK + PBAMASK_GLOC;
    const int CLK_TRNG = PBAMASK + PBAMASK_TRNG;
    const int CLK_USB = PBBMASK + PBBMASK_USBC;


    // The main clock is used by the CPU and the peripheral buses
    // and can be connected to any of these clock sources
    enum class MainClockSource {
        RCSYS = MCCTRL_MCSEL_RCSYS,
        OSC0 = MCCTRL_MCSEL_OSC0,
        PLL = MCCTRL_MCSEL_PLL,
        DFLL = MCCTRL_MCSEL_DFLL,
        RC80M = MCCTRL_MCSEL_RC80M,
        RCFAST = MCCTRL_MCSEL_RCFAST,
    };

    // Some peripherals can wake up the chip from sleep mode
    // See enableWakeUpSource() for more details
    enum class WakeUpSource {
        I2C_SLAVE_0 = 0,
        I2C_SLAVE_1 = 1,
        USBC = 2,
        PSOK = 3,
        BOD18 = 4,
        BOD33 = 5,
        PICOUART = 6,
        LCDA = 7
    };

    // The PM can be used to determine the cause of the last reset
    // See resetCause() for more details
    enum class ResetCause {
        UNKNOWN = -1,
        POR = 0,    // Power-on reset (core power supply)
        BOD = 1,    // Brown-out reset (core power supply)
        EXT = 2,    // External reset pin
        WDT = 3,    // Watchdog reset
        BKUP = 6,   // Backup reset
        SYSRESETREQ = 8, // Software reset
        POR33 = 10, // Power-on reset (IO 3.3V supply)
        BOD33 = 13  // Brown-out reset (IO 3.3V supply)
    };

    enum class WakeUpCause {
        UNKNOWN = -1,
        I2C_SLAVE_0 = 0,
        I2C_SLAVE_1 = 1,
        USB = 2,
        PSOK = 3,
        BOD18 = 4,
        BOD33 = 5,
        PICOUART = 6,
        LCD = 7,
        EIC = 16,
        AST = 17,
    };

    const int N_INTERRUPTS = 3;
    enum class Interrupt {
        CLOCK_FAILURE = 0,
        CLOCK_READY = 5,
        WAKE = 8,
    };

    // Clock frequency values used by inline functions below
    extern unsigned long _mainClockFrequency;
    extern unsigned long _cpuClockFrequency;


    // Module API

    // Clock management
    void setMainClockSource(MainClockSource clockSource, unsigned long cpudiv=0);
    inline unsigned long getMainClockFrequency() { return _mainClockFrequency; }
    inline unsigned long getCPUClockFrequency() { return _cpuClockFrequency; }
    unsigned long getModuleClockFrequency(uint8_t peripheral);
    void enablePeripheralClock(uint8_t peripheral, bool enabled=true);
    void disablePeripheralClock(uint8_t peripheral);
    void enablePBADivClock(uint8_t pow);

    // Wake-up and reset
    ResetCause resetCause();
    WakeUpCause wakeUpCause();
    void enableWakeUpSource(WakeUpSource src);
    void disableWakeUpSource(WakeUpSource src);
    void disableWakeUpSources();

    // Interrupts
    void enableInterrupt(void (*handler)(), Interrupt interrupt=Interrupt::WAKE);
    void disableInterrupt(Interrupt interrupt=Interrupt::WAKE);

}


#endif