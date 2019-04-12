#include "core.h"
#include "bscif.h"
#include "pm.h"
#include "bpm.h"
#include "gpio.h"
#include "flash.h"
#include "ast.h"
#include "wdt.h"
#include "error.h"
#include <string.h>

namespace Core {

    // The ISR vector stores the pointers to the functions called when a fault or an interrupt is triggered.
    // It must be aligned to the next power of 2 following the size of the vector table size. The SAM4L has 80
    // external interrupts + the 16 standard Cortex M4 exceptions, so the table length is 96 words, or 384 bytes.
    // The table must therefore must be aligned to 512 bytes boundaries.
    // See ARMv7-M Architecture Reference Manual, B3.2.5 "Vector Table Offset Register, VTOR", page 657,
    // and http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0553a/Ciheijba.html.
    uint32_t _isrVector[N_INTERNAL_EXCEPTIONS + N_EXTERNAL_INTERRUPTS] __attribute__ ((aligned (512)));

    // This array is used to store the NVIC ISERs (Interrupt Set-Enable Registers) when interrupts are stashed
    uint32_t _nvicStash[N_NVIC_IMPLEMENTED];


    // Initialize the core components : exceptions/interrupts table, 
    // clocks, SysTick (system timer), ...
    void init() {
        // Initialize the ISR vector
        memset(_isrVector, 0, sizeof(_isrVector));

        // Change the default handlers
        setExceptionHandler(Exception::NMI, handlerNMI);
        setExceptionHandler(Exception::HARDFAULT, handlerHardFault);
        setExceptionHandler(Exception::MEMMANAGE, handlerMemManage);
        setExceptionHandler(Exception::BUSFAULT, handlerBusFault);
        setExceptionHandler(Exception::USAGEFAULT, handlerUsageFault);
        setExceptionHandler(Exception::SVCALL, handlerSVCall);
        setExceptionHandler(Exception::DEBUGMONITOR, handlerDebugMonitor);
        setExceptionHandler(Exception::PENDSV, handlerPendSV);
        setExceptionHandler(Exception::SYSTICK, handlerSysTick);

        // Change the core vector pointer to the new table
        (*(volatile uint32_t*) VTOR) = (uint32_t) _isrVector;

        // Configure the Peripheral Debug register
        (*(volatile uint32_t*) PDBG)
            = 1 << PDBG_WDT     // Freeze WDT when Core is halted in debug mode
            | 0 << PDBG_AST     // Don't freeze AST when Core is halted in debug mode
            | 0 << PDBG_PEVC;   // Don't freeze PEVC when Core is halted in debug mode

        // Init the error reporting system
        Error::init();

        // Init the GPIO module
        GPIO::init();

        // Enable the default clocks
        BSCIF::enableOSC32K();

        // Init the AST timer which is used as a low-speed time reference (>10ms)
        // This is required by time() and sleep()
        AST::init();

        // Init the SysTick which is used as a high-speed time reference (based on CPU clock)
        // This is required by waitMicroseconds()
        enableSysTick();

        // Clear the watchdog interrupt in case this was the cause of reset
        WDT::clearInterrupt();

        // Init the stashed interrupts array
        memset(_nvicStash, 0, N_NVIC_IMPLEMENTED * sizeof(uint32_t));
    }

    // Reset the chip
    // PM::resetCause will be SYSRESETREQ
    void reset() {
        // AIRCR (Application Interrupt and Reset Control Register)
        uint32_t aircr = (*(volatile uint32_t*) AIRCR);
        (*(volatile uint32_t*) AIRCR)
            = (aircr & 0xFFFF)          // Keep the previous register value without the key
            | 1 << AIRCR_SYSRESETREQ    // SYSRESETREQ : request reset
            | AIRCR_VECTKEY;            // Write protection key
    }

    // Reset the chip in bootloader mode
    void resetToBootloader() {
        Flash::writeFuse(Flash::FUSE_BOOTLOADER_FORCE, true);
        reset();
    }

    // Reset the chip in bootloader mode after a delay using the watchdog timer
    void resetToBootloader(unsigned int delayMs) {
        Flash::writeFuse(Flash::FUSE_BOOTLOADER_FORCE, true);
        WDT::enable(delayMs, WDT::Unit::MILLISECONDS);
    }

    // Get the chip's package type
    Package package() {
        bool ext = ((*(volatile uint32_t*) CHIPID_CIDR) >> CHIPID_CIDR_EXT) & 1;
        if (ext) {
            uint8_t pck = ((*(volatile uint32_t*) CHIPID_EXID) >> CHIPID_EXID_PACKAGE) & 0b111;
            if (pck == CHIPID_EXID_PACKAGE_48) {
                return Package::PCK_48PIN;
            } else if (pck == CHIPID_EXID_PACKAGE_64) {
                return Package::PCK_64PIN;
            } else if (pck == CHIPID_EXID_PACKAGE_100) {
                return Package::PCK_100PIN;
            }
        }
        return Package::UNKNOWN;
    }

