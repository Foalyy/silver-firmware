#include "gui.h"
#include "sync.h"
#include "pins.h"
#include "drivers/oled_ssd1306/oled.h"
#include "drivers/oled_ssd1306/font.h"
#include <string.h>


const Font::Char8 ICON_TRIGGER = {
    8,
    8,
    {
        0b00111100,
        0b01000010,
        0b10000001,
        0b10011001,
        0b10011001,
        0b10000001,
        0b01000010,
        0b00111100,
    }
};
const Font::Char8 ICON_DELAY = {
    8,
    8,
    {
        0b00111100,
        0b01000010,
        0b10000001,
        0b10011101,
        0b10010001,
        0b10010001,
        0b01000010,
        0b00111100,
    }
};
const Font::Char8 ICON_INTVL = {
    8,
    8,
    {
        0b11100000,
        0b10100000,
        0b10101000,
        0b11101010,
        0b00001010,
        0b00111010,
        0b00000010,
        0b00001110,
    }
};
const Font::Char8 ICON_INPUT = {
    7,
    8,
    {
        0b00010000,
        0b00010000,
        0b10010010,
        0b01010100,
        0b00111000,
        0b00010000,
        0b11111110,
    }
};
const Font::Char8 ICON_SETTINGS = {
    6,
    8,
    {
        0b00001100,
        0b00011111,
        0b11110000,
        0b11110000,
        0b00011111,
        0b00001100,
    }
};
const Font::Char8 ICON_ADVANCED = {
    8,
    8,
    {
        0b00000000,
        0b01110000,
        0b11111000,
        0b11011000,
        0b11111000,
        0b01110100,
        0b00001110,
        0b00000100,
    }
};
const Font::Char8 ICON_FOCUS = {
    7,
    8,
    {
        0b01111110,
        0b10000001,
        0b10111101,
        0b10010101,
        0b10000101,
        0b10000001,
        0b01111110,
    }
};

const Font::Char8 MENU_ICONS[GUI::N_MENU_ITEMS] = {
    ICON_TRIGGER,
    ICON_DELAY,
    ICON_INTVL,
    ICON_INPUT,
    ICON_SETTINGS,
    ICON_ADVANCED,
};
const char MENU_LABELS[GUI::N_MENU_ITEMS][10] = {
    "TRIGGER",
    "DELAY",
    "INTERVAL",
    "INPUT",
    "SETTINGS",
    "ADVANCED",
};



const int MENU_HEIGHT = 12;
Core::Time _tMenuChange = 0;
const int DELAY_MENU_LABEL = 500;


void GUI::init() {
    // Init the screen
    OLED::initScreen(0, PIN_OLED_DC, PIN_OLED_RES);
    OLED::setUpsideDown(true);
    GUI::showMenu();
    GUI::showMenuContent();
    OLED::refresh();
}

void GUI::showMenu() {
    // Clear the whole screen
    OLED::clear();

    // Print tabs
    const int tabCenterX = (Context::_menuItemSelected + 1) * OLED::WIDTH / (N_MENU_ITEMS + 1);
    for (int x = 0; x < OLED::WIDTH; x++) {
        if (x > tabCenterX - 9 && x < tabCenterX + 8) {
            if (Context::_submenuItemSelected == 0) {
                for (int y = 0; y <= MENU_HEIGHT; y++) {
                    OLED::setPixel(x, y);
                }
            } else {
                OLED::setPixel(x, 0);
            }
        } else if (x == tabCenterX - 9 || x == tabCenterX + 8) {
            if (Context::_submenuItemSelected == 0) {
                for (int y = 1; y <= MENU_HEIGHT; y++) {
                    OLED::setPixel(x, y);
                }
            } else {
                OLED::setPixel(x, 1);
            }
        } else if (x == tabCenterX - 10 || x == tabCenterX + 9) {
            for (int y = 2; y <= MENU_HEIGHT; y++) {
                OLED::setPixel(x, y);
            }
        } else {
            OLED::setPixel(x, MENU_HEIGHT);
        }
    }

    // Print icons
    for (int i = 0; i < N_MENU_ITEMS; i++) {
        const Font::Char8 c = MENU_ICONS[i];
        OLED::setInverted(i == Context::_menuItemSelected && Context::_submenuItemSelected == 0);
        OLED::printMedium((i + 1) * OLED::WIDTH / (N_MENU_ITEMS + 1) - c.width / 2, 2 + (i == Context::_menuItemSelected ? 1 : 0), c);
    }
}

void GUI::setMenu(int menuItemSelected) {
    Context::_menuItemSelected = menuItemSelected;

    showMenu();

    _tMenuChange = Core::time();
}

void GUI::showMenuContent() {
    bool displayMenuLabel = false;
    if (_tMenuChange > 0) {
        if (Core::time() < _tMenuChange + DELAY_MENU_LABEL) {
            displayMenuLabel = true;
        } else {
            _tMenuChange = 0;
        }
    }

    // Clear the main region of the screen
    OLED::clear(0, MENU_HEIGHT + 1, OLED::WIDTH, OLED::HEIGHT - (MENU_HEIGHT + 1));

    if (displayMenuLabel) {
        // Print label
        OLED::setSize(Font::Size::LARGE);
        OLED::printCentered(OLED::WIDTH / 2, MENU_HEIGHT + (OLED::HEIGHT - MENU_HEIGHT) / 2 - 16 / 2, MENU_LABELS[Context::_menuItemSelected]);

    } else {
        OLED::setSize(Font::Size::MEDIUM);
        const int buttonHeight = 15;

        if (Context::_menuItemSelected == MENU_TRIGGER) {
            if (Context::_tFocus == 0) {
                if (!Context::_submenuFocusHold) {
                    OLED::button(2, MENU_HEIGHT + 3, OLED::WIDTH - 4, buttonHeight, "FOCUS", Context::_submenuItemSelected == SUBMENU_TRIGGER_FOCUS, Context::_submenuItemSelected == SUBMENU_TRIGGER_FOCUS && Context::_btnOkPressed, false, true);
                } else {
                    OLED::button(2, MENU_HEIGHT + 3, OLED::WIDTH - 4, buttonHeight, "FOCUS HOLD", Context::_submenuItemSelected == SUBMENU_TRIGGER_FOCUS, Context::_submenuItemSelected == SUBMENU_TRIGGER_FOCUS && Context::_btnOkPressed, true, false);
                }
            } else {
                OLED::button(2, MENU_HEIGHT + 3, OLED::WIDTH - 4, buttonHeight, "STOP", Context::_submenuItemSelected == SUBMENU_TRIGGER_FOCUS, Context::_submenuItemSelected == SUBMENU_TRIGGER_FOCUS && Context::_btnOkPressed);
            }
            if (Context::_tTrigger == 0) {
                if (!Context::_submenuTriggerHold) {
                    OLED::button(2, MENU_HEIGHT + 3 + buttonHeight + 2, OLED::WIDTH - 4, buttonHeight, "TRIGGER", Context::_submenuItemSelected == SUBMENU_TRIGGER_SHOOT, Context::_submenuItemSelected == SUBMENU_TRIGGER_SHOOT && Context::_btnOkPressed, false, true);
                } else {
                    OLED::button(2, MENU_HEIGHT + 3 + buttonHeight + 2, OLED::WIDTH - 4, buttonHeight, "TRIGGER HOLD", Context::_submenuItemSelected == SUBMENU_TRIGGER_SHOOT, Context::_submenuItemSelected == SUBMENU_TRIGGER_SHOOT && Context::_btnOkPressed, true, false);
                }
            } else {
                OLED::button(2, MENU_HEIGHT + 3 + buttonHeight + 2, OLED::WIDTH - 4, buttonHeight, "STOP", Context::_submenuItemSelected == SUBMENU_TRIGGER_SHOOT, Context::_submenuItemSelected == SUBMENU_TRIGGER_SHOOT && Context::_btnOkPressed);
            }
            OLED::checkbox(2, OLED::HEIGHT - buttonHeight, OLED::WIDTH - 4, buttonHeight, "SYNC", Context::_submenuItemSelected == SUBMENU_TRIGGER_SYNC, Context::_submenuItemSelected == SUBMENU_TRIGGER_SYNC && Context::_btnOkPressed, Context::_triggerSync);

        } else if (Context::_menuItemSelected == MENU_DELAY) {
            char strDelay[14] = "DELAY : \0\0\0\0\0";
            if (Context::_delayMs >= 10000) {
                strDelay[8] = ((Context::_delayMs / 10000) % 10) + '0';
                strDelay[9] = ((Context::_delayMs / 1000) % 10) + '0';
                strDelay[10] = 'S';
            } else if (Context::_delayMs >= 1000) {
                strDelay[8] = ((Context::_delayMs / 1000) % 10) + '0';
                strDelay[9] = 'S';
            } else {
                strDelay[8] = '0';
                strDelay[9] = '.';
                strDelay[10] = ((Context::_delayMs / 100) % 10) + '0';
                strDelay[11] = 'S';
            }
            OLED::button(2, MENU_HEIGHT + 3, OLED::WIDTH - 4, buttonHeight, strDelay, Context::_submenuItemSelected == SUBMENU_DELAY_DELAY, false);
            OLED::checkbox(2, OLED::HEIGHT - buttonHeight, OLED::WIDTH - 4, buttonHeight, "SYNC", Context::_submenuItemSelected == SUBMENU_DELAY_SYNC, Context::_submenuItemSelected == SUBMENU_DELAY_SYNC && Context::_btnOkPressed, Context::_delaySync);

        } else if (Context::_menuItemSelected == MENU_INTERVAL) {
            char strShots[12] = "SHOTS : \0\0\0";
            if (Context::_intervalNShots >= 100) {
                strShots[8] = ((Context::_intervalNShots / 100) % 10) + '0';
                strShots[9] = ((Context::_intervalNShots / 10) % 10) + '0';
                strShots[10] = (Context::_intervalNShots % 10) + '0';
            } else if (Context::_intervalNShots >= 10) {
                strShots[8] = ((Context::_intervalNShots / 10) % 10) + '0';
                strShots[9] = (Context::_intervalNShots % 10) + '0';
            } else {
                strShots[8] = (Context::_intervalNShots % 10) + '0';
            }
            OLED::button(2, MENU_HEIGHT + 3, OLED::WIDTH - 4, buttonHeight, strShots, Context::_submenuItemSelected == SUBMENU_INTERVAL_SHOTS, false);

            char strDelay[17] = "INTERVAL : \0\0\0\0\0";
            if (Context::_intervalDelayMs >= 10000) {
                strDelay[11] = ((Context::_intervalDelayMs / 10000) % 10) + '0';
                strDelay[12] = ((Context::_intervalDelayMs / 1000) % 10) + '0';
                strDelay[13] = 'S';
            } else if (Context::_intervalDelayMs >= 1000) {
                strDelay[11] = ((Context::_intervalDelayMs / 1000) % 10) + '0';
                strDelay[12] = 'S';
            } else {
                strDelay[11] = '0';
                strDelay[12] = '.';
                strDelay[13] = ((Context::_intervalDelayMs / 100) % 10) + '0';
                strDelay[14] = 'S';
            }
            OLED::button(2, MENU_HEIGHT + 3 + buttonHeight + 2, OLED::WIDTH - 4, buttonHeight, strDelay, Context::_submenuItemSelected == SUBMENU_INTERVAL_DELAY, false);

            OLED::checkbox(2, OLED::HEIGHT - buttonHeight, OLED::WIDTH - 4, buttonHeight, "SYNC", Context::_submenuItemSelected == SUBMENU_INTERVAL_SYNC, Context::_submenuItemSelected == SUBMENU_INTERVAL_SYNC && Context::_btnOkPressed, Context::_intervalSync);

        } else if (Context::_menuItemSelected == MENU_INPUT) {
            char str[25] = "";
            if (Context::_inputMode == SUBMENU_INPUT_MODE_DISABLED) {
                strncpy(str, "DISABLED", 25);
            } else if (Context::_inputMode == SUBMENU_INPUT_MODE_TRIGGER) {
                strncpy(str, "MODE : TRIGGER", 25);
            } else if (Context::_inputMode == SUBMENU_INPUT_MODE_TRIGGER_NODELAY) {
                strncpy(str, "MODE : TRIGGER NO DELAY", 25);
            } else if (Context::_inputMode == SUBMENU_INPUT_MODE_PASSTHROUGH) {
                strncpy(str, "MODE : PASSTHROUGH", 25);
            }
            OLED::button(2, MENU_HEIGHT + 3, OLED::WIDTH - 4, buttonHeight, str, Context::_submenuItemSelected == SUBMENU_INPUT_MODE, false);

            OLED::checkbox(2, OLED::HEIGHT - buttonHeight, OLED::WIDTH - 4, buttonHeight, "SYNC", Context::_submenuItemSelected == SUBMENU_INPUT_SYNC, Context::_submenuItemSelected == SUBMENU_INPUT_SYNC && Context::_btnOkPressed, Context::_inputSync);

        } else if (Context::_menuItemSelected == MENU_SETTINGS) {
            char str[13] = "CHANNEL :   ";
            if (Context::_syncChannel >= 10) {
                str[10] = ((Context::_syncChannel / 10) % 10) + '0';
                str[11] = (Context::_syncChannel % 10) + '0';
            } else {
                str[10] = Context::_syncChannel + '0';
            }
            OLED::button(2, MENU_HEIGHT + 3, OLED::WIDTH - 4, buttonHeight, str, Context::_submenuItemSelected == SUBMENU_SETTINGS_CHANNEL, false);
            
        } else if (Context::_menuItemSelected == MENU_ADVANCED) {
            char strFocus[23] = "FOCUS DURATION : \0\0\0\0\0";
            if (Context::_settingsFocusDurationMs >= 10000) {
                strFocus[17] = ((Context::_settingsFocusDurationMs / 10000) % 10) + '0';
                strFocus[18] = ((Context::_settingsFocusDurationMs / 1000) % 10) + '0';
                strFocus[19] = 'S';
            } else if (Context::_settingsFocusDurationMs >= 1000) {
                strFocus[17] = ((Context::_settingsFocusDurationMs / 1000) % 10) + '0';
                strFocus[18] = 'S';
            } else {
                strFocus[17] = '0';
                strFocus[18] = '.';
                strFocus[19] = ((Context::_settingsFocusDurationMs / 100) % 10) + '0';
                strFocus[20] = 'S';
            }
            OLED::button(2, MENU_HEIGHT + 3, OLED::WIDTH - 4, buttonHeight, strFocus, Context::_submenuItemSelected == SUBMENU_ADVANCED_FOCUS_DURATION, false);

            char strTrigger[25] = "TRIGGER DURATION : \0\0\0\0\0";
            if (Context::_settingsTriggerDurationMs >= 10000) {
                strTrigger[19] = ((Context::_settingsTriggerDurationMs / 10000) % 10) + '0';
                strTrigger[20] = ((Context::_settingsTriggerDurationMs / 1000) % 10) + '0';
                strTrigger[21] = 'S';
            } else if (Context::_settingsTriggerDurationMs >= 1000) {
                strTrigger[19] = ((Context::_settingsTriggerDurationMs / 1000) % 10) + '0';
                strTrigger[20] = 'S';
            } else {
                strTrigger[19] = '0';
                strTrigger[20] = '.';
                strTrigger[21] = ((Context::_settingsTriggerDurationMs / 100) % 10) + '0';
                strTrigger[22] = 'S';
            }
            OLED::button(2, MENU_HEIGHT + 3 + buttonHeight + 2, OLED::WIDTH - 4, buttonHeight, strTrigger, Context::_submenuItemSelected == SUBMENU_ADVANCED_TRIGGER_DURATION, false);

            OLED::checkbox(2, OLED::HEIGHT - buttonHeight, OLED::WIDTH - 4, buttonHeight, "SYNC", Context::_submenuItemSelected == SUBMENU_ADVANCED_SYNC, Context::_submenuItemSelected == SUBMENU_ADVANCED_SYNC && Context::_btnOkPressed, Context::_settingsSync);
        }
    }
}

bool GUI::handleButtons() {
    bool buttonPressed = false;
    int menuModified = -1;

    if (GPIO::fallingEdge(PIN_BTN_UP)) {
        if (Context::_submenuItemSelected > 0) {
            Context::_submenuItemSelected--;
            if (Context::_submenuItemSelected == 0) {
                showMenu();
            }
        }
        buttonPressed = true;

    } else if (GPIO::fallingEdge(PIN_BTN_DOWN)) {
        _tMenuChange = 0;
        if (Context::_menuItemSelected == MENU_TRIGGER && Context::_submenuItemSelected < SUBMENU_TRIGGER_SYNC) {
            Context::_submenuItemSelected++;
            if (Context::_submenuItemSelected == 1) {
                showMenu();
            }
        } else if (Context::_menuItemSelected == MENU_DELAY && Context::_submenuItemSelected < SUBMENU_DELAY_SYNC) {
            Context::_submenuItemSelected++;
            if (Context::_submenuItemSelected == 1) {
                showMenu();
            }
        } else if (Context::_menuItemSelected == MENU_INTERVAL && Context::_submenuItemSelected < SUBMENU_INTERVAL_SYNC) {
            Context::_submenuItemSelected++;
            if (Context::_submenuItemSelected == 1) {
                showMenu();
            }
        } else if (Context::_menuItemSelected == MENU_INPUT && Context::_submenuItemSelected < SUBMENU_INPUT_SYNC) {
            Context::_submenuItemSelected++;
            if (Context::_submenuItemSelected == 1) {
                showMenu();
            }
        } else if (Context::_menuItemSelected == MENU_SETTINGS && Context::_submenuItemSelected < SUBMENU_SETTINGS_CHANNEL) {
            Context::_submenuItemSelected++;
            if (Context::_submenuItemSelected == 1) {
                showMenu();
            }
        } else if (Context::_menuItemSelected == MENU_ADVANCED && Context::_submenuItemSelected < SUBMENU_ADVANCED_SYNC) {
            Context::_submenuItemSelected++;
            if (Context::_submenuItemSelected == 1) {
                showMenu();
            }
        }
        buttonPressed = true;

    } else if (GPIO::fallingEdge(PIN_BTN_LEFT)) {
        if (Context::_submenuItemSelected == 0) {
            if (Context::_menuItemSelected > 0) {
                setMenu(Context::_menuItemSelected - 1);
            } else {
                setMenu(N_MENU_ITEMS - 1);
            }
        } else {
            if (Context::_menuItemSelected == MENU_TRIGGER) {
                if (Context::_submenuItemSelected == SUBMENU_TRIGGER_FOCUS) {
                    Context::_submenuFocusHold = false;
                    menuModified = MENU_TRIGGER;
                } else if (Context::_submenuItemSelected == SUBMENU_TRIGGER_SHOOT) {
                    Context::_submenuTriggerHold = false;
                    menuModified = MENU_TRIGGER;
                }
            } else if (Context::_menuItemSelected == MENU_DELAY) {
                if (Context::_submenuItemSelected == SUBMENU_DELAY_DELAY) {
                    if (Context::_delayMs > 10000) {
                        Context::_delayMs -= 10000;
                    } else if (Context::_delayMs > 1000) {
                        Context::_delayMs -= 1000;
                    } else if (Context::_delayMs >= 100) {
                        Context::_delayMs -= 100;
                    }
                }
                menuModified = MENU_DELAY;
            } else if (Context::_menuItemSelected == MENU_INTERVAL) {
                if (Context::_submenuItemSelected == SUBMENU_INTERVAL_SHOTS) {
                    if (Context::_intervalNShots > 10) {
                        Context::_intervalNShots -= 10;
                    } else if (Context::_intervalNShots > 1) {
                        Context::_intervalNShots--;
                    }
                } else if (Context::_submenuItemSelected == SUBMENU_INTERVAL_DELAY) {
                    if (Context::_intervalDelayMs > 10000) {
                        Context::_intervalDelayMs -= 10000;
                    } else if (Context::_intervalDelayMs > 1000) {
                        Context::_intervalDelayMs -= 1000;
                    } else if (Context::_intervalDelayMs >= 100) {
                        Context::_intervalDelayMs -= 100;
                    }
                }
                menuModified = MENU_INTERVAL;
            } else if (Context::_menuItemSelected == MENU_INPUT) {
                if (Context::_submenuItemSelected == SUBMENU_INPUT_MODE) {
                    if (Context::_inputMode > 0) {
                        Context::_inputMode--;
                    } else {
                        Context::_inputMode = 3;
                    }
                }
                menuModified = MENU_INPUT;
            } else if (Context::_menuItemSelected == MENU_SETTINGS) {
                if (Context::_submenuItemSelected == SUBMENU_SETTINGS_CHANNEL) {
                    if (Context::_syncChannel > 0) {
                        Context::_syncChannel--;
                    }
                }
            } else if (Context::_menuItemSelected == MENU_ADVANCED) {
                if (Context::_submenuItemSelected == SUBMENU_ADVANCED_FOCUS_DURATION) {
                    if (Context::_settingsFocusDurationMs > 10000) {
                        Context::_settingsFocusDurationMs -= 10000;
                    } else if (Context::_settingsFocusDurationMs > 1000) {
                        Context::_settingsFocusDurationMs -= 1000;
                    } else if (Context::_settingsFocusDurationMs >= 100) {
                        Context::_settingsFocusDurationMs -= 100;
                    }
                } else if (Context::_submenuItemSelected == SUBMENU_ADVANCED_TRIGGER_DURATION) {
                    if (Context::_settingsTriggerDurationMs > 10000) {
                        Context::_settingsTriggerDurationMs -= 10000;
                    } else if (Context::_settingsTriggerDurationMs > 1000) {
                        Context::_settingsTriggerDurationMs -= 1000;
                    } else if (Context::_settingsTriggerDurationMs > 100) {
                        Context::_settingsTriggerDurationMs -= 100;
                    }
                }
                menuModified = MENU_ADVANCED;
            }
        }
        buttonPressed = true;

    } else if (GPIO::fallingEdge(PIN_BTN_RIGHT)) {
        if (Context::_submenuItemSelected == 0) {
            if (Context::_menuItemSelected < N_MENU_ITEMS - 1) {
                setMenu(Context::_menuItemSelected + 1);
            } else {
                setMenu(0);
            }
        } else {
            if (Context::_menuItemSelected == MENU_TRIGGER) {
                if (Context::_submenuItemSelected == SUBMENU_TRIGGER_FOCUS && Context::_tFocus == 0) {
                    Context::_submenuFocusHold = true;
                    menuModified = MENU_TRIGGER;
                } else if (Context::_submenuItemSelected == SUBMENU_TRIGGER_SHOOT && Context::_tTrigger == 0) {
                    Context::_submenuTriggerHold = true;
                    menuModified = MENU_TRIGGER;
                }
            } else if (Context::_menuItemSelected == MENU_DELAY) {
                if (Context::_submenuItemSelected == SUBMENU_DELAY_DELAY) {
                    if (Context::_delayMs < 1000) {
                        Context::_delayMs += 100;
                    } else if (Context::_delayMs < 10000) {
                        Context::_delayMs += 1000;
                    } else if (Context::_delayMs < 90000) {
                        Context::_delayMs += 10000;
                    }
                }
                menuModified = MENU_DELAY;
            } else if (Context::_menuItemSelected == MENU_INTERVAL) {
                if (Context::_submenuItemSelected == SUBMENU_INTERVAL_SHOTS) {
                    if (Context::_intervalNShots < 10) {
                        Context::_intervalNShots++;
                    } else if (Context::_intervalNShots < 99) {
                        Context::_intervalNShots += 10;
                    }
                } else if (Context::_submenuItemSelected == SUBMENU_INTERVAL_DELAY) {
                    if (Context::_intervalDelayMs < 1000) {
                        Context::_intervalDelayMs += 100;
                    } else if (Context::_intervalDelayMs < 10000) {
                        Context::_intervalDelayMs += 1000;
                    } else if (Context::_intervalDelayMs < 90000) {
                        Context::_intervalDelayMs += 10000;
                    }
                }
                menuModified = MENU_INTERVAL;
            } else if (Context::_menuItemSelected == MENU_INPUT) {
                if (Context::_submenuItemSelected == SUBMENU_INPUT_MODE) {
                    if (Context::_inputMode < 3) {
                        Context::_inputMode++;
                    } else {
                        Context::_inputMode = 0;
                    }
                }
                menuModified = MENU_INPUT;
            } else if (Context::_menuItemSelected == MENU_SETTINGS) {
                if (Context::_submenuItemSelected == SUBMENU_SETTINGS_CHANNEL) {
                    if (Context::_syncChannel < Sync::N_CHANNELS) {
                        Context::_syncChannel++;
                    }
                }
            } else if (Context::_menuItemSelected == MENU_ADVANCED) {
                if (Context::_submenuItemSelected == SUBMENU_ADVANCED_FOCUS_DURATION) {
                    if (Context::_settingsFocusDurationMs < 1000) {
                        Context::_settingsFocusDurationMs += 100;
                    } else if (Context::_settingsFocusDurationMs < 10000) {
                        Context::_settingsFocusDurationMs += 1000;
                    } else if (Context::_settingsFocusDurationMs < 90000) {
                        Context::_settingsFocusDurationMs += 10000;
                    }
                } else if (Context::_submenuItemSelected == SUBMENU_ADVANCED_TRIGGER_DURATION) {
                    if (Context::_settingsTriggerDurationMs < 1000) {
                        Context::_settingsTriggerDurationMs += 100;
                    } else if (Context::_settingsTriggerDurationMs < 10000) {
                        Context::_settingsTriggerDurationMs += 1000;
                    } else if (Context::_settingsTriggerDurationMs < 90000) {
                        Context::_settingsTriggerDurationMs += 10000;
                    }
                }
                menuModified = MENU_ADVANCED;
            }
        }
        buttonPressed = true;

    } else if ((!Context::_btnOkPressed && GPIO::fallingEdge(PIN_BTN_OK)) || (Context::_btnOkPressed && GPIO::risingEdge(PIN_BTN_OK))) {
        Context::_btnOkPressed = !GPIO::get(PIN_BTN_OK);

        if (Context::_menuItemSelected == MENU_TRIGGER) {
            if (Context::_submenuItemSelected == SUBMENU_TRIGGER_FOCUS && !Context::_submenuFocusHold) {
                if (Context::_btnOkPressed) {
                    if (Context::_tFocus == 0) {
                        // Start
                        Context::_tFocus = Core::time();
                    } else {
                        // Stop
                        Context::_tFocus = 0;
                    }
                }
            } else if (Context::_submenuItemSelected == SUBMENU_TRIGGER_SHOOT && !Context::_submenuTriggerHold) {
                if (Context::_btnOkPressed) {
                    if (Context::_tTrigger == 0) {
                        // Start
                        Context::_tTrigger = Core::time();
                        Context::_skipDelay = false;
                        Sync::send(Sync::CMD_TRIGGER);
                    } else {
                        // Stop
                        Context::_tTrigger = 0;
                        Context::_skipDelay = false;
                        Sync::send(Sync::CMD_TRIGGER_RELEASE);
                    }
                }
            } else if (Context::_submenuItemSelected == SUBMENU_TRIGGER_SYNC) {
                if (Context::_btnOkPressed) {
                    Context::_triggerSync = !Context::_triggerSync;
                }
            }
        } else if (Context::_menuItemSelected == MENU_DELAY) {
            if (Context::_submenuItemSelected == SUBMENU_DELAY_SYNC) {
                if (Context::_btnOkPressed) {
                    Context::_delaySync = !Context::_delaySync;
                    menuModified = MENU_DELAY;
                }
            }
        } else if (Context::_menuItemSelected == MENU_INTERVAL) {
            if (Context::_submenuItemSelected == SUBMENU_INTERVAL_SYNC) {
                if (Context::_btnOkPressed) {
                    Context::_intervalSync = !Context::_intervalSync;
                    menuModified = MENU_INTERVAL;
                }
            }
        } else if (Context::_menuItemSelected == MENU_INPUT) {
            if (Context::_submenuItemSelected == SUBMENU_INPUT_SYNC) {
                if (Context::_btnOkPressed) {
                    Context::_inputSync = !Context::_inputSync;
                    menuModified = MENU_INPUT;
                }
            }
        } else if (Context::_menuItemSelected == MENU_SETTINGS) {
        } else if (Context::_menuItemSelected == MENU_ADVANCED) {
            if (Context::_submenuItemSelected == SUBMENU_ADVANCED_SYNC) {
                if (Context::_btnOkPressed) {
                    Context::_settingsSync = !Context::_settingsSync;
                    menuModified = MENU_ADVANCED;
                }
            }
        }

        buttonPressed = true;
    }

    // Sync
    if (menuModified > -1) {
        uint8_t payload[Sync::MAX_PAYLOAD_SIZE];
        int payloadSize = 0;
        if (menuModified == MENU_TRIGGER && Context::_triggerSync) {
            payload[0] = Context::_submenuFocusHold;
            payload[1] = Context::_submenuTriggerHold;
            payloadSize = 2;
        } else if (menuModified == MENU_DELAY && Context::_delaySync) {
            payload[0] = ((Context::_delayMs / 100) >> 8) & 0xFF;
            payload[1] = (Context::_delayMs / 100) & 0xFF;
            payloadSize = 2;
        } else if (menuModified == MENU_INTERVAL && Context::_intervalSync) {
            payload[0] = Context::_intervalNShots & 0xFF;
            payload[1] = ((Context::_intervalDelayMs / 100) >> 8) & 0xFF;
            payload[2] = (Context::_intervalDelayMs / 100) & 0xFF;
            payloadSize = 3;
        } else if (menuModified == MENU_INPUT && Context::_inputSync) {
            payload[0] = Context::_inputMode & 0xFF;
            payloadSize = 1;
        } else if (menuModified == MENU_ADVANCED &&  Context::_settingsSync) {
            payload[0] = ((Context::_settingsFocusDurationMs / 100) >> 8) & 0xFF;
            payload[1] = (Context::_settingsFocusDurationMs / 100) & 0xFF;
            payload[2] = ((Context::_settingsTriggerDurationMs / 100) >> 8) & 0xFF;
            payload[3] = (Context::_settingsTriggerDurationMs / 100) & 0xFF;
            payloadSize = 4;
        }
        Sync::send(menuModified, payload, payloadSize);
    }

    return buttonPressed;
}

void GUI::update(bool refresh, bool trigger, bool focus, bool waiting, bool input) {
    // Hide menu label after timeout
    if (_tMenuChange > 0 && Core::time() >= _tMenuChange + DELAY_MENU_LABEL) {
        refresh = true;
    }

    // Update menu display
    if (refresh) {
        GUI::showMenuContent();
    }

    // Show infos on the bottom right
    if (trigger) { // Trigger status
        Font::Char8 c = ICON_TRIGGER;
        OLED::printMedium(OLED::WIDTH - c.width, OLED::HEIGHT - c.height, c);
    } else if (focus) {
        Font::Char8 c = ICON_FOCUS;
        OLED::printMedium(OLED::WIDTH - c.width, OLED::HEIGHT - c.height, c);
    } else if (waiting) {
        Font::Char8 c = ICON_DELAY;
        OLED::printMedium(OLED::WIDTH - c.width, OLED::HEIGHT - c.height, c);
    }
    if (input) { // Input
        Font::Char8 c = ICON_INPUT;
        OLED::printMedium(OLED::WIDTH - ICON_TRIGGER.width - 2 - c.width, OLED::HEIGHT - c.height, c);
    }

    // Update screen
    if (refresh) {
        OLED::refresh();
    }
}