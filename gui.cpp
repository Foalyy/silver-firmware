#include "gui.h"
#include "sync.h"
#include "sync_usb.h"
#include "pins.h"
#include "icons.h"
#include "drivers/oled_ssd1306/oled.h"
#include "drivers/oled_ssd1306/font.h"
#include <string.h>


const Font::Char8 MENU_ICONS[GUI::N_MENU_ITEMS] = {
    ICON_TRIGGER,
    ICON_DELAY,
    ICON_INTVL,
    ICON_TIMINGS,
    ICON_INPUT,
    ICON_SETTINGS,
};
const char MENU_LABELS[GUI::N_MENU_ITEMS][10] = {
    "TRIGGER",
    "DELAY",
    "INTERVAL",
    "TIMINGS",
    "INPUT",
    "SETTINGS",
};



const int MENU_HEIGHT = 11;
const int FOOTER_HEIGHT = 11;
Core::Time _tMenuChange = 0;
const int DELAY_MENU_LABEL = 500;
Core::Time _tGUIInit = 0;
const int DELAY_LOGO_INIT = 1500;

const int N_BRIGHTNESS_LEVELS = 4;
const int _brightnessValues[N_BRIGHTNESS_LEVELS] = {0, 10, 20, 255};


void GUI::init() {
    // Init the screen
    SPI::setPin(static_cast<SPI::PinFunction>(static_cast<int>(SPI::PinFunction::CS0) + static_cast<int>(SPI_SLAVE_OLED)), PIN_OLED_CS);
    OLED::initScreen(SPI_SLAVE_OLED, PIN_OLED_DC, PIN_OLED_RES);
    OLED::setRotation(OLED::Rotation::R180);
    OLED::printXXLarge((OLED::WIDTH - 64) / 2, (OLED::HEIGHT - 64) / 2, ICON_SILICA_XXL);
    OLED::setSize(Font::Size::MEDIUM);
    OLED::printCentered(OLED::WIDTH / 2, 54, "SILVER");
    OLED::refresh();
    _tGUIInit = Core::time();
}

void GUI::showMenu() {
    // Clear the whole screen except the footer
    OLED::clear(0, 0, OLED::WIDTH, OLED::HEIGHT - FOOTER_HEIGHT - 1);

    // Print tabs
    const int tabCenterX = (Context::_menuItemSelected + 1) * OLED::WIDTH / (N_MENU_ITEMS + 1);
    for (int x = 0; x < OLED::WIDTH; x++) {
        if (x > tabCenterX - 8 && x < tabCenterX + 7) {
            if (Context::_submenuItemSelected == 0) {
                for (int y = 0; y <= MENU_HEIGHT; y++) {
                    OLED::setPixel(x, y);
                }
            } else {
                OLED::setPixel(x, 0);
            }
        } else if (x == tabCenterX - 8 || x == tabCenterX + 7) {
            if (Context::_submenuItemSelected == 0) {
                for (int y = 1; y <= MENU_HEIGHT; y++) {
                    OLED::setPixel(x, y);
                }
            } else {
                OLED::setPixel(x, 1);
            }
        } else if (x == tabCenterX - 9 || x == tabCenterX + 8) {
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
        OLED::printMedium((i + 1) * OLED::WIDTH / (N_MENU_ITEMS + 1) - c.width / 2, 1 + (i == Context::_menuItemSelected ? 1 + (Context::_submenuItemSelected > 0 ? 1 : 0) : 0), c);
    }
}

void GUI::setMenu(int menuItemSelected) {
    Context::_menuItemSelected = menuItemSelected;

    showMenu();

    _tMenuChange = Core::time();
}

void GUI::showFooter(bool trigger, bool triggerHold, bool focus, bool focusHold, bool waiting, bool input) {
    // Clear the footer area
    OLED::clear(0, OLED::HEIGHT - FOOTER_HEIGHT, OLED::WIDTH, FOOTER_HEIGHT);
    OLED::rect(0, OLED::HEIGHT - FOOTER_HEIGHT, OLED::WIDTH, 1);
    OLED::setInverted(false);
    OLED::setSize(Font::Size::MEDIUM);

    // Display battery indicator
    //OLED::printInt(0, OLED::HEIGHT - FOOTER_HEIGHT + 2, Context::_vBat);
    const int BAT_X = 0;
    const int BAT_Y = OLED::HEIGHT - FOOTER_HEIGHT + 3;
    const int BAT_N_BARS = 3;
    const int BAT_BAR_WIDTH = 2;
    const int BAT_WIDTH = 2 + BAT_N_BARS * BAT_BAR_WIDTH + (BAT_N_BARS - 1) + 2 + 1;
    const int BAT_HEIGHT = 7;
    const int BAT_LEVELS[BAT_N_BARS] = {3300, 3700, 4000};
    for (int x = 0; x < BAT_WIDTH; x++) {
        if (x == 0 || x == BAT_WIDTH - 2) {
            for (int y = 1; y < BAT_HEIGHT - 1; y++) {
                OLED::setPixel(BAT_X + x, BAT_Y + y);
            }
        } else if (x == BAT_WIDTH - 1) {
            for (int y = 2; y < BAT_HEIGHT - 2; y++) {
                OLED::setPixel(BAT_X + x, BAT_Y + y);
            }
        } else {
            OLED::setPixel(BAT_X + x, BAT_Y);
            OLED::setPixel(BAT_X + x, BAT_Y + BAT_HEIGHT - 1);
        }
    }
    for (int i = 0; i < BAT_N_BARS; i++) {
        if (Context::_vBat >= BAT_LEVELS[i]) {
            for (int x = 0; x < BAT_BAR_WIDTH; x++) {
                for (int y = 2; y < BAT_HEIGHT - 2; y++) {
                    OLED::setPixel(BAT_X + 2 + i * BAT_BAR_WIDTH + i + x, BAT_Y + y);
                }
            }
        } else {
            break;
        }
    }

    // RSSI indicator
    bool showRSSI = false;
    if (Context::_tReceivedCommand > 0) {
        showRSSI = true;
        //OLED::printInt(30, OLED::HEIGHT - 8, Context::_rssi + 137);
        if (Context::_rssi >= Context::RSSI_HIGH) {
            Font::Char8 c = ICON_RSSI_HIGH;
            OLED::printMedium(15, OLED::HEIGHT - c.height, c);
        } else if (Context::_rssi >= Context::RSSI_MID) {
            Font::Char8 c = ICON_RSSI_MID;
            OLED::printMedium(15, OLED::HEIGHT - c.height, c);
        } else {
            Font::Char8 c = ICON_RSSI_LOW;
            OLED::printMedium(15, OLED::HEIGHT - c.height, c);
        }
    }

    // Shots left and countdown
    if (trigger || focus || waiting) {
        if (Context::_tTrigger > 0) {
            OLED::printInt(40, OLED::HEIGHT - 8, Context::_shotsLeft);
            OLED::print("/");
            OLED::printInt(Context::_shadowIntervalNShots);
        }
        displayTime(OLED::WIDTH - 10, OLED::HEIGHT - 8, "", Context::_countdown - 1, false, false, 0, OLED::Alignment::RIGHT, Context::_countdown <= 10000);
    }

    // Current status icon
    if (trigger || triggerHold) {
        Font::Char8 c = ICON_TRIGGER;
        OLED::printMedium(OLED::WIDTH - c.width, OLED::HEIGHT - c.height, c);
    } else if (focus || focusHold) {
        Font::Char8 c = ICON_FOCUS;
        OLED::printMedium(OLED::WIDTH - c.width, OLED::HEIGHT - c.height, c);
    } else if (waiting) {
        Font::Char8 c = ICON_DELAY;
        OLED::printMedium(OLED::WIDTH - c.width, OLED::HEIGHT - c.height, c);
    }
    if (input) {
        Font::Char8 c = ICON_INPUT;
        OLED::printMedium(15 + (showRSSI ? 10 : 0), OLED::HEIGHT - c.height, c);
    }
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
    OLED::clear(0, MENU_HEIGHT + 1, OLED::WIDTH, OLED::HEIGHT - MENU_HEIGHT - FOOTER_HEIGHT - 1);
    OLED::setInverted(false);

    if (displayMenuLabel) {
        // Print label
        OLED::setSize(Font::Size::LARGE);
        OLED::printCentered(OLED::WIDTH / 2, MENU_HEIGHT + (OLED::HEIGHT - MENU_HEIGHT - FOOTER_HEIGHT) / 2 - 16 / 2, MENU_LABELS[Context::_menuItemSelected]);

    } else {
        OLED::setSize(Font::Size::MEDIUM);
        const int buttonHeight = 12;

        if (Context::_menuItemSelected == MENU_TRIGGER) {
            if (Context::_tFocus == 0) {
                if (!Context::_submenuFocusHold) {
                    OLED::button(2, MENU_HEIGHT + 2, OLED::WIDTH - 4, buttonHeight, "Focus", Context::_submenuItemSelected == SUBMENU_TRIGGER_FOCUS, Context::_submenuItemSelected == SUBMENU_TRIGGER_FOCUS && Context::_btnOkPressed, false, true);
                } else {
                    OLED::button(2, MENU_HEIGHT + 2, OLED::WIDTH - 4, buttonHeight, "Focus Hold", Context::_submenuItemSelected == SUBMENU_TRIGGER_FOCUS, Context::_submenuItemSelected == SUBMENU_TRIGGER_FOCUS && Context::_btnOkPressed, true, false);
                }
            } else {
                OLED::button(2, MENU_HEIGHT + 2, OLED::WIDTH - 4, buttonHeight, "Stop", Context::_submenuItemSelected == SUBMENU_TRIGGER_FOCUS, Context::_submenuItemSelected == SUBMENU_TRIGGER_FOCUS && Context::_btnOkPressed);
            }
            if (Context::_tTrigger == 0) {
                if (!Context::_submenuTriggerHold) {
                    OLED::button(2, MENU_HEIGHT + 2 + buttonHeight + 1, OLED::WIDTH - 4, buttonHeight, "Trigger", Context::_submenuItemSelected == SUBMENU_TRIGGER_SHOOT, Context::_submenuItemSelected == SUBMENU_TRIGGER_SHOOT && Context::_btnOkPressed, false, true);
                } else {
                    OLED::button(2, MENU_HEIGHT + 2 + buttonHeight + 1, OLED::WIDTH - 4, buttonHeight, "Trigger Hold", Context::_submenuItemSelected == SUBMENU_TRIGGER_SHOOT, Context::_submenuItemSelected == SUBMENU_TRIGGER_SHOOT && Context::_btnOkPressed, true, false);
                }
            } else {
                OLED::button(2, MENU_HEIGHT + 2 + buttonHeight + 1, OLED::WIDTH - 4, buttonHeight, "Stop", Context::_submenuItemSelected == SUBMENU_TRIGGER_SHOOT, Context::_submenuItemSelected == SUBMENU_TRIGGER_SHOOT && Context::_btnOkPressed);
            }
            OLED::checkbox(2, MENU_HEIGHT + 2 + 2 * (buttonHeight + 1), OLED::WIDTH - 4, buttonHeight, "Sync", Context::_submenuItemSelected == SUBMENU_TRIGGER_SYNC, Context::_submenuItemSelected == SUBMENU_TRIGGER_SYNC && Context::_btnOkPressed, Context::_triggerSync);

        } else if (Context::_menuItemSelected == MENU_DELAY) {
            displayTimeButton(2, MENU_HEIGHT + 2, OLED::WIDTH - 4, buttonHeight, "Delay : ", Context::_delayMs, Context::_submenuItemSelected == SUBMENU_DELAY_DELAY, Context::_editingItem, Context::_editingItemCursor);
            OLED::checkbox(2, MENU_HEIGHT + 2 + buttonHeight + 1, OLED::WIDTH - 4, buttonHeight, "Sync", Context::_submenuItemSelected == SUBMENU_DELAY_SYNC, Context::_submenuItemSelected == SUBMENU_DELAY_SYNC && Context::_btnOkPressed, Context::_delaySync);

        } else if (Context::_menuItemSelected == MENU_INTERVAL) {
            displayIntButton(2, MENU_HEIGHT + 2, OLED::WIDTH - 4, buttonHeight, "Shots : ", "", Context::_intervalNShots, 4, Context::_submenuItemSelected == SUBMENU_INTERVAL_SHOTS, Context::_editingItem, Context::_editingItemCursor);
            displayTimeButton(2, MENU_HEIGHT + 2 + buttonHeight + 1, OLED::WIDTH - 4, buttonHeight, "Interval : ", Context::_intervalDelayMs, Context::_submenuItemSelected == SUBMENU_INTERVAL_DELAY, Context::_editingItem, Context::_editingItemCursor);

            OLED::checkbox(2, MENU_HEIGHT + 2 + 2 * (buttonHeight + 1), OLED::WIDTH - 4, buttonHeight, "Sync", Context::_submenuItemSelected == SUBMENU_INTERVAL_SYNC, Context::_submenuItemSelected == SUBMENU_INTERVAL_SYNC && Context::_btnOkPressed, Context::_intervalSync);
            
        } else if (Context::_menuItemSelected == MENU_TIMINGS) {
            displayTimeButton(2, MENU_HEIGHT + 2, OLED::WIDTH - 4, buttonHeight, "Focus : ", Context::_timingsFocusDurationMs, Context::_submenuItemSelected == SUBMENU_TIMINGS_FOCUS_DURATION, Context::_editingItem, Context::_editingItemCursor);
            displayTimeButton(2, MENU_HEIGHT + 2 + buttonHeight + 1, OLED::WIDTH - 4, buttonHeight, "Trigger : ", Context::_timingsTriggerDurationMs, Context::_submenuItemSelected == SUBMENU_TIMINGS_TRIGGER_DURATION, Context::_editingItem, Context::_editingItemCursor);
            OLED::checkbox(2, MENU_HEIGHT + 2 + 2 * (buttonHeight + 1), OLED::WIDTH - 4, buttonHeight, "Sync", Context::_submenuItemSelected == SUBMENU_TIMINGS_SYNC, Context::_submenuItemSelected == SUBMENU_TIMINGS_SYNC && Context::_btnOkPressed, Context::_timingsSync);

        } else if (Context::_menuItemSelected == MENU_INPUT) {
            char str[26] = "";
            if (Context::_inputMode == SUBMENU_INPUT_MODE_DISABLED) {
                strncpy(str, "Disabled", 26);
            } else if (Context::_inputMode == SUBMENU_INPUT_MODE_TRIGGER) {
                strncpy(str, "Mode : trigger", 26);
            } else if (Context::_inputMode == SUBMENU_INPUT_MODE_TRIGGER_NODELAY) {
                strncpy(str, "Mode : trig. (no delay)", 26);
            } else if (Context::_inputMode == SUBMENU_INPUT_MODE_PASSTHROUGH) {
                strncpy(str, "Mode : passthrough", 26);
            }
            OLED::button(2, MENU_HEIGHT + 2, OLED::WIDTH - 4, buttonHeight, str, Context::_submenuItemSelected == SUBMENU_INPUT_MODE, false, true, true);

            OLED::checkbox(2, MENU_HEIGHT + 2 + buttonHeight + 1, OLED::WIDTH - 4, buttonHeight, "Sync", Context::_submenuItemSelected == SUBMENU_INPUT_SYNC, Context::_submenuItemSelected == SUBMENU_INPUT_SYNC && Context::_btnOkPressed, Context::_inputSync);

        } else if (Context::_menuItemSelected == MENU_SETTINGS) {
            char str[22] = "";
            if (Context::_radio == SUBMENU_SETTINGS_RADIO_DISABLED) {
                strncpy(str, "Radio : disabled", 22);
            } else if (Context::_radio == SUBMENU_SETTINGS_RADIO_RX_ONLY) {
                strncpy(str, "Radio : receiver only", 22);
            } else if (Context::_radio == SUBMENU_SETTINGS_RADIO_ENABLED) {
                strncpy(str, "Radio : enabled", 22);
            }
            OLED::button(2, MENU_HEIGHT + 2, OLED::WIDTH - 4, buttonHeight, str, Context::_submenuItemSelected == SUBMENU_SETTINGS_RADIO, false, Context::_radio != SUBMENU_SETTINGS_RADIO_DISABLED, Context::_radio != SUBMENU_SETTINGS_RADIO_ENABLED);

            char strChannel[13] = "Channel :   ";
            if (Context::_syncChannel >= 10) {
                strChannel[10] = ((Context::_syncChannel / 10) % 10) + '0';
                strChannel[11] = (Context::_syncChannel % 10) + '0';
            } else {
                strChannel[10] = Context::_syncChannel + '0';
            }
            OLED::button(2, MENU_HEIGHT + 2 + buttonHeight + 1, OLED::WIDTH - 4, buttonHeight, strChannel, Context::_submenuItemSelected == SUBMENU_SETTINGS_CHANNEL, false);

            OLED::button(2, MENU_HEIGHT + 2 + 2 * (buttonHeight + 1), OLED::WIDTH - 4, buttonHeight, "", Context::_submenuItemSelected == SUBMENU_SETTINGS_BRIGHTNESS, false, Context::_brightness > 0, Context::_brightness < N_BRIGHTNESS_LEVELS - 1);
            const char* label = "Brightness : ";
            int width = OLED::textWidth(label) + 14;
            OLED::print((OLED::WIDTH - width) / 2, MENU_HEIGHT + 2 + 2 * (buttonHeight + 1) + (buttonHeight - 8) / 2, label);
            OLED::progressbar(OLED::cursorX(), OLED::cursorY() + 1, 14, 7, Context::_brightness * 100 / (N_BRIGHTNESS_LEVELS - 1));
        }
    }
}

bool GUI::handleButtons(int forceSync) {
    bool buttonPressed = false;
    int menuModified = -1;

    if (GPIO::fallingEdge(PIN_BTN_UP)) {
        if (Context::_menuItemSelected == MENU_TRIGGER && Context::_submenuItemSelected > 0) {
            Context::_submenuItemSelected--;

        } else if (Context::_menuItemSelected == MENU_DELAY && Context::_submenuItemSelected > 0) {
            if (!Context::_editingItem) {
                Context::_submenuItemSelected--;
            } else {
                if (Context::_submenuItemSelected == SUBMENU_DELAY_DELAY) {
                    incrementTimeButton(Context::_delayMs);
                    menuModified = MENU_DELAY;
                }
            }

        } else if (Context::_menuItemSelected == MENU_INTERVAL && Context::_submenuItemSelected > 0) {
            if (!Context::_editingItem) {
                Context::_submenuItemSelected--;
            } else {
                if (Context::_submenuItemSelected == SUBMENU_INTERVAL_SHOTS) {
                    incrementIntButton(Context::_intervalNShots, 4);
                    menuModified = MENU_INTERVAL;
                } else if (Context::_submenuItemSelected == SUBMENU_INTERVAL_DELAY) {
                    incrementTimeButton(Context::_intervalDelayMs);
                    menuModified = MENU_INTERVAL;
                }
            }

        } else if (Context::_menuItemSelected == MENU_TIMINGS && Context::_submenuItemSelected > 0) {
            if (!Context::_editingItem) {
                Context::_submenuItemSelected--;
            } else {
                if (Context::_submenuItemSelected == SUBMENU_TIMINGS_FOCUS_DURATION) {
                    incrementTimeButton(Context::_timingsFocusDurationMs);
                    menuModified = MENU_TIMINGS;
                } else if (Context::_submenuItemSelected == SUBMENU_TIMINGS_TRIGGER_DURATION) {
                    incrementTimeButton(Context::_timingsTriggerDurationMs);
                    menuModified = MENU_TIMINGS;
                }
            }

        } else if (Context::_menuItemSelected == MENU_INPUT && Context::_submenuItemSelected > 0) {
            Context::_submenuItemSelected--;

        } else if (Context::_menuItemSelected == MENU_SETTINGS && Context::_submenuItemSelected > 0) {
            Context::_submenuItemSelected--;
        }
        if (Context::_submenuItemSelected == 0) {
            showMenu();
        }
        buttonPressed = true;

    } else if (GPIO::fallingEdge(PIN_BTN_DOWN)) {
        _tMenuChange = 0;
        if (Context::_menuItemSelected == MENU_TRIGGER && Context::_submenuItemSelected < SUBMENU_TRIGGER_SYNC) {
            Context::_submenuItemSelected++;

        } else if (Context::_menuItemSelected == MENU_DELAY && Context::_submenuItemSelected < SUBMENU_DELAY_SYNC) {
            if (!Context::_editingItem) {
                Context::_submenuItemSelected++;
            } else {
                if (Context::_submenuItemSelected == SUBMENU_DELAY_DELAY) {
                    decrementTimeButton(Context::_delayMs);
                    menuModified = MENU_DELAY;
                }
            }

        } else if (Context::_menuItemSelected == MENU_INTERVAL && Context::_submenuItemSelected < SUBMENU_INTERVAL_SYNC) {
            if (!Context::_editingItem) {
                Context::_submenuItemSelected++;
            } else {
                if (Context::_submenuItemSelected == SUBMENU_INTERVAL_SHOTS) {
                    decrementIntButton(Context::_intervalNShots, 4, 1);
                    menuModified = MENU_INTERVAL;
                } else if (Context::_submenuItemSelected == SUBMENU_INTERVAL_DELAY) {
                    decrementTimeButton(Context::_intervalDelayMs);
                    menuModified = MENU_INTERVAL;
                }
            }

        } else if (Context::_menuItemSelected == MENU_TIMINGS && Context::_submenuItemSelected < SUBMENU_TIMINGS_SYNC) {
            if (!Context::_editingItem) {
                Context::_submenuItemSelected++;
            } else {
                if (Context::_submenuItemSelected == SUBMENU_TIMINGS_FOCUS_DURATION) {
                    decrementTimeButton(Context::_timingsFocusDurationMs);
                    menuModified = MENU_TIMINGS;
                } else if (Context::_submenuItemSelected == SUBMENU_TIMINGS_TRIGGER_DURATION) {
                    decrementTimeButton(Context::_timingsTriggerDurationMs);
                    menuModified = MENU_TIMINGS;
                }
            }

        } else if (Context::_menuItemSelected == MENU_INPUT && Context::_submenuItemSelected < SUBMENU_INPUT_SYNC) {
            Context::_submenuItemSelected++;

        } else if (Context::_menuItemSelected == MENU_SETTINGS && Context::_submenuItemSelected < SUBMENU_SETTINGS_BRIGHTNESS) {
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
                    if (!Context::_editingItem) {
                        Context::_editingItem = true;
                        Context::_editingItemCursor = 0;
                    } else {
                        if (Context::_editingItemCursor < 6) {
                            Context::_editingItemCursor++;
                        }
                    }
                }

            } else if (Context::_menuItemSelected == MENU_INTERVAL) {
                if (Context::_submenuItemSelected == SUBMENU_INTERVAL_SHOTS) {
                    if (!Context::_editingItem) {
                        Context::_editingItem = true;
                        Context::_editingItemCursor = 0;
                    } else {
                        if (Context::_editingItemCursor < 3) {
                            Context::_editingItemCursor++;
                        }
                    }
                } else if (Context::_submenuItemSelected == SUBMENU_INTERVAL_DELAY) {
                    if (!Context::_editingItem) {
                        Context::_editingItem = true;
                        Context::_editingItemCursor = 0;
                    } else {
                        if (Context::_editingItemCursor < 6) {
                            Context::_editingItemCursor++;
                        }
                    }
                }

            } else if (Context::_menuItemSelected == MENU_TIMINGS) {
                if (Context::_submenuItemSelected == SUBMENU_TIMINGS_FOCUS_DURATION || Context::_submenuItemSelected == SUBMENU_TIMINGS_TRIGGER_DURATION) {
                    if (!Context::_editingItem) {
                        Context::_editingItem = true;
                        Context::_editingItemCursor = 0;
                    } else {
                        if (Context::_editingItemCursor < 6) {
                            Context::_editingItemCursor++;
                        }
                    }
                }

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
                if (Context::_submenuItemSelected == SUBMENU_SETTINGS_RADIO) {
                    if (Context::_radio > 0) {
                        Context::_radio--;
                    }
                } else if (Context::_submenuItemSelected == SUBMENU_SETTINGS_CHANNEL) {
                    if (Context::_syncChannel > 0) {
                        Context::_syncChannel--;
                    }
                } else if (Context::_submenuItemSelected == SUBMENU_SETTINGS_BRIGHTNESS) {
                    if (Context::_brightness > 0) {
                        Context::_brightness--;
                        updateBrightness();
                    }
                }
                menuModified = MENU_SETTINGS;
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
                    if (!Context::_editingItem) {
                        Context::_editingItem = true;
                        Context::_editingItemCursor = 6;
                    } else {
                        if (Context::_editingItemCursor > 0) {
                            Context::_editingItemCursor--;
                        }
                    }
                }

            } else if (Context::_menuItemSelected == MENU_INTERVAL) {
                if (Context::_submenuItemSelected == SUBMENU_INTERVAL_SHOTS) {
                    if (!Context::_editingItem) {
                        Context::_editingItem = true;
                        Context::_editingItemCursor = 3;
                    } else {
                        if (Context::_editingItemCursor > 0) {
                            Context::_editingItemCursor--;
                        }
                    }
                } else if (Context::_submenuItemSelected == SUBMENU_INTERVAL_DELAY) {
                    if (!Context::_editingItem) {
                        Context::_editingItem = true;
                        Context::_editingItemCursor = 6;
                    } else {
                        if (Context::_editingItemCursor > 0) {
                            Context::_editingItemCursor--;
                        }
                    }
                }
                
            } else if (Context::_menuItemSelected == MENU_TIMINGS) {
                if (Context::_submenuItemSelected == SUBMENU_TIMINGS_FOCUS_DURATION || Context::_submenuItemSelected == SUBMENU_TIMINGS_TRIGGER_DURATION) {
                    if (!Context::_editingItem) {
                        Context::_editingItem = true;
                        Context::_editingItemCursor = 6;
                    } else {
                        if (Context::_editingItemCursor > 0) {
                            Context::_editingItemCursor--;
                        }
                    }
                }

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
                if (Context::_submenuItemSelected == SUBMENU_SETTINGS_RADIO) {
                    if (Context::_radio < SUBMENU_SETTINGS_RADIO_ENABLED) {
                        Context::_radio++;
                    }
                } else if (Context::_submenuItemSelected == SUBMENU_SETTINGS_CHANNEL) {
                    if (Context::_syncChannel < Sync::N_CHANNELS) {
                        Context::_syncChannel++;
                    }
                } else if (Context::_submenuItemSelected == SUBMENU_SETTINGS_BRIGHTNESS) {
                    if (Context::_brightness < N_BRIGHTNESS_LEVELS - 1) {
                        Context::_brightness++;
                        updateBrightness();
                    }
                }
                menuModified = MENU_SETTINGS;
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
                        copyShadowContext();
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

            } else if (Context::_submenuItemSelected == SUBMENU_TRIGGER_SHOOT) {
                if (Context::_btnOkPressed) {
                    // Pressed
                    if (Context::_tTrigger == 0) {
                        if (!Context::_submenuTriggerHold) {
                            // Start
                            copyShadowContext();
                            Context::_tTrigger = Core::time();
                            Context::_skipDelay = false;
                            if (Context::_triggerSync) {
                                Sync::send(Sync::CMD_TRIGGER);
                            }
                        }
                    } else {
                        // Stop
                        Context::_tTrigger = 0;
                        Context::_skipDelay = false;
                        if (Context::_triggerSync) {
                            Sync::send(Sync::CMD_TRIGGER_RELEASE);
                        }
                        if (Context::_submenuTriggerHold) {
                            Context::_inhibitTriggerHold = true;
                        }
                    }
                } else {
                    // Released
                    Context::_inhibitTriggerHold = false;
                }
            } else if (Context::_submenuItemSelected == SUBMENU_TRIGGER_SYNC) {
                if (Context::_btnOkPressed) {
                    Context::_triggerSync = !Context::_triggerSync;
                    menuModified = MENU_TRIGGER;
                }
            }

        } else if (Context::_menuItemSelected == MENU_DELAY) {
            if (Context::_submenuItemSelected == SUBMENU_DELAY_DELAY) {
                if (Context::_btnOkPressed) {
                    if (!Context::_editingItem) {
                        Context::_editingItem = true;
                        Context::_editingItemCursor = 1;
                    } else {
                        Context::_editingItem = false;
                    }
                }
            } else if (Context::_submenuItemSelected == SUBMENU_DELAY_SYNC) {
                if (Context::_btnOkPressed) {
                    Context::_delaySync = !Context::_delaySync;
                    menuModified = MENU_DELAY;
                }
            }

        } else if (Context::_menuItemSelected == MENU_INTERVAL) {
            if (Context::_submenuItemSelected == SUBMENU_INTERVAL_SHOTS || Context::_submenuItemSelected == SUBMENU_INTERVAL_DELAY) {
                if (Context::_btnOkPressed) {
                    if (!Context::_editingItem) {
                        Context::_editingItem = true;
                        Context::_editingItemCursor = (Context::_submenuItemSelected == SUBMENU_INTERVAL_DELAY ? 1 : 0);
                    } else {
                        Context::_editingItem = false;
                    }
                }
            } else if (Context::_submenuItemSelected == SUBMENU_INTERVAL_SYNC) {
                if (Context::_btnOkPressed) {
                    Context::_intervalSync = !Context::_intervalSync;
                    menuModified = MENU_INTERVAL;
                }
            }

        } else if (Context::_menuItemSelected == MENU_TIMINGS) {
            if (Context::_submenuItemSelected == SUBMENU_TIMINGS_FOCUS_DURATION || Context::_submenuItemSelected == SUBMENU_TIMINGS_TRIGGER_DURATION) {
                if (Context::_btnOkPressed) {
                    if (!Context::_editingItem) {
                        Context::_editingItem = true;
                        Context::_editingItemCursor = 1;
                    } else {
                        Context::_editingItem = false;
                    }
                }
            } else if (Context::_submenuItemSelected == SUBMENU_TIMINGS_SYNC) {
                if (Context::_btnOkPressed) {
                    Context::_timingsSync = !Context::_timingsSync;
                    menuModified = MENU_TIMINGS;
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
            // Nothing to check in this menu
        }

        buttonPressed = true;
    }

    // Sync
    if (forceSync > -1) {
        menuModified = forceSync;
    }
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

        } else if (menuModified == MENU_TIMINGS) {
            payload[0] = ((Context::_timingsFocusDurationMs / 100) >> 16) & 0xFF;
            payload[1] = ((Context::_timingsFocusDurationMs / 100) >> 8) & 0xFF;
            payload[2] = (Context::_timingsFocusDurationMs / 100) & 0xFF;
            payload[3] = ((Context::_timingsTriggerDurationMs / 100) >> 16) & 0xFF;
            payload[4] = ((Context::_timingsTriggerDurationMs / 100) >> 8) & 0xFF;
            payload[5] = (Context::_timingsTriggerDurationMs / 100) & 0xFF;
            payload[6] = Context::_timingsSync;
            payloadSize = 6;
            payloadSizeUSB = 7;
            if (Context::_timingsSync) {
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

void GUI::update(bool refresh, bool refreshFooter, bool trigger, bool triggerHold, bool focus, bool focusHold, bool waiting, bool input) {
    // Hide init logo after timeout
    if (_tGUIInit > 0) {
        if (Core::time() >= _tGUIInit + DELAY_LOGO_INIT) {
            GUI::showMenu();
            _tGUIInit = 0;
            refresh = true;
            refreshFooter = true;
        } else {
            refresh = false;
            refreshFooter = false;
        }
    }
    
    // Hide menu label after timeout
    if (_tMenuChange > 0 && Core::time() >= _tMenuChange + DELAY_MENU_LABEL) {
        refresh = true;
    }

    // Update menu content
    if (refresh) {
        GUI::showMenuContent();
    }

    // Update footer
    if (refreshFooter) {
        GUI::showFooter(trigger, triggerHold, focus, focusHold, waiting, input);
    }

    // Update screen
    if (refresh || refreshFooter) {
        OLED::refresh();
    }
}

void GUI::displayTimeButton(unsigned int x, unsigned int y, unsigned int width, unsigned int height, const char* label, unsigned int valueMs, bool selected, bool editing, int editingCursor) {
    // Display an empty button
    OLED::button(x, y, width, height, "", selected, false);

    // Display the time in the label
    displayTime(x + width / 2, y + (height - 8) / 2, label, valueMs, selected, editing, editingCursor, OLED::Alignment::CENTERED);
}

void GUI::displayTime(unsigned int x, unsigned int y, const char* label, unsigned int valueMs, bool selected, bool editing, int editingCursor, OLED::Alignment alignment, bool displayFrac) {
    // Compute the text length
    int textWidth = OLED::textWidth(label);
    if (selected && editing) {
        textWidth += OLED::textWidth("00h00m00s");
        if (displayFrac) {
            textWidth += OLED::textWidth(".0");
        }
        textWidth += 2; // 1px each side of the selected field
    } else {
        if (valueMs >= 10 * 60 * 60 * 1000) {
            textWidth += OLED::textWidth("0");
        }
        if (valueMs >= 60 * 60 * 1000) {
            textWidth += OLED::textWidth("0h");
        }
        if (valueMs >= 10 * 60 * 1000) {
            textWidth += OLED::textWidth("0");
        }
        if (valueMs >= 60 * 1000) {
            textWidth += OLED::textWidth("0m");
        }
        if (valueMs >= 10 * 1000) {
            textWidth += OLED::textWidth("0");
        }
        textWidth += OLED::textWidth("00");
        if (displayFrac && valueMs % 1000 != 0) {
            textWidth += OLED::textWidth(".0");
        }
        textWidth += OLED::textWidth("s");
    }
    int xText = 0;
    if (alignment == OLED::Alignment::LEFT) {
        xText = x;
    } else if (alignment == OLED::Alignment::CENTERED) {
        xText = x - textWidth / 2;
    } else if (alignment == OLED::Alignment::RIGHT) {
        xText = x - textWidth;
    }
    if (xText < 0) {
        xText = 0;
    }

    // Display the label
    OLED::print(xText, y, label);

    // Display the time, with the cursor if editing is enabled
    const unsigned int dividers[] = {100, 1000, 10 * 1000, 60 * 1000, 10 * 60 * 1000, 60 * 60 * 1000, 10 * 60 * 60 * 1000};
    const unsigned int modulos[] = {10, 10, 6, 10, 6, 10, 10};
    int start = 0;
    int end = 0;
    if (selected && editing) {
        start = 6;
        end = 0;
    } else {
        if (valueMs >= 10 * 60 * 60 * 1000) {
            start = 6;
        } else if (valueMs >= 60 * 60 * 1000) {
            start = 5;
        } else if (valueMs >= 10 * 60 * 1000) {
            start = 4;
        } else if (valueMs >= 60 * 1000) {
            start = 3;
        } else if (valueMs >= 10 * 1000) {
            start = 2;
        } else {
            start = 1;
        }
        if (displayFrac && valueMs % 1000 != 0) {
            end = 0;
        } else {
            end = 1;
        }
    }
    for (int i = start; i >= end; i--) {
        char c = ((valueMs / dividers[i]) % modulos[i]) + '0';
        int charWidth = OLED::charWidth(c);
        if (selected && editing && editingCursor == i) {
            OLED::rect(OLED::cursorX(), OLED::cursorY() - 1, charWidth + 2, 10);
            OLED::moveCursor(1, 0);
            OLED::setInverted(true);
        }
        OLED::print(c);
        if (selected && editing && editingCursor == i) {
            OLED::moveCursor(1, 0);
            OLED::setInverted(false);
        }
        if (i == 5) {
            OLED::print('h');
        } else if (i == 3) {
            OLED::print('m');
        } else if (i == 1) {
            if (end == 0) {
                OLED::print('.');
            } else {
                OLED::print('s');
            }
        } else if (i == 0) {
            OLED::print("s");
        }
    }
}

void GUI::incrementTimeButton(unsigned int& valueMs) {
    const int ARRAY_SIZE = 7;
    const unsigned int increments[ARRAY_SIZE] = {100, 1000, 10 * 1000, 60 * 1000, 10 * 60 * 1000, 60 * 60 * 1000, 10 * 60 * 60 * 1000};
    if (Context::_editingItemCursor < ARRAY_SIZE) {
        valueMs += increments[Context::_editingItemCursor];
    }
    const unsigned int MAX = 99 * 60 * 60 * 1000 + 59 * 60 * 1000 + 59 * 1000 + 900;
    if (valueMs > MAX) {
        valueMs = MAX;
    }
}

void GUI::decrementTimeButton(unsigned int& valueMs) {
    const int ARRAY_SIZE = 7;
    const unsigned int increments[ARRAY_SIZE] = {100, 1000, 10 * 1000, 60 * 1000, 10 * 60 * 1000, 60 * 60 * 1000, 10 * 60 * 60 * 1000};
    int valueMsSigned = valueMs;
    if (Context::_editingItemCursor < ARRAY_SIZE) {
        valueMsSigned -= increments[Context::_editingItemCursor];
    }
    if (valueMsSigned < 0) {
        valueMsSigned = 0;
    }
    valueMs = valueMsSigned;
}

void GUI::displayIntButton(unsigned int x, unsigned int y, unsigned int width, unsigned int height, const char* label, const char* labelUnit, unsigned int value, unsigned int length, bool selected, bool editing, int editingCursor) {
    // Display an empty button
    OLED::button(x, y, width, height, "", selected, false);

    // Compute the text length
    unsigned int start = 0;
    int textWidth = OLED::textWidth(label);
    if (labelUnit != nullptr) {
        textWidth += OLED::textWidth(labelUnit);
    }
    if (selected && editing) {
        textWidth += OLED::textWidth("0") * length;
        textWidth += 2; // 1px each side of the selected field
    } else {
        for (unsigned int i = 0; i < length; i++) {
            unsigned int p = 1;
            for (unsigned int j = 0; j < i; j++) {
                p *= 10;
            }
            if ((i == 0 && value == 0) || value >= p) {
                textWidth += OLED::textWidth("0");
                start++;
            } else {
                break;
            }
        }
    }
    int xText = x + (width - textWidth) / 2;
    if (xText < 0) {
        xText = 0;
    }

    // Display the label
    OLED::print(xText, y + (height - 8) / 2, label);

    // Display the value, with the cursor if editing is enabled
    if (selected && editing) {
        start = length;
    }
    for (int i = start - 1; i >= 0; i--) {
        unsigned int p = 1;
        for (int j = 0; j < i; j++) {
            p *= 10;
        }
        char c = ((value / p) % 10) + '0';
        int charWidth = OLED::charWidth(c);
        if (selected && editing && editingCursor == i) {
            OLED::rect(OLED::cursorX(), OLED::cursorY() - 1, charWidth + 2, 10);
            OLED::moveCursor(1, 0);
            OLED::setInverted(true);
        }
        OLED::print(c);
        if (selected && editing && editingCursor == i) {
            OLED::moveCursor(1, 0);
            OLED::setInverted(false);
        }
    }
    if (labelUnit != nullptr) {
        OLED::print(labelUnit);
    }
}

void GUI::incrementIntButton(int& value, unsigned int length) {
    unsigned int p = 1;
    for (int i = 0; i < Context::_editingItemCursor; i++) {
        p *= 10;
    }
    value += p;
    int max = 1;
    for (unsigned int i = 0; i < length; i++) {
        max *= 10;
    }
    max -= 1;
    if (value > max) {
        value = max;
    }
}

void GUI::decrementIntButton(int& value, unsigned int length, int min) {
    int valueSigned = value;
    unsigned int p = 1;
    for (int i = 0; i < Context::_editingItemCursor; i++) {
        p *= 10;
    }
    valueSigned -= p;
    if (valueSigned < min) {
        valueSigned = min;
    }
    value = valueSigned;
}

void GUI::showExitScreen() {
    OLED::clear();
    OLED::setSize(Font::Size::MEDIUM);
    OLED::printXXLarge((OLED::WIDTH - 64) / 2, (OLED::HEIGHT - 64) / 2, ICON_SILICA_XXL);
    OLED::printCentered(OLED::WIDTH / 2, 54, "Bye!");
    OLED::refresh();
}

void GUI::updateBrightness() {
    OLED::setContrast(_brightnessValues[Context::_brightness]);
}

void GUI::copyShadowContext() {
    Context::_shadowDelayMs = Context::_delayMs;
    Context::_shadowIntervalNShots = Context::_intervalNShots;
    Context::_shadowIntervalDelayMs = Context::_intervalDelayMs;
    Context::_shadowTimingsFocusDurationMs = Context::_timingsFocusDurationMs;
    Context::_shadowTimingsTriggerDurationMs = Context::_timingsTriggerDurationMs;
}