    // Get the chip's RAM size
    RAMSize ramSize() {
        uint8_t sramsiz = ((*(volatile uint32_t*) CHIPID_CIDR) >> CHIPID_CIDR_SRAMSIZ) & 0b1111;
        if (sramsiz == CHIPID_CIDR_SRAMSIZ_32K) {
            return RAMSize::RAM_32K;
        } else if (sramsiz == CHIPID_CIDR_SRAMSIZ_64K) {
            return RAMSize::RAM_64K;
        }
        return RAMSize::UNKNOWN;
    }

    // Get the chip's Flash size
    FlashSize flashSize() {
        uint8_t nvpsiz = ((*(volatile uint32_t*) CHIPID_CIDR) >> CHIPID_CIDR_NVPSIZ) & 0b1111;
        if (nvpsiz == CHIPID_CIDR_NVPSIZ_128K) {
            return FlashSize::FLASH_128K;
        } else if (nvpsiz == CHIPID_CIDR_NVPSIZ_256K) {
            return FlashSize::FLASH_256K;
        } else if (nvpsiz == CHIPID_CIDR_NVPSIZ_512K) {
            return FlashSize::FLASH_512K;
        }
        return FlashSize::UNKNOWN;
    }

    // Copy the 120-bit unique serial number of the chip into the user buffer
    void serialNumber(uint8_t* sn) {
        memcpy(sn, (uint32_t*)SERIAL_NUMBER_ADDR, SERIAL_NUMBER_LENGTH);
    }

    // Set the handler for the specified exception
    void setExceptionHandler(Exception exception, void (*handler)()) {
        _isrVector[static_cast<int>(exception)] = (uint32_t)handler;
    }

    // Set the handler for the specified interrupt
    void setInterruptHandler(Interrupt interrupt, void (*handler)()) {
        _isrVector[N_INTERNAL_EXCEPTIONS + static_cast<int>(interrupt)] = (uint32_t)handler;
    }

    // Enable the specified interrupt with the given priority
    void enableInterrupt(Interrupt interrupt, uint8_t priority) {
        // For ICPR and ISER : 
        // 32 channels and 4 bytes per register :
        // (channel / 32) * 4 = (channel >> 5) << 2 = (channel >> 3) & 0xFFFC
        // channel % 32 = channel & 0x1F

        // For IPR, each channel is coded on a single byte (4 channels in each 4-byte register),
        // we can therefore access each channel directly

        const int channel = static_cast<int>(interrupt);

        // ICPR (Interrupt Clear-Pending Register) : clear any pending interrupt
        (*(volatile uint32_t*) (NVIC_ICPR0 + ((channel >> 3) & 0xFFFC))) = 1 << (channel & 0x1F);

        // IPR (Interrupt Priority Register) : set the interrupt priority
        (*(volatile uint8_t*) (NVIC_IPR0 + channel)) = priority;

        // ISER (Interrupt Set-Enable Register) : enable this interrupt
        (*(volatile uint32_t*) (NVIC_ISER0 + ((channel >> 3) & 0xFFFC))) = 1 << (channel & 0x1F);
    }

    // Disable the specified interrupt
    void disableInterrupt(Interrupt interrupt) {
        // For ICER and ICPR :
        // 32 channels and 4 bytes per register :
        // (channel / 32) * 4 = (channel >> 5) << 2 = (channel >> 3) & 0xFFFC
        // channel % 32 = channel & 0x1F

        const int channel = static_cast<int>(interrupt);

        // ICER (Interrupt Clear-Enable Register) : disable this interrupt
        (*(volatile uint32_t*) (NVIC_ICER0 + ((channel >> 3) & 0xFFFC))) = 1 << (channel & 0x1F);

        // ICPR (Interrupt Clear-Pending Register) : clear any pending interrupt
        (*(volatile uint32_t*) (NVIC_ICPR0 + ((channel >> 3) & 0xFFFC))) = 1 << (channel & 0x1F);
    }

    // Change the priority for the specified interrupt
    void setInterruptPriority(Interrupt interrupt, uint8_t priority) {
        // IPR (Interrupt Priority Register) : set the interrupt priority
        (*(volatile uint8_t*) (NVIC_IPR0 + static_cast<int>(interrupt))) = priority;
    }

    // Disable temporarily all the interrupts at the NVIC level. The user is still
    // able to re-enable some interrupts manually. This is useful to create a code
    // section where only some selected interrupts can be triggered. Use applyStashedInterrupts()
    // to enable the interrupts as they were before.
    void stashInterrupts() {
        disableInterrupts();

        for (int i = 0; i < N_NVIC_IMPLEMENTED; i++) {
            _nvicStash[i] = (*(volatile uint32_t*) (NVIC_ISER0 + i * sizeof(uint32_t)));
            (*(volatile uint32_t*) (NVIC_ICER0 + i * sizeof(uint32_t))) = 0xFFFFFFFF;
        }

        enableInterrupts();
    }

