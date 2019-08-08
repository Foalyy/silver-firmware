#include <core.h>
#include <scif.h>
#include <pm.h>
#include <gpio.h>
#include <spi.h>
#include <adc.h>
#include <stdio.h>
#include "silver.h"
#include "gui.h"
#include "drivers/oled_ssd1306/oled.h"
#include "sync.h"
#include "sync_usb.h"
#include "context.h"
#include "pins.h"


const uint8_t OLED_CONTRAST_DIMMED = 0;
const int OLED_DIM_DELAY = 0; // 30s
const int OLED_TURNOFF_DELAY = 0; // 10min
const int TURNON_DELAY = 1000;
const int TURNOFF_DELAY = 1000;
const int LED_BLINK_DELAY = 2000;
const unsigned long DELAY_VBAT_MEAS = 10000;

int main() {
    // Init the microcontroller
    Core::init();
    SCIF::enableRCFAST(SCIF::RCFASTFrequency::RCFAST_12MHZ);
    PM::setMainClockSource(PM::MainClockSource::RCFAST);
    Error::setHandler(Error::Severity::WARNING, warningHandler);
    Error::setHandler(Error::Severity::CRITICAL, criticalHandler);

    // Power
    Core::sleep(TURNON_DELAY);
    GPIO::enableOutput(PIN_PW_EN, GPIO::HIGH);

    // Enable the SPI interface
    SPI::setPin(SPI::PinFunction::MISO, PIN_MISO);
    SPI::setPin(SPI::PinFunction::MOSI, PIN_MOSI);
    SPI::setPin(SPI::PinFunction::SCK, PIN_SCK);
    SPI::enableMaster();

    // Init the GUI
    GUI::init();
    OLED::setContrast(Context::_brightness * 25);

    // Init the buttons
    GPIO::enableInput(PIN_BTN_UP, GPIO::Pulling::PULLUP);
    GPIO::enableInput(PIN_BTN_DOWN, GPIO::Pulling::PULLUP);
    GPIO::enableInput(PIN_BTN_LEFT, GPIO::Pulling::PULLUP);
    GPIO::enableInput(PIN_BTN_RIGHT, GPIO::Pulling::PULLUP);
    GPIO::enableInput(PIN_BTN_OK, GPIO::Pulling::PULLUP);
    GPIO::enableInput(PIN_BTN_PW);
    GPIO::enableInput(PIN_BTN_TRIGGER, GPIO::Pulling::PULLUP);
    GPIO::enableInput(PIN_BTN_FOCUS, GPIO::Pulling::PULLUP);

    // Init the leds
    GPIO::enableOutput(PIN_LED_TRIGGER, GPIO::HIGH);
    GPIO::enableOutput(PIN_LED_FOCUS, GPIO::HIGH);
    GPIO::enableOutput(PIN_LED_INPUT, GPIO::HIGH);

    // Init the battery voltage measurement
    GPIO::enableOutput(PIN_VBAT_MEAS_CMD, GPIO::LOW);
    ADC::setPin(ADC_VBAT, PIN_VBAT_MEAS);
    ADC::enable(ADC_VBAT);

    // Init the sync module
    if (!Sync::init()) {
        warningHandler();
    }

    // Init USB
    SyncUSB::init();

    // Init input
    GPIO::enableInput(PIN_INPUT, GPIO::Pulling::PULLUP);

    // Init outputs
    GPIO::enableOutput(PIN_FOCUS, GPIO::LOW);
    GPIO::enableOutput(PIN_TRIGGER, GPIO::LOW);

    // Read settings
    Context::read();
    GUI::updateBrightness();


    // Current state
    bool lastWaiting = false;
    bool lastFocus = false;
    bool lastTrigger = false;
    bool lastBtnPw = true;
    bool lastBtnFocus = false;
    bool lastBtnTrigger = false;
    bool lastBtnOk = false;
    bool lastInput = false;
    Core::Time t = Core::time();
    Core::Time tLastActivity = t;
    Core::Time tBtnPwPressed = 0;
    Core::Time tWaitingLed = 0;
    const int REMOTE_HOLD_KEEPALIVE = 500;
    const int REMOTE_HOLD_TIMEOUT = 3000;
    bool remoteFocusHold = false;
    bool isRemoteFocusHoldFromUSB = false;
    Core::Time tRemoteFocusHold = 0;
    Core::Time tFocusHoldKeepalive = 0;
    bool remoteTriggerHold = false;
    bool isRemoteTriggerHoldFromUSB = false;
    Core::Time tRemoteTriggerHold = 0;
    Core::Time tTriggerHoldKeepalive = 0;
    Core::Time tRefreshFooter = 0;
    bool screenDimmed = false;
    bool screenOff = false;
    Core::Time tVbatMeas = 0;

    // Main loop
    while (1) {
        bool waiting = false;
        bool focus = false;
        bool trigger = false;
        bool refresh = false;
        bool refreshFooter = false;

        // Power button
        bool btnPw = GPIO::get(PIN_BTN_PW);
        if (!lastBtnPw && btnPw) {
            // Button pressed
            t = Core::time();
            tBtnPwPressed = t;
            tLastActivity = t;
        } else if (!btnPw) {
            // Button released
            tBtnPwPressed = 0;
        }
        if (tBtnPwPressed > 0 && Core::time() - tBtnPwPressed >= TURNOFF_DELAY) {
            // Shutdown

            // Display the shutdown message on the screen
            GUI::showExitScreen();

            // Save the current settings
            Context::save();

            // Wait a second
            Core::sleep(1000);

            // Turn off the screen and the power LED
            OLED::disable();

            // Ready to shutdown, release the power supply enable line
            GPIO::set(PIN_PW_EN, GPIO::LOW);
        }

        // Change menu when the button is pressed
        bool buttonPressed = GUI::handleButtons();
        if (buttonPressed) {
            tLastActivity = Core::time();
        }
        refresh = refresh || buttonPressed;

        // Trigger button
        t = Core::time();
        bool btnTrigger = !GPIO::get(PIN_BTN_TRIGGER);
        if (!lastBtnTrigger && btnTrigger) {
            // Pressed
            tLastActivity = t;
            refresh = true;
            if (Context::_submenuTriggerHold) {
                if (Context::_triggerSync) {
                    Sync::send(Sync::CMD_TRIGGER_HOLD);
                    tTriggerHoldKeepalive = Core::time();
                }
            } else {
                if (Context::_tTrigger == 0) {
                    // Start
                    Context::_tTrigger = t;
                    Context::_skipDelay = false;
                    if (Context::_triggerSync) {
                        Sync::send(Sync::CMD_TRIGGER);
                    }
                } else {
                    // Stop
                    Context::_tTrigger = 0;
                    if (Context::_triggerSync) {
                        Sync::send(Sync::CMD_TRIGGER_RELEASE);
                    }
                }
            }
        } else if (lastBtnTrigger && !btnTrigger) {
            // Released
            tLastActivity = t;
            refresh = true;
            if (Context::_submenuTriggerHold) {
                if (Context::_triggerSync) {
                    Sync::send(Sync::CMD_TRIGGER_RELEASE);
                    tTriggerHoldKeepalive = 0;
                }
            }
        }
        if (btnTrigger && Context::_submenuTriggerHold) {
            // Hold down
            tLastActivity = t;
            trigger = true;
        }

        // Focus button
        t = Core::time();
        bool btnFocus = !GPIO::get(PIN_BTN_FOCUS);
        if (!lastBtnFocus && btnFocus) {
            // Pressed
            tLastActivity = t;
            refresh = true;
            if (Context::_submenuFocusHold) {
                if (Context::_triggerSync) {
                    Sync::send(Sync::CMD_FOCUS_HOLD);
                    tFocusHoldKeepalive = Core::time();
                }
            } else {
                if (Context::_tFocus == 0) {
                    // Start
                    Context::_tFocus = t;
                    if (Context::_triggerSync) {
                        Sync::send(Sync::CMD_FOCUS);
                    }
                } else {
                    // Stop
                    Context::_tFocus = 0;
                    if (Context::_triggerSync) {
                        Sync::send(Sync::CMD_FOCUS_RELEASE);
                    }
                }
            }
        } else if (lastBtnFocus && !btnFocus) {
            // Released
            tLastActivity = t;
            refresh = true;
            if (Context::_submenuFocusHold) {
                if (Context::_triggerSync) {
                    Sync::send(Sync::CMD_FOCUS_RELEASE);
                    tFocusHoldKeepalive = 0;
                }
            }
        }
        if (btnFocus && Context::_submenuFocusHold) {
            // Hold down
            tLastActivity = t;
            focus = true;
        }

        // External input
        bool inputStatus = !GPIO::get(PIN_INPUT);
        if (Context::_submenuTriggerHold || Context::_inputMode == GUI::SUBMENU_INPUT_MODE_PASSTHROUGH) {
            if (inputStatus) {
                // Input low
                tLastActivity = t;
                trigger = true;
                if (!lastInput) {
                    // Input just asserted
                    if (Context::_triggerSync) {
                        Sync::send(Sync::CMD_TRIGGER_HOLD);
                        tTriggerHoldKeepalive = Core::time();
                    }
                }
            } else if (!inputStatus && lastInput) {
                if (Context::_triggerSync) {
                    Sync::send(Sync::CMD_TRIGGER_RELEASE);
                    tTriggerHoldKeepalive = 0;
                }
            }
        } else if (Context::_inputMode == GUI::SUBMENU_INPUT_MODE_TRIGGER && Context::_tTrigger == 0 && !lastInput && inputStatus) {
            tLastActivity = t;
            Context::_tTrigger = Core::time();
            Context::_skipDelay = false;
            if (Context::_triggerSync) {
                Sync::send(Sync::CMD_TRIGGER);
            }
        } else if (Context::_inputMode == GUI::SUBMENU_INPUT_MODE_TRIGGER_NODELAY && Context::_tTrigger == 0 && !lastInput && inputStatus) {
            tLastActivity = t;
            Context::_tTrigger = Core::time();
            Context::_skipDelay = true;
            if (Context::_triggerSync) {
                Sync::send(Sync::CMD_TRIGGER_NO_DELAY);
            }
        }

        // Focus and trigger hold with center button
        bool btnOk = !GPIO::get(PIN_BTN_OK);
        if (Context::_menuItemSelected == GUI::MENU_TRIGGER && Context::_submenuItemSelected == GUI::SUBMENU_TRIGGER_FOCUS && Context::_submenuFocusHold) {
            if (btnOk) {
                focus = true;
                if (!lastBtnOk) {
                    Sync::send(Sync::CMD_FOCUS_HOLD);
                    tFocusHoldKeepalive = Core::time();
                }
            } else if (lastBtnOk && !btnOk) {
                Sync::send(Sync::CMD_FOCUS_RELEASE);
                tFocusHoldKeepalive = 0;
            }
        } else if (Context::_menuItemSelected == GUI::MENU_TRIGGER && Context::_submenuItemSelected == GUI::SUBMENU_TRIGGER_SHOOT && Context::_submenuTriggerHold) {
            if (btnOk) {
                trigger = true;
                if (!lastBtnOk) {
                    Sync::send(Sync::CMD_TRIGGER_HOLD);
                    tTriggerHoldKeepalive = Core::time();
                }
            } else if (lastBtnOk && !btnOk) {
                Sync::send(Sync::CMD_TRIGGER_RELEASE);
                tTriggerHoldKeepalive = 0;
            }
        }

        // Focus and trigger hold keepalive
        t = Core::time();
        if (tFocusHoldKeepalive > 0 && t >= tFocusHoldKeepalive + REMOTE_HOLD_KEEPALIVE) {
            Sync::send(Sync::CMD_FOCUS_HOLD);
            tFocusHoldKeepalive = t;
        }
        if (tTriggerHoldKeepalive > 0 && t >= tTriggerHoldKeepalive + REMOTE_HOLD_KEEPALIVE) {
            Sync::send(Sync::CMD_TRIGGER_HOLD);
            tTriggerHoldKeepalive = t;
        }

        // Receive data from other modules and from USB
        bool commandAvailable = false;
        uint8_t command = 0x00;
        uint8_t payload[Sync::MAX_PAYLOAD_SIZE];
        int payloadSize = 0;
        bool isCommandFromUSB = false;
        if (Sync::commandAvailable()) {
            command = Sync::getCommand();
            payloadSize = Sync::getPayload(payload);
            commandAvailable = true;
        }
        if (!commandAvailable && SyncUSB::commandAvailable()) {
            command = SyncUSB::getCommand();
            payloadSize = SyncUSB::getPayload(payload);
            commandAvailable = true;
            isCommandFromUSB = true;
        }
        if (commandAvailable) {
            if (command == GUI::MENU_TRIGGER && (isCommandFromUSB || Context::_triggerSync) && payloadSize >= 2) {
                Context::_submenuFocusHold = payload[0];
                Context::_submenuTriggerHold = payload[1];
                if (isCommandFromUSB && payloadSize >= 3) {
                    Context::_triggerSync = payload[2];
                }
                if (isCommandFromUSB && Context::_triggerSync) {
                    Sync::send(command, payload, payloadSize);
                }
                if (!isCommandFromUSB && SyncUSB::isConnected()) {
                    payload[2] = Context::_triggerSync;
                    SyncUSB::send(command, payload, payloadSize + 1);
                }
            } else if (command == GUI::MENU_DELAY && (isCommandFromUSB || Context::_delaySync) && payloadSize >= 3) {
                Context::_delayMs = ((payload[0] << 16) + (payload[1] << 8) + payload[2]) * 100;
                if (isCommandFromUSB && payloadSize >= 4) {
                    Context::_delaySync = payload[3];
                }
                if (isCommandFromUSB && Context::_delaySync) {
                    Sync::send(command, payload, payloadSize);
                }
                if (!isCommandFromUSB && SyncUSB::isConnected()) {
                    payload[3] = Context::_delaySync;
                    SyncUSB::send(command, payload, payloadSize + 1);
                }
            } else if (command == GUI::MENU_INTERVAL && (isCommandFromUSB || Context::_intervalSync) && payloadSize >= 4) {
                Context::_intervalNShots = payload[0];
                Context::_intervalDelayMs = ((payload[1] << 16) + (payload[2] << 8) + payload[3]) * 100;
                if (isCommandFromUSB && payloadSize >= 5) {
                    Context::_intervalSync = payload[4];
                }
                if (isCommandFromUSB && Context::_intervalSync) {
                    Sync::send(command, payload, payloadSize);
                }
                if (!isCommandFromUSB && SyncUSB::isConnected()) {
                    payload[4] = Context::_intervalSync;
                    SyncUSB::send(command, payload, payloadSize + 1);
                }
            } else if (command == GUI::MENU_INPUT && (isCommandFromUSB || Context::_inputSync) && payloadSize >= 1) {
                Context::_inputMode = payload[0];
                if (isCommandFromUSB && payloadSize >= 2) {
                    Context::_inputSync = payload[1];
                }
                if (isCommandFromUSB && Context::_inputSync) {
                    Sync::send(command, payload, payloadSize);
                }
                if (!isCommandFromUSB && SyncUSB::isConnected()) {
                    payload[1] = Context::_inputSync;
                    SyncUSB::send(command, payload, payloadSize + 1);
                }
            } else if (command == GUI::MENU_SETTINGS && isCommandFromUSB && payloadSize >= 1) {
                Context::_syncChannel = payload[0];
            } else if (command == GUI::MENU_TIMINGS && (isCommandFromUSB || Context::_timingsSync) && payloadSize >= 6) {
                Context::_timingsFocusDurationMs = ((payload[0] << 16) + (payload[1] << 8) + payload[2]) * 100;
                Context::_timingsTriggerDurationMs = ((payload[3] << 8) + (payload[4] << 8) + payload[5]) * 100;
                if (isCommandFromUSB && payloadSize >= 7) {
                    Context::_timingsSync = payload[6];
                }
                if (isCommandFromUSB && Context::_timingsSync) {
                    Sync::send(command, payload, payloadSize);
                }
                if (!isCommandFromUSB && SyncUSB::isConnected()) {
                    payload[6] = Context::_timingsSync;
                    SyncUSB::send(command, payload, payloadSize + 1);
                }
            } else if (command == Sync::CMD_FOCUS && (isCommandFromUSB || Context::_triggerSync)) {
                Context::_tFocus = Core::time();
                if (isCommandFromUSB && Context::_triggerSync) {
                    Sync::send(command);
                }
            } else if (command == Sync::CMD_FOCUS_HOLD && (isCommandFromUSB || Context::_triggerSync)) {
                if (!remoteFocusHold) {
                    remoteFocusHold = true;
                    isRemoteFocusHoldFromUSB = isCommandFromUSB;
                }
                tRemoteFocusHold = Core::time();
                if (isCommandFromUSB && Context::_triggerSync) {
                    Sync::send(command);
                }
            } else if (command == Sync::CMD_FOCUS_RELEASE && (isCommandFromUSB || Context::_triggerSync)) {
                Context::_tFocus = 0;
                remoteFocusHold = false;
                if (isCommandFromUSB && Context::_triggerSync) {
                    Sync::send(command);
                }
            } else if (command == Sync::CMD_TRIGGER && (isCommandFromUSB || Context::_triggerSync)) {
                Context::_tTrigger = Core::time();
                Context::_skipDelay = false;
                if (isCommandFromUSB && Context::_triggerSync) {
                    Sync::send(command);
                }
            } else if (command == Sync::CMD_TRIGGER_NO_DELAY && (isCommandFromUSB || Context::_triggerSync)) {
                Context::_tTrigger = Core::time();
                Context::_skipDelay = true;
                if (isCommandFromUSB && Context::_triggerSync) {
                    Sync::send(command);
                }
            } else if (command == Sync::CMD_TRIGGER_HOLD && (isCommandFromUSB || Context::_triggerSync)) {
                if (!remoteTriggerHold) {
                    remoteTriggerHold = true;
                    isRemoteTriggerHoldFromUSB = isCommandFromUSB;
                }
                tRemoteTriggerHold = Core::time();
                Context::_skipDelay = false;
                if (isCommandFromUSB && Context::_triggerSync) {
                    Sync::send(command);
                }
            } else if (command == Sync::CMD_TRIGGER_RELEASE && (isCommandFromUSB || Context::_triggerSync)) {
                Context::_tTrigger = 0;
                Context::_skipDelay = false;
                remoteTriggerHold = false;
                if (isCommandFromUSB && Context::_triggerSync) {
                    Sync::send(command);
                }
            }

            refresh = true;
        }

        // Remote focus and trigger hold
        t = Core::time();
        if (remoteFocusHold) {
            focus = true;
            if (t >= tRemoteFocusHold + REMOTE_HOLD_TIMEOUT) {
                // Timeout
                focus = false;
                Context::_tFocus = 0;
                remoteFocusHold = false;
                if (isRemoteFocusHoldFromUSB && Context::_triggerSync) {
                    Sync::send(Sync::CMD_FOCUS_RELEASE);
                }
            }
        }
        if (remoteTriggerHold) {
            trigger = true;
            if (t >= tRemoteTriggerHold + REMOTE_HOLD_TIMEOUT) {
                // Timeout
                trigger = false;
                Context::_tTrigger = 0;
                remoteTriggerHold = false;
                if (isRemoteTriggerHoldFromUSB && Context::_triggerSync) {
                    Sync::send(Sync::CMD_TRIGGER_RELEASE);
                }
            }
        }

        // Focus and trigger timings (non-hold)
        t = Core::time();
        if (Context::_tFocus > 0 && !Context::_submenuFocusHold) {
            if (t > Context::_tFocus + Context::_timingsFocusDurationMs) {
                Context::_tFocus = 0;
            } else {
                focus = true;
            }
        }
        if (Context::_tTrigger > 0 && !Context::_submenuTriggerHold) {
            // Start and end times
            unsigned int intervalDelayMs = Context::_intervalDelayMs;
            if (intervalDelayMs < Context::_timingsFocusDurationMs + Context::_timingsTriggerDurationMs) {
                intervalDelayMs = Context::_timingsFocusDurationMs + Context::_timingsTriggerDurationMs;
            }
            Core::Time tStart = Context::_tTrigger + (Context::_skipDelay ? 0 : Context::_delayMs);
            Core::Time tEnd = tStart + (Context::_intervalNShots - 1) * intervalDelayMs + Context::_timingsFocusDurationMs + Context::_timingsTriggerDurationMs;

            if (t < tStart) {
                waiting = true;
                Context::_shotsLeft = Context::_intervalNShots;
                Context::_countdown = tStart - t;
                if (tWaitingLed == 0) {
                    tWaitingLed = Core::time();
                }
            } else {
                if (t >= tEnd) {
                    Context::_tTrigger = 0;
                } else {
                    unsigned int intervalNumber = (t - tStart) / intervalDelayMs;
                    Core::Time tInInterval = (t - tStart) % intervalDelayMs;
                    Context::_shotsLeft = Context::_intervalNShots - intervalNumber;
                    if (tInInterval < Context::_timingsFocusDurationMs) {
                        focus = true;
                        Context::_countdown = Context::_timingsFocusDurationMs - tInInterval;
                    } else if (tInInterval < Context::_timingsFocusDurationMs + Context::_timingsTriggerDurationMs) {
                        trigger = true;
                        Context::_countdown = Context::_timingsFocusDurationMs + Context::_timingsTriggerDurationMs - tInInterval;
                    } else {
                        waiting = true;
                        Context::_shotsLeft -= 1;
                        Context::_countdown = intervalDelayMs - tInInterval;
                    }
                }
            }
        }
        GPIO::set(PIN_FOCUS, focus || trigger);
        GPIO::set(PIN_TRIGGER, trigger);

        // Input LED
        if (inputStatus) {
            // Off
            GPIO::set(PIN_LED_INPUT, GPIO::LOW);
        } else {
            // On
            GPIO::set(PIN_LED_INPUT, GPIO::HIGH);
        }

        // Focus LED
        if (focus) {
            // Off
            GPIO::set(PIN_LED_FOCUS, GPIO::LOW);
        } else {
            // On
            GPIO::set(PIN_LED_FOCUS, GPIO::HIGH);
        }

        // Trigger LED
        if (waiting) {
            // Blink the trigger LED while waiting
            const int DELAY = 400;
            t = Core::time();
            if (t - tWaitingLed < DELAY / 2) {
                // On
                GPIO::set(PIN_LED_TRIGGER, GPIO::LOW);
            } else if (t - tWaitingLed < DELAY) {
                // Off
                GPIO::set(PIN_LED_TRIGGER, GPIO::HIGH);
            } else {
                tWaitingLed += DELAY;
            }
        } else if (trigger) {
            // On
            GPIO::set(PIN_LED_TRIGGER, GPIO::LOW);
        } else {
            // Off
            GPIO::set(PIN_LED_TRIGGER, GPIO::HIGH);
        }

        // Measure battery voltage
        t = Core::time();
        if (tVbatMeas == 0 || t >= tVbatMeas + DELAY_VBAT_MEAS) {
            GPIO::set(PIN_VBAT_MEAS_CMD, GPIO::HIGH);
            Core::sleep(10);
            Context::_vBat = 2 * ADC::read(ADC_VBAT);
            GPIO::set(PIN_VBAT_MEAS_CMD, GPIO::LOW);
            refreshFooter = true;
            tVbatMeas = t;
        }

        // Dim then turn off the screen in case of inactivity
        t = Core::time();
        if (OLED_TURNOFF_DELAY > 0 && !screenOff && t - tLastActivity > OLED_TURNOFF_DELAY) {
            OLED::disable();
            screenOff = true;
        } if (OLED_DIM_DELAY > 0 && !screenDimmed && t - tLastActivity > OLED_DIM_DELAY) {
            OLED::setContrast(OLED_CONTRAST_DIMMED);
            screenDimmed = true;
        } else if (screenDimmed && t - tLastActivity <= OLED_DIM_DELAY) {
            OLED::setContrast(Context::_brightness * 25);
            screenDimmed = false;
            OLED::enable();
            screenOff = false;
        }

        // Refresh the footer when there is a change of state
        if ((waiting || focus || trigger) && t > tRefreshFooter + (Context::_countdown < 10000 ? 100 : 1000)) {
            refreshFooter = true;
            tRefreshFooter = t;
        }
        if (lastWaiting != waiting || lastFocus != focus || lastTrigger != trigger || lastInput != inputStatus) {
            refreshFooter = true;
        }

        // Update the display
        GUI::update(refresh, refreshFooter, trigger, focus, waiting, inputStatus);

        Core::sleep(10);

        lastWaiting = waiting;
        lastFocus = focus;
        lastTrigger = trigger;
        lastBtnPw = btnPw;
        lastBtnFocus = btnFocus;
        lastBtnTrigger = btnTrigger;
        lastInput = inputStatus;
        lastBtnOk = btnOk;
    }
}


void warningHandler(Error::Module module, int userModule, Error::Code code) {
    GPIO::set(PIN_LED_TRIGGER, GPIO::LOW);
    Core::sleep(100);
    GPIO::set(PIN_LED_TRIGGER, GPIO::HIGH);
    Core::sleep(100);
    GPIO::set(PIN_LED_TRIGGER, GPIO::LOW);
    Core::sleep(100);
    GPIO::set(PIN_LED_TRIGGER, GPIO::HIGH);
    Core::sleep(100);
    GPIO::set(PIN_LED_TRIGGER, GPIO::LOW);
    Core::sleep(100);
}

void criticalHandler(Error::Module module, int userModule, Error::Code code) {
    while (1) {
        GPIO::set(PIN_LED_TRIGGER, GPIO::LOW);
        Core::sleep(100);
        GPIO::set(PIN_LED_TRIGGER, GPIO::HIGH);
        Core::sleep(100);
    }
}
