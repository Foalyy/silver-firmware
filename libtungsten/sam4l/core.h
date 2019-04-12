#ifndef _CORE_H_
#define _CORE_H_

#include <stdint.h>
#include "ast.h"
#include "error.h"

// This module manages the core features of the Cortex microcontroller, 
// such as interrupts and the SysTick. Most of the registers are defined 
// in the ARMv7-M Architecture Reference Manual.
// Doc : https://libtungsten.io/reference/core
namespace Core {

    // System Control Space (SCS) Registers
    const uint32_t CPUID = 0xE000ED00;      // CPUID Base Register
    const uint32_t ICSR = 0xE000ED04;       // Interrupt Control and State Register
    const uint32_t VTOR = 0xE000ED08;       // Vector table offset
    const uint32_t AIRCR = 0xE000ED0C;      // Application Interrupt and Reset Control Register
    const uint32_t SCR = 0xE000ED10;        // System Control Register
    const uint32_t CCR = 0xE000ED14;        // Configuration and Control Register
    const uint32_t CFSR = 0xE000ED28;       // Configurable Fault Status Register
    const uint32_t HFSR = 0xE000ED2C;       // HardFault Status Register
    const uint32_t DFSR = 0xE000ED30;       // Debug Fault Status Register
    const uint32_t MMFAR = 0xE000ED30;      // MemManage Fault Address Register
    const uint32_t BFAR = 0xE000ED38;       // BusFault Address Register

    // SysTick Registers
    const uint32_t SYST_CSR = 0xE000E010;   // SysTick Control and Status Register
    const uint32_t SYST_RVR = 0xE000E014;   // SysTick Reload Value Register
    const uint32_t SYST_CVR = 0xE000E018;   // SysTick Current Value Register
    const uint32_t SYST_CALIB = 0xE000E01C; // SysTick Calibration value register

    // Nested Vectored Interrupt Controller (NVIC) Registers
    const int N_NVIC_IMPLEMENTED = 3;
    const uint32_t NVIC_ISER0 = 0xE000E100; // Interrupt Set-Enable Register 0
    const uint32_t NVIC_ICER0 = 0xE000E180; // Interrupt Clear-Enable Register 0
    const uint32_t NVIC_ISPR0 = 0xE000E200; // Interrupt Set-Pending Register 0
    const uint32_t NVIC_ICPR0 = 0xE000E280; // Interrupt Clear-Pending Register 0
    const uint32_t NVIC_IABR0 = 0xE000E300; // Interrupt Active Bit Register 0
    const uint32_t NVIC_IPR0 = 0xE000E400;  // Interrupt Priority Register 0

    // Peripheral Debug
    const uint32_t PDBG = 0xE0042000; // Peripheral Debug Register

    // ChipID and Serial number
    const uint32_t CHIPID_CIDR = 0x400E0740;
    const uint32_t CHIPID_EXID = 0x400E0744;
    const uint32_t SERIAL_NUMBER_ADDR = 0x0080020C;
    const uint32_t SERIAL_NUMBER_LENGTH = 15; // bytes


    // Subregisters
    const uint32_t SCR_SLEEPONEXIT = 1; // Enter sleep state when no interrupt is being handled
    const uint32_t SCR_SLEEPDEEP = 2; // Select between sleep and wait/retention/backup mode
    const uint32_t SYST_CSR_ENABLE = 0; // SysTick enabled
    const uint32_t SYST_CSR_TICKINT = 1; // SysTick interrupt enabled
    const uint32_t SYST_CSR_CLKSOURCE = 2; // SysTick clock source
    const uint32_t SYST_CSR_COUNTFLAG = 6; // SysTick timer has reached 0
    const uint32_t AIRCR_SYSRESETREQ = 2; // System reset request
    const uint32_t AIRCR_VECTKEY = 0x05FA << 16; // AIRCR access key
    const uint32_t PDBG_WDT = 0; // Freeze WDT when Core is halted in debug mode
    const uint32_t PDBG_AST = 1; // Freeze AST when Core is halted in debug mode
    const uint32_t PDBG_PEVC = 2; // Freeze PEVX when Core is halted in debug mode
    const uint32_t CHIPID_CIDR_NVPSIZ = 8; // Flash size
    const uint32_t CHIPID_CIDR_SRAMSIZ = 16; // RAM size
    const uint32_t CHIPID_CIDR_EXT = 31; // Extension flag
    const uint32_t CHIPID_EXID_PACKAGE = 24; // Package type

    // Constant values
    const uint8_t CHIPID_CIDR_NVPSIZ_128K = 7;
    const uint8_t CHIPID_CIDR_NVPSIZ_256K = 9;
    const uint8_t CHIPID_CIDR_NVPSIZ_512K = 10;
    const uint8_t CHIPID_CIDR_SRAMSIZ_32K = 10;
    const uint8_t CHIPID_CIDR_SRAMSIZ_64K = 11;
    const uint8_t CHIPID_EXID_PACKAGE_48 = 2;
    const uint8_t CHIPID_EXID_PACKAGE_64 = 3;
    const uint8_t CHIPID_EXID_PACKAGE_100 = 4;


    // Exceptions
    const int N_INTERNAL_EXCEPTIONS = 16;
    enum class Exception {
        NMI = 2,
        HARDFAULT = 3,
        MEMMANAGE = 4,
        BUSFAULT = 5,
        USAGEFAULT = 6,
        SVCALL = 11,
        DEBUGMONITOR = 12,
        PENDSV = 14,
        SYSTICK = 15,
    };

    // Interrupts
    const int N_EXTERNAL_INTERRUPTS = 80;
    enum class Interrupt {
        DMA0  = 1,
        DMA1  = 2,
        DMA2  = 3,
        DMA3  = 4,
        DMA4  = 5,
        DMA5  = 6,
        DMA6  = 7,
        DMA7  = 8,
        DMA8  = 9,
        DMA9  = 10,
        DMA10  = 11,
        DMA11  = 12,
        DMA12  = 13,
        DMA13  = 14,
        DMA14  = 15,
        DMA15  = 16,
        CRCCU  = 17,
        USBC  = 18,
        PEVC_TR  = 19,
        PEVC_OV  = 20,
        AESA  = 21,
        PM  = 22,
        SCIF  = 23,
        FREQM  = 24,
        GPIO0 = 25, // PORTA
        GPIO1 = 26,
        GPIO2 = 27,
        GPIO3 = 28,
        GPIO4 = 29, // PORTB
        GPIO5 = 30,
        GPIO6 = 31,
        GPIO7 = 32,
        GPIO8 = 33, // PORTC
        GPIO9 = 34,
        GPI010 = 35,
        GPIO11 = 36,
        BPM = 37,
        BSCIF = 38,
        AST_ALARM = 39,
        AST_PER = 40,
        AST_OVF = 41,
        AST_READY = 42,
        AST_CLKREADY = 43,
        WDT = 44,
        EIC1 = 45,
        EIC2 = 46,
        EIC3 = 47,
        EIC4 = 48,
        EIC5 = 49,
        EIC6 = 50,
        EIC7 = 51,
        EIC8 = 52,
        IISC = 53,
        SPI = 54,
        TC00 = 55,
        TC01 = 56,
        TC02 = 57,
        TC10 = 58,
        TC11 = 59,
        TC12 = 60,
        TWIM0 = 61,
        TWIS0 = 62,
        TWIM1 = 63,
        TWIS1 = 64,
        USART0 = 65,
        USART1 = 66,
        USART2 = 67,
        USART3 = 68,
        ADCIFE = 69,
        DACC = 70,
        ACIFC = 71,
        ABDACB = 72,
        TRNG = 73,
        PARC = 74,
        CATB = 75,
        TWIM2 = 77,
        TWIM3 = 78,
        LCDCA = 79,
    };

    enum class TimeUnit {
        SECONDS,
        MILLISECONDS,
    };

    // See the doc for more details about the different sleep modes
    enum class SleepMode {
        SLEEP0,
        SLEEP1,
        SLEEP2,
        SLEEP3,
        WAIT,
        RETENTION,
        BACKUP,
    };

    // Chip information
    enum class Package {
        PCK_48PIN,
        PCK_64PIN,
        PCK_100PIN,
        UNKNOWN
    };

    enum class RAMSize {
        RAM_32K,
        RAM_64K,
        UNKNOWN
    };

    enum class FlashSize {
        FLASH_128K,
        FLASH_256K,
        FLASH_512K,
        UNKNOWN
    };

    using Time = AST::Time;


    // General purpose functions
    void init();
    void reset();
    void resetToBootloader();
    void resetToBootloader(unsigned int delayMs);

    // Microcontroller information
    Package package();
    RAMSize ramSize();
    FlashSize flashSize();
    void serialNumber(uint8_t* sn);

    // Interrupts
    void setExceptionHandler(Exception exception, void (*handler)());
    void setInterruptHandler(Interrupt interrupt, void (*handler)());
    void enableInterrupt(Interrupt interrupt, uint8_t priority);
    void disableInterrupt(Interrupt interrupt);
    inline void enableInterrupts() { __asm__("CPSIE I"); } // Change Program State Interrupt Enable
    inline void disableInterrupts() { __asm__("CPSID I"); } // Change Program State Interrupt Disable
    void setInterruptPriority(Interrupt interrupt, uint8_t priority);
    void stashInterrupts();
    void applyStashedInterrupts();
    Interrupt currentInterrupt();

    // Time and power related functions
    inline Time time() { return AST::time(); }
    void sleep(unsigned long length, TimeUnit unit=TimeUnit::MILLISECONDS);
    void sleep(SleepMode mode=SleepMode::SLEEP0, unsigned long length=0, TimeUnit unit=TimeUnit::MILLISECONDS);
    void waitMicroseconds(unsigned long length);
    void enableSysTick();
    void disableSysTick();

    // Default exception handlers
    void handlerNMI();
    void handlerHardFault();
    void handlerMemManage();
    void handlerBusFault();
    void handlerUsageFault();
    void handlerSVCall();
    void handlerDebugMonitor();
    void handlerPendSV();
    void handlerSysTick();

}

#endif