    // Re-enable stashed interrupts. Note that this will not disable interrupts that were
    // manually enabled after stashInterrupts()
    void applyStashedInterrupts() {
        disableInterrupts();

        for (int i = 0; i < N_NVIC_IMPLEMENTED; i++) {
            (*(volatile uint32_t*) (NVIC_ISER0 + i * sizeof(uint32_t))) = _nvicStash[i];
        }

        enableInterrupts();
    }

    Interrupt currentInterrupt() {
        // ICSR (Interrupt Control and State Register) : get the currently active
        // interrupt number from the VECTACTIVE field
        return static_cast<Interrupt>(((*(volatile uint32_t*) ICSR) & 0x1FF) - N_INTERNAL_EXCEPTIONS);
    }

    // Sleep for a specified amount of time
    void sleep(SleepMode mode, unsigned long length, TimeUnit unit) {
        // Select the correct sleep mode
        if (mode >= SleepMode::SLEEP0 && mode <= SleepMode::SLEEP3) {
            *(volatile uint32_t*) SCR &= ~(uint32_t)(1 << SCR_SLEEPDEEP);
        } else {
            *(volatile uint32_t*) SCR |= 1 << SCR_SLEEPDEEP;
        }
        BPM::setSleepMode(mode);

        // The length is optional, if not specified the chip will sleep until an event
        // wakes it up. See PM::enableWakeUpSource() and BPM::enableBackupWakeUpSource().
        if (length > 0) {
            // Convert between units
            if (unit == TimeUnit::SECONDS) {
                length *= 1000;
            }

            // Enable an alarm to wake up the chip after a specified
            // amount of time
            AST::enableAlarm(length);
        }

        // Sleep until a known event happens
        do {
            // WFI : Wait For Interrupt
            // This special ARM instruction can put the chip in sleep mode until
            // an interrupt with a sufficient priority is triggered
            // See §B1.5.17 Power Management in the ARMv7-M Architecture Reference Manual
            __asm__ __volatile__("WFI");
        } while (!(length > 0 
                    ? AST::alarmPassed()                    // If length is specified, wait until the AST alarm is triggered
                    : PM::wakeUpCause() != PM::WakeUpCause::UNKNOWN)); // Otherwise, wait for any interrupt
    }

    // The default sleep mode is SLEEP0
    void sleep(unsigned long length, TimeUnit unit) {
        sleep(SleepMode::SLEEP0, length, unit);
    }

    // Enable SysTick as a simple counter clocked on the CPU clock
    void enableSysTick() {
        // Reset reload and current values (this is a 24-bit timer)
        *(volatile uint32_t*) SYST_RVR = 0xFFFFFF;
        *(volatile uint32_t*) SYST_CVR = 0xFFFFFF;

        *(volatile uint32_t*) SYST_CSR
            = 1 << SYST_CSR_ENABLE      // Enable counter
            | 0 << SYST_CSR_TICKINT;    // Disable interrupt when the counter reaches 0
    }

    // Disable SysTick
    void disableSysTick() {
        *(volatile uint32_t*) SYST_CSR = 0;
    }

    // Waste CPU clock cycles to wait for a specified amount of time
    void waitMicroseconds(unsigned long length) {
        // Save the last CVR (Current Value Register) value at each loop iteration
        uint32_t last = *(volatile uint32_t*) SYST_CVR;

        // Save the RVR (Reset Value Register) register
        uint32_t rvr = *(volatile uint32_t*) SYST_RVR;

        // Compute the number of CPU clock counts to wait for
        uint64_t t = 0;
        uint64_t delay = 0;
        delay = (length * (uint64_t)PM::getCPUClockFrequency()) / 1000000UL;

        // Compensate (approximately) for the function overhead : about 50 clock cycles
        const int OVERHEAD = 50;
        if (delay > OVERHEAD) {
            delay -= OVERHEAD;
        }

        // Do the actual waiting
        while (1) {
            if (t >= delay) {
                break;
            }
            uint32_t current = *(volatile uint32_t*) SYST_CVR;
            if (current < last) {
                t += last - current;
            } else {
                t += last + (rvr - current);
            }
            last = current;
        }
    }


    // Default exception handlers
    void handlerNMI() {
        while (1);
    }

    void handlerHardFault() {
        while (1);
    }

    void handlerMemManage() {
        while (1);
    }

    void handlerBusFault() {
        while (1);
    }

    void handlerUsageFault() {
        while (1);
    }

    void handlerSVCall() {
        while (1);
    }

    void handlerDebugMonitor() {
        while (1);
    }

    void handlerPendSV() {
        while (1);
    }

    void handlerSysTick() {
    }
}