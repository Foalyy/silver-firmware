#ifndef _BPM_H_
#define _BPM_H_

#include <stdint.h>
#include "core.h"

// Backup Power Manager
// This module manages power-saving features
// TODO : finish this module
namespace BPM {

    // Peripheral memory space base address
    const uint32_t BASE = 0x400F0000;


    // Registers addresses
    const uint32_t OFFSET_IER =         0x000; // Interrupt Enable Register
    const uint32_t OFFSET_IDR =         0x004; // Interrupt Disable Register
    const uint32_t OFFSET_IMR =         0x008; // Interrupt Mask Register
    const uint32_t OFFSET_ISR =         0x00C; // Interrupt Status Register
    const uint32_t OFFSET_ICR =         0x010; // Interrupt Clear Register
    const uint32_t OFFSET_SR =          0x014; // Status Register
    const uint32_t OFFSET_UNLOCK =      0x018; // Unlock Register
    const uint32_t OFFSET_PMCON =       0x01C; // Power Mode Control Register
    const uint32_t OFFSET_BKUPWCAUSE =  0x028; // Backup Wake up Cause Register
    const uint32_t OFFSET_BKUPWEN =     0x02C; // Backup Wake up Enable Register
    const uint32_t OFFSET_BKUPPMUX =    0x030; // Backup Pin Muxing Register
    const uint32_t OFFSET_IORET =       0x034; // Input Output Retention Register


    // Subregisters
    const uint32_t SR_PSOK = 0;
    const uint32_t SR_AE = 1;
    const uint32_t UNLOCK_ADDR = 0;
    const uint32_t UNLOCK_KEY = 0xAA << 24;
    const uint32_t PMCON_PS = 0;
    const uint32_t PMCON_PSCREQ = 2;
    const uint32_t PMCON_PSCM = 3;  // Undocumented bit : set to 1 to make a no-halt change
    const uint32_t PMCON_BKUP = 8;
    const uint32_t PMCON_RET = 9;
    const uint32_t PMCON_SLEEP = 12;
    const uint32_t PMCON_CK32S = 16;
    const uint32_t PMCON_FASTWKUP = 24;

    // Error codes
    const Error::Code ERR_UNKNOWN_BACKUP_WAKEUP_CAUSE = 0x0001;

    // Constants
    enum class PowerScaling {
        PS0 = 0,
        PS1 = 1,
        PS2 = 2
    };

    enum class BackupWakeUpCause {
        EIC = 0,
        AST = 1,
        WDT = 2,
        BOD33 = 3,
        BOD18 = 4,
        PICOUART = 5,
    };

    // Some peripherals can wake up the chip from backup mode
    // See enableBackupWakeUpSource() for more details
    enum class BackupWakeUpSource {
        EIC = 0,
        AST = 1,
        WDT = 2,
        BOD33 = 3,
        BOD18 = 4,
        PICOUART = 5,
    };

    // Source for the reference 32KHz (and 1KHz) clocks,
    // used by the AST among others
    enum class CLK32KSource {
        OSC32K, // External crystal
        RC32K,  // Internal RC
    };

    const int N_INTERRUPTS = 1;
    enum class Interrupt {
        PSOK = 0,
    };


    // Module API

    // Power scaling
    void setPowerScaling(PowerScaling ps);
    PowerScaling currentPowerScaling();
    
    // Power save modes
    void setSleepMode(Core::SleepMode mode);
    BackupWakeUpCause backupWakeUpCause();
    void enableBackupWakeUpSource(BackupWakeUpSource src);
    void disableBackupWakeUpSource(BackupWakeUpSource src);
    void disableBackupWakeUpSources();
    void enableBackupPin(unsigned int eicChannel);
    void disableBackupPin(unsigned int eicChannel);

    // 32KHz clock selection
    void set32KHzClockSource(CLK32KSource source);

    // Interrupts
    void enableInterrupt(void (*handler)(), Interrupt interrupt=Interrupt::PSOK);
    void disableInterrupt(Interrupt interrupt=Interrupt::PSOK);

}


#endif