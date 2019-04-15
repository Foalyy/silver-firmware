#include "gui.h"
#include "sync.h"
#include "sync_usb.h"
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
        0b11101000,
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
const Font::Char32 ICON_SILICA_XL = {
    32,
    32,
    {
        0b00000000000000000000000000000000,
        0b00000000000000000000000000000000,
        0b00000000000000000000000011100000,
        0b00000000000000000011111111100000,
        0b00000000000000000011111111010000,
        0b00000000000000000111111110010000,
        0b00000000000000001111111100010000,
        0b00000000000000011111111000010000,
        0b00000000110000011111110000010000,
        0b00000001101000111111100000010000,
        0b00000011101001111111100000010000,
        0b00000111100111111111000000010000,
        0b00001111100011111110000000100000,
        0b00001111100011111100000001000000,
        0b00001111000011111000000010000000,
        0b00001111000011111000000010000000,
        0b00001111000111110000000100000000,
        0b00001111000111100000001000000000,
        0b00001111000111000000010000000000,
        0b00001111000110000000111110000000,
        0b00001111000110000000111111100000,
        0b00001111000100000001111111110000,
        0b00001111000100000011111111010000,
        0b00001111000100000110000000100000,
        0b00001111001000001000000000100000,
        0b00000111010000001000000001000000,
        0b00000111100000010000111110000000,
        0b00000111000000111111000000000000,
        0b00000110001111100000000000000000,
        0b00000011110000000000000000000000,
        0b00000000000000000000000000000000,
        0b00000000000000000000000000000000,
    }
};
const Font::Char64 ICON_SILICA_XXL = {
    64,
    64,
    {
        0b0000000000000000000000000000000000000000000000000000000000000000,
        0b0000000000000000000000000000000000000000000000000000000000000000,
        0b0000000000000000000000000000000000000000000000000000000000000000,
        0b0000000000000000000000000000000000000000000000000000000000000000,
        0b0000000000000000000000000000000000000000000000000000000000000000,
        0b0000000000000000000000000000000000000000000000000000000000000000,
        0b0000000000000000000000000000000000000000000000000000000000000000,
        0b0000000000000000000000000000000000000000000000000111111111100000,
        0b0000000000000000000000000000000000000000000111111111111111100000,
        0b0000000000000000000000000000000000000000001111111111111110100000,
        0b0000000000000000000000000000000000000000011101111111111100110000,
        0b0000000000000000000000000000000000000000011110111111111000010000,
        0b0000000000000000000000000000000000000000111110111111110000010000,
        0b0000000000000000000000000000000000000001111111011111100000010000,
        0b0000000000000000000000000000000000000011111111011111000000010000,
        0b0000000000000000000000000000000000000011111111101110000000010000,
        0b0000000000000000000000000000000000000111111111110110000000010000,
        0b0000000000000000000000001000000000001111111111111100000000010000,
        0b0000000000000000000000011100000000011111111111111011000000011000,
        0b0000000000000000000000111010000000111111111111110000111000001000,
        0b0000000000000000000011111011000000111111111111100000000110001000,
        0b0000000000000000000111111001100001111111111111000000000001111000,
        0b0000000000000000001111111000110011111111111111000000000000011000,
        0b0000000000000000011111110000011111111111111110000000000000110000,
        0b0000000000000000010011110000001111111111111100000000000001100000,
        0b0000000000000000011100110001101111111111111000000000000001000000,
        0b0000000000000000011111010110001111111111110000000000000011000000,
        0b0000000000000000011111111000001111111111110000000000000110000000,
        0b0000000000000000111111110000001111111111100000000000001100000000,
        0b0000000000000000111111110000001111111111000000000000001000000000,
        0b0000000000000000111111110000001111111110000000000000010000000000,
        0b0000000000000000111111100000001111111110000000000000110000000000,
        0b0000000000000000111111100000001111111100000000000001100000000000,
        0b0000000000000000111111100000001111111000000000000011000000000000,
        0b0000000000000000111111100000011111110000000000000010000000000000,
        0b0000000000000000111111100000011111100000000000000100000000000000,
        0b0000000000000000111111100000011111100000000000001100000000000000,
        0b0000000000000000111111100000011111000000000000011111100000000000,
        0b0000000000000000111111100000011110000000000000111110111000000000,
        0b0000000000000001111111100000011100000000000000111110111100000000,
        0b0000000000000001111111100000011000000000000001111110111111000000,
        0b0000000000000001111111100000011000000000000011111110111111100000,
        0b0000000000000001111111100000010000000000000111111110111111110000,
        0b0000000000000001111111000000010000000000000111111111111111111000,
        0b0000000000000001111111000000010000000000001111111111000000001000,
        0b0000000000000001111111000000010000000000011111000000100000010000,
        0b0000000000000001111111000000110000000000110000000000100000100000,
        0b0000000000000000100111000000110000000001100000000000100001000000,
        0b0000000000000000011001111101100000000001000000000000010010000000,
        0b0000000000000000011111000011000000000010000000000000010100000000,
        0b0000000000000000001111000100000000000110000000000000001000000000,
        0b0000000000000000001111011000000000001100000000000111110000000000,
        0b0000000000000000001111110111000000011000000111111100000000000000,
        0b0000000000000000001111100000111000011111111100000000000000000000,
        0b0000000000000000001110000000000110111100000000000000000000000000,
        0b0000000000000000001110000001111111100000000000000000000000000000,
        0b0000000000000000001111111111100000000000000000000000000000000000,
        0b0000000000000000000000000000000000000000000000000000000000000000,
        0b0000000000000000000000000000000000000000000000000000000000000000,
        0b0000000000000000000000000000000000000000000000000000000000000000,
        0b0000000000000000000000000000000000000000000000000000000000000000,
        0b0000000000000000000000000000000000000000000000000000000000000000,
        0b0000000000000000000000000000000000000000000000000000000000000000,
        0b0000000000000000000000000000000000000000000000000000000000000000,
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
Core::Time _tGUIInit = 0;
const int DELAY_LOGO_INIT = 2000;


void GUI::init() {
    // Init the screen
    OLED::initScreen(0, PIN_OLED_DC, PIN_OLED_RES);
    OLED::setRotation(OLED::Rotation::R180);
    OLED::printXXLarge((OLED::WIDTH - 64) / 2, (OLED::HEIGHT - 64) / 2, ICON_SILICA_XXL);
    OLED::setSize(Font::Size::MEDIUM);
    OLED::printCentered(OLED::WIDTH / 2, 54, "silica.io");
    OLED::refresh();
    _tGUIInit = Core::time();
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
                    OLED::button(2, MENU_HEIGHT + 3, OLED::WIDTH - 4, buttonHeight, "Focus", Context::_submenuItemSelected == SUBMENU_TRIGGER_FOCUS, Context::_submenuItemSelected == SUBMENU_TRIGGER_FOCUS && Context::_btnOkPressed, false, true);
                } else {
                    OLED::button(2, MENU_HEIGHT + 3, OLED::WIDTH - 4, buttonHeight, "Focus Hold", Context::_submenuItemSelected == SUBMENU_TRIGGER_FOCUS, Context::_submenuItemSelected == SUBMENU_TRIGGER_FOCUS && Context::_btnOkPressed, true, false);
                }
            } else {
                OLED::button(2, MENU_HEIGHT + 3, OLED::WIDTH - 4, buttonHeight, "Stop", Context::_submenuItemSelected == SUBMENU_TRIGGER_FOCUS, Context::_submenuItemSelected == SUBMENU_TRIGGER_FOCUS && Context::_btnOkPressed);
            }
            if (Context::_tTrigger == 0) {
                if (!Context::_submenuTriggerHold) {
                    OLED::button(2, MENU_HEIGHT + 3 + buttonHeight + 2, OLED::WIDTH - 4, buttonHeight, "Trigger", Context::_submenuItemSelected == SUBMENU_TRIGGER_SHOOT, Context::_submenuItemSelected == SUBMENU_TRIGGER_SHOOT && Context::_btnOkPressed, false, true);
                } else {
                    OLED::button(2, MENU_HEIGHT + 3 + buttonHeight + 2, OLED::WIDTH - 4, buttonHeight, "Trigger Hold", Context::_submenuItemSelected == SUBMENU_TRIGGER_SHOOT, Context::_submenuItemSelected == SUBMENU_TRIGGER_SHOOT && Context::_btnOkPressed, true, false);
                }
            } else {
                OLED::button(2, MENU_HEIGHT + 3 + buttonHeight + 2, OLED::WIDTH - 4, buttonHeight, "Stop", Context::_submenuItemSelected == SUBMENU_TRIGGER_SHOOT, Context::_submenuItemSelected == SUBMENU_TRIGGER_SHOOT && Context::_btnOkPressed);
            }
            OLED::checkbox(2, OLED::HEIGHT - buttonHeight, OLED::WIDTH - 4, buttonHeight, "Sync", Context::_submenuItemSelected == SUBMENU_TRIGGER_SYNC, Context::_submenuItemSelected == SUBMENU_TRIGGER_SYNC && Context::_btnOkPressed, Context::_triggerSync);

        } else if (Context::_menuItemSelected == MENU_DELAY) {
            char strDelay[14] = "Delay : \0\0\0\0\0";
            if (Context::_delayMs >= 10000) {
                strDelay[8] = ((Context::_delayMs / 10000) % 10) + '0';
                strDelay[9] = ((Context::_delayMs / 1000) % 10) + '0';
                strDelay[10] = 's';
            } else if (Context::_delayMs >= 1000) {
                strDelay[8] = ((Context::_delayMs / 1000) % 10) + '0';
                strDelay[9] = 's';
            } else {
                strDelay[8] = '0';
                strDelay[9] = '.';
                strDelay[10] = ((Context::_delayMs / 100) % 10) + '0';
                strDelay[11] = 's';
            }
            OLED::button(2, MENU_HEIGHT + 3, OLED::WIDTH - 4, buttonHeight, strDelay, Context::_submenuItemSelected == SUBMENU_DELAY_DELAY, false);
            OLED::checkbox(2, OLED::HEIGHT - buttonHeight, OLED::WIDTH - 4, buttonHeight, "Sync", Context::_submenuItemSelected == SUBMENU_DELAY_SYNC, Context::_submenuItemSelected == SUBMENU_DELAY_SYNC && Context::_btnOkPressed, Context::_delaySync);

        } else if (Context::_menuItemSelected == MENU_INTERVAL) {
            char strShots[12] = "Shots : \0\0\0";
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

            char strDelay[17] = "Interval : \0\0\0\0\0";
            if (Context::_intervalDelayMs >= 10000) {
                strDelay[11] = ((Context::_intervalDelayMs / 10000) % 10) + '0';
                strDelay[12] = ((Context::_intervalDelayMs / 1000) % 10) + '0';
                strDelay[13] = 's';
            } else if (Context::_intervalDelayMs >= 1000) {
                strDelay[11] = ((Context::_intervalDelayMs / 1000) % 10) + '0';
                strDelay[12] = 's';
            } else {
                strDelay[11] = '0';
                strDelay[12] = '.';
                strDelay[13] = ((Context::_intervalDelayMs / 100) % 10) + '0';
                strDelay[14] = 's';
            }
            OLED::button(2, MENU_HEIGHT + 3 + buttonHeight + 2, OLED::WIDTH - 4, buttonHeight, strDelay, Context::_submenuItemSelected == SUBMENU_INTERVAL_DELAY, false);

            OLED::checkbox(2, OLED::HEIGHT - buttonHeight, OLED::WIDTH - 4, buttonHeight, "Sync", Context::_submenuItemSelected == SUBMENU_INTERVAL_SYNC, Context::_submenuItemSelected == SUBMENU_INTERVAL_SYNC && Context::_btnOkPressed, Context::_intervalSync);

        } else if (Context::_menuItemSelected == MENU_INPUT) {
            char str[26] = "";
            if (Context::_inputMode == SUBMENU_INPUT_MODE_DISABLED) {
                strncpy(str, "Disabled", 26);
            } else if (Context::_inputMode == SUBMENU_INPUT_MODE_TRIGGER) {
                strncpy(str, "Mode : trigger", 26);
            } else if (Context::_inputMode == SUBMENU_INPUT_MODE_TRIGGER_NODELAY) {
                strncpy(str, "Mode : trigger (no delay)", 26);
            } else if (Context::_inputMode == SUBMENU_INPUT_MODE_PASSTHROUGH) {
                strncpy(str, "Mode : passthrough", 26);
            }
            OLED::button(2, MENU_HEIGHT + 3, OLED::WIDTH - 4, buttonHeight, str, Context::_submenuItemSelected == SUBMENU_INPUT_MODE, false);

            OLED::checkbox(2, OLED::HEIGHT - buttonHeight, OLED::WIDTH - 4, buttonHeight, "Sync", Context::_submenuItemSelected == SUBMENU_INPUT_SYNC, Context::_submenuItemSelected == SUBMENU_INPUT_SYNC && Context::_btnOkPressed, Context::_inputSync);

        } else if (Context::_menuItemSelected == MENU_SETTINGS) {
            char strChannel[13] = "Channel :   ";
            if (Context::_syncChannel >= 10) {
                strChannel[10] = ((Context::_syncChannel / 10) % 10) + '0';
                strChannel[11] = (Context::_syncChannel % 10) + '0';
            } else {
                strChannel[10] = Context::_syncChannel + '0';
            }
            OLED::button(2, MENU_HEIGHT + 3, OLED::WIDTH - 4, buttonHeight, strChannel, Context::_submenuItemSelected == SUBMENU_SETTINGS_CHANNEL, false);

            char strBrightness[16] = "Brightness :   ";
            if (Context::_brightness == 10) {
                strBrightness[13] = '1';
                strBrightness[14] = '0';
            } else {
                strBrightness[14] = Context::_brightness + '0';
            }
            OLED::button(2, MENU_HEIGHT + 3 + buttonHeight + 2, OLED::WIDTH - 4, buttonHeight, strBrightness, Context::_submenuItemSelected == SUBMENU_SETTINGS_BRIGHTNESS, false);
            
        } else if (Context::_menuItemSelected == MENU_ADVANCED) {
            char strFocus[23] = "Focus duration : \0\0\0\0\0";
            if (Context::_settingsFocusDurationMs >= 10000) {
                strFocus[17] = ((Context::_settingsFocusDurationMs / 10000) % 10) + '0';
                strFocus[18] = ((Context::_settingsFocusDurationMs / 1000) % 10) + '0';
                strFocus[19] = 's';
            } else if (Context::_settingsFocusDurationMs >= 1000) {
                strFocus[17] = ((Context::_settingsFocusDurationMs / 1000) % 10) + '0';
                strFocus[18] = 's';
            } else {
                strFocus[17] = '0';
                strFocus[18] = '.';
                strFocus[19] = ((Context::_settingsFocusDurationMs / 100) % 10) + '0';
                strFocus[20] = 's';
            }
            OLED::button(2, MENU_HEIGHT + 3, OLED::WIDTH - 4, buttonHeight, strFocus, Context::_submenuItemSelected == SUBMENU_ADVANCED_FOCUS_DURATION, false);

            char strTrigger[25] = "Trigger duration : \0\0\0\0\0";
            if (Context::_settingsTriggerDurationMs >= 10000) {
                strTrigger[19] = ((Context::_settingsTriggerDurationMs / 10000) % 10) + '0';
                strTrigger[20] = ((Context::_settingsTriggerDurationMs / 1000) % 10) + '0';
                strTrigger[21] = 's';
            } else if (Context::_settingsTriggerDurationMs >= 1000) {
                strTrigger[19] = ((Context::_settingsTriggerDurationMs / 1000) % 10) + '0';
                strTrigger[20] = 's';
            } else {
                strTrigger[19] = '0';
                strTrigger[20] = '.';
                strTrigger[21] = ((Context::_settingsTriggerDurationMs / 100) % 10) + '0';
                strTrigger[22] = 's';
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
        } else if (Context::_menuItemSelected == MENU_DELAY && Context::_submenuItemSelected < SUBMENU_DELAY_SYNC) {
            Context::_submenuItemSelected++;
        } else if (Context::_menuItemSelected == MENU_INTERVAL && Context::_submenuItemSelected < SUBMENU_INTERVAL_SYNC) {
            Context::_submenuItemSelected++;
        } else if (Context::_menuItemSelected == MENU_INPUT && Context::_submenuItemSelected < SUBMENU_INPUT_SYNC) {
            Context::_submenuItemSelected++;
        } else if (Context::_menuItemSelected == MENU_SETTINGS && Context::_submenuItemSelected < SUBMENU_SETTINGS_BRIGHTNESS) {
            Context::_submenuItemSelected++;
        } else if (Context::_menuItemSelected == MENU_ADVANCED && Context::_submenuItemSelected < SUBMENU_ADVANCED_SYNC) {
            Context::_submenuItemSelected++;
        }
        if (Context::_submenuItemSelected == 1) {
            showMenu();
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
                } else if (Context::_submenuItemSelected == SUBMENU_SETTINGS_BRIGHTNESS) {
                    if (Context::_brightness > 0) {
                        Context::_brightness--;
                        OLED::setContrast(Context::_brightness * 25);
                    }
                }
                menuModified = MENU_SETTINGS;
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
                } else if (Context::_submenuItemSelected == SUBMENU_SETTINGS_BRIGHTNESS) {
                    if (Context::_brightness < 10) {
                        Context::_brightness++;
                        OLED::setContrast(Context::_brightness * 25);
                    }
                }
                menuModified = MENU_SETTINGS;
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
            } else if (Context::_submenuItemSelected == SUBMENU_TRIGGER_SHOOT && !Context::_submenuTriggerHold) {
                if (Context::_btnOkPressed) {
                    if (Context::_tTrigger == 0) {
                        // Start
                        Context::_tTrigger = Core::time();
                        Context::_skipDelay = false;
                        if (Context::_triggerSync) {
                            Sync::send(Sync::CMD_TRIGGER);
                        }
                    } else {
                        // Stop
                        Context::_tTrigger = 0;
                        Context::_skipDelay = false;
                        if (Context::_triggerSync) {
                            Sync::send(Sync::CMD_TRIGGER_RELEASE);
                        }
                    }
                }
            } else if (Context::_submenuItemSelected == SUBMENU_TRIGGER_SYNC) {
                if (Context::_btnOkPressed) {
                    Context::_triggerSync = !Context::_triggerSync;
                    menuModified = MENU_TRIGGER;
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
        int payloadSizeUSB = 0;
        bool sendSync = false;
        bool sendUSB = SyncUSB::isConnected();
        if (menuModified == MENU_TRIGGER) {
            payload[0] = Context::_submenuFocusHold;
            payload[1] = Context::_submenuTriggerHold;
            payload[2] = Context::_triggerSync;
            payloadSize = 2;
            payloadSizeUSB = 3;
            if (Context::_triggerSync) {
                sendSync = true;
            }
        } else if (menuModified == MENU_DELAY) {
            payload[0] = ((Context::_delayMs / 100) >> 16) & 0xFF;
            payload[1] = ((Context::_delayMs / 100) >> 8) & 0xFF;
            payload[2] = (Context::_delayMs / 100) & 0xFF;
            payload[3] = Context::_delaySync;
            payloadSize = 3;
            payloadSizeUSB = 4;
            if (Context::_delaySync) {
                sendSync = true;
            }
        } else if (menuModified == MENU_INTERVAL) {
            payload[0] = Context::_intervalNShots & 0xFF;
            payload[1] = ((Context::_intervalDelayMs / 100) >> 16) & 0xFF;
            payload[2] = ((Context::_intervalDelayMs / 100) >> 8) & 0xFF;
            payload[3] = (Context::_intervalDelayMs / 100) & 0xFF;
            payload[4] = Context::_intervalSync;
            payloadSize = 4;
            payloadSizeUSB = 5;
            if (Context::_intervalSync) {
                sendSync = true;
            }
        } else if (menuModified == MENU_INPUT) {
            payload[0] = Context::_inputMode & 0xFF;
            payload[1] = Context::_inputSync;
            payloadSize = 1;
            payloadSizeUSB = 2;
            if (Context::_inputSync) {
                sendSync = true;
            }
        } else if (menuModified == MENU_SETTINGS) {
            payload[0] = Context::_syncChannel;
            payloadSizeUSB = 1;
        } else if (menuModified == MENU_ADVANCED) {
            payload[0] = ((Context::_settingsFocusDurationMs / 100) >> 16) & 0xFF;
            payload[1] = ((Context::_settingsFocusDurationMs / 100) >> 8) & 0xFF;
            payload[2] = (Context::_settingsFocusDurationMs / 100) & 0xFF;
            payload[3] = ((Context::_settingsTriggerDurationMs / 100) >> 16) & 0xFF;
            payload[4] = ((Context::_settingsTriggerDurationMs / 100) >> 8) & 0xFF;
            payload[5] = (Context::_settingsTriggerDurationMs / 100) & 0xFF;
            payload[6] = Context::_settingsSync;
            payloadSize = 6;
            payloadSizeUSB = 7;
            if (Context::_settingsSync) {
                sendSync = true;
            }
        }
        if (sendSync) {
            Sync::send(menuModified, payload, payloadSize);
        }
        if (sendUSB) {
            SyncUSB::send(menuModified, payload, payloadSizeUSB);
        }
    }

    return buttonPressed;
}

void GUI::update(bool refresh, bool trigger, bool focus, bool waiting, bool input) {
    // Hide init logo after timeout
    if (_tGUIInit > 0 && Core::time() >= _tGUIInit + DELAY_LOGO_INIT) {
        GUI::showMenu();
        _tGUIInit = 0;
        refresh = true;
    }
    
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