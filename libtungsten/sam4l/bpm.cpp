#include "bpm.h"
#include "pm.h"
#include "eic.h"
#include "flash.h"
#include "error.h"

namespace BPM {

    // Current power scaling setting
    PowerScaling _currentPS = PowerScaling::PS0;

    // Interrupt handlers
    extern uint8_t INTERRUPT_PRIORITY;
    uint32_t _interruptHandlers[N_INTERRUPTS];
    const int _interruptBits[N_INTERRUPTS] = {SR_PSOK};
    void interruptHandlerWrapper();

    void setPowerScaling(PowerScaling ps) {
        Core::stashInterrupts();

        // Enable Flash High Speed mode if entering PS2
        if (ps == PowerScaling::PS2) {
            Flash::enableHighSpeedMode();
        }

        // Get the value of PMCON without PS
        uint32_t pmcon = (*(volatile uint32_t*)(BASE + OFFSET_PMCON)) & ~(uint32_t)(0b11 << PMCON_PS);

        // Unlock the PMCON register, which is locked by default as a safety mesure
        (*(volatile uint32_t*)(BASE + OFFSET_UNLOCK))
                = UNLOCK_KEY               // KEY : Magic word (see datasheet)
                | OFFSET_PMCON;            // ADDR : unlock PMCON

        // Select the power scaling register and request a Power Scaling Change
        (*(volatile uint32_t*)(BASE + OFFSET_PMCON))
                = pmcon
                | static_cast<int>(ps) << PMCON_PS // PS : select Power Scaling mode
                | 1 << PMCON_PSCREQ                // PSCREQ : Power Scaling Change Request
                | 1 << PMCON_PSCM;                 // PSCM : Power Scaling Change Request

        // Wait for the Power Scaling OK flag
        while (!(*(volatile uint32_t*)(BASE + OFFSET_SR) & (1 << SR_PSOK)));

        // Disable Flash High Speed mode if exiting PS2
        if (_currentPS == PowerScaling::PS2) {
            Flash::disableHighSpeedMode();
        }

        // Save the current setting
        _currentPS = ps;

        Core::applyStashedInterrupts();
    }

    PowerScaling currentPowerScaling() {
        // Return the current power scaling setting
        return _currentPS;
    }

    void setSleepMode(Core::SleepMode mode) {
        // Read PMCON (Power Mode Control Register) and reset sleep-related fields
        uint32_t pmcon = (*(volatile uint32_t*)(BASE + OFFSET_PMCON)) & 0xFFFF00FF;

        // Configure mode
        switch (mode) {
            case Core::SleepMode::SLEEP0:
            case Core::SleepMode::SLEEP1:
            case Core::SleepMode::SLEEP2:
            case Core::SleepMode::SLEEP3:
                pmcon |= (static_cast<int>(mode) - static_cast<int>(Core::SleepMode::SLEEP0)) << PMCON_SLEEP;
                break;

            case Core::SleepMode::WAIT:
                // SCR.SLEEPDEEP is already set in Core::sleep()
                break;

            case Core::SleepMode::RETENTION:
                pmcon |= 1 << PMCON_RET;
                break;

            case Core::SleepMode::BACKUP:
                pmcon |= 1 << PMCON_BKUP;
                break;
        }

        // Unlock the PMCON register, which is locked by default as a safety mesure
        (*(volatile uint32_t*)(BASE + OFFSET_UNLOCK))
                = UNLOCK_KEY               // KEY : Magic word (see datasheet)
                | OFFSET_PMCON;            // ADDR : unlock PMCON

        // Write PMCON
        (*(volatile uint32_t*)(BASE + OFFSET_PMCON)) = pmcon;
    }

    // Returns the cause of the last backup wake up
    BackupWakeUpCause backupWakeUpCause() {
        uint32_t bkupwcause = (*(volatile uint32_t*)(BASE + OFFSET_BKUPWCAUSE));
        for (int i = 0; i < 32; i++) {
            if (bkupwcause & 1 << i) {
                return static_cast<BackupWakeUpCause>(i);
            }
        }
        Error::happened(Error::Module::BPM, ERR_UNKNOWN_BACKUP_WAKEUP_CAUSE, Error::Severity::WARNING);
        return BackupWakeUpCause::EIC; // Default value that should not happen
    }

    void enableBackupWakeUpSource(BackupWakeUpSource src) {
        // BKUPWEN (Backup Wake Up Enable Register) : set the corresponding bit
        (*(volatile uint32_t*)(BASE + OFFSET_BKUPWEN))
            |= 1 << static_cast<int>(src);
    }

    void disableBackupWakeUpSource(BackupWakeUpSource src) {
        // BKUPWEN (Backup Wake Up Enable Register) : clear the corresponding bit
        (*(volatile uint32_t*)(BASE + OFFSET_BKUPWEN))
            &= ~(uint32_t)(1 << static_cast<int>(src));
    }

    void disableBackupWakeUpSources() {
        // BKUPWEN (Backup Wake Up Enable Register) : clear the register
        (*(volatile uint32_t*)(BASE + OFFSET_BKUPWEN)) = 0;
    }

    void enableBackupPin(unsigned int eicChannel) {
        if (eicChannel < EIC::N_CHANNELS) {
            (*(volatile uint32_t*)(BASE + OFFSET_BKUPPMUX))
                |= 1 << static_cast<int>(eicChannel);
        }
    }

    void disableBackupPin(unsigned int eicChannel) {
        if (eicChannel < EIC::N_CHANNELS) {
            (*(volatile uint32_t*)(BASE + OFFSET_BKUPPMUX))
                &= ~(uint32_t)(1 << static_cast<int>(eicChannel));
        }
    }

    void set32KHzClockSource(CLK32KSource source) {
        // Read PMCON (Power Mode Control Register) and reset the CK32S field
        uint32_t pmcon = (*(volatile uint32_t*)(BASE + OFFSET_PMCON)) & ~(uint32_t)(1 << PMCON_CK32S);

        // Select the source
        if (source == CLK32KSource::RC32K) {
            pmcon |= 1 << PMCON_CK32S;
        }

        // Unlock the PMCON register, which is locked by default as a safety mesure
        (*(volatile uint32_t*)(BASE + OFFSET_UNLOCK))
                = UNLOCK_KEY               // KEY : Magic word (see datasheet)
                | OFFSET_PMCON;            // ADDR : unlock PMCON

        // Write PMCON back
        (*(volatile uint32_t*)(BASE + OFFSET_PMCON))
            = pmcon;
    }

    void enableInterrupt(void (*handler)(), Interrupt interrupt) {
        // Save the user handler
        _interruptHandlers[static_cast<int>(interrupt)] = (uint32_t)handler;

        // IER (Interrupt Enable Register) : enable the requested interrupt (PSOK by default)
        (*(volatile uint32_t*)(BASE + OFFSET_IER))
                = 1 << _interruptBits[static_cast<int>(interrupt)];

        // Set the handler and enable the module interrupt at the Core level
        Core::setInterruptHandler(Core::Interrupt::BPM, interruptHandlerWrapper);
        Core::enableInterrupt(Core::Interrupt::BPM, INTERRUPT_PRIORITY);
    }

    void disableInterrupt(Interrupt interrupt) {
        // IDR (Interrupt Disable Register) : disable the requested interrupt (PSOK by default)
        (*(volatile uint32_t*)(BASE + OFFSET_IDR))
                = 1 << _interruptBits[static_cast<int>(interrupt)];

        // If no interrupt is enabled anymore, disable the module interrupt at the Core level
        if ((*(volatile uint32_t*)(BASE + OFFSET_IMR)) == 0) {
            Core::disableInterrupt(Core::Interrupt::BPM);
        }
    }

    void interruptHandlerWrapper() {
        // Call the user handler of every interrupt that is enabled and pending
        for (int i = 0; i < N_INTERRUPTS; i++) {
            if ((*(volatile uint32_t*)(BASE + OFFSET_IMR)) & (1 << _interruptBits[i]) // Interrupt is enabled
                    && (*(volatile uint32_t*)(BASE + OFFSET_ISR)) & (1 << _interruptBits[i])) { // Interrupt is pending
                void (*handler)() = (void (*)())_interruptHandlers[i];
                if (handler != nullptr) {
                    handler();
                }

                // Clear the interrupt by reading ISR
                (*(volatile uint32_t*)(BASE + OFFSET_ICR)) = 1 << _interruptBits[i];
            }
        }
    }

}