#ifndef _GUI_H_
#define _GUI_H_

#include <core.h>
#include "drivers/oled_ssd1306/oled.h"
#include "context.h"

namespace GUI {

    const int N_MENU_ITEMS = 6;

    const int PANNEL_MENU = 0;
    const int PANNEL_LORA_TEST = 1;
    const int MENU_TRIGGER = 0;
    const int SUBMENU_TRIGGER_FOCUS = 1;
    const int SUBMENU_TRIGGER_SHOOT = 2;
    const int SUBMENU_TRIGGER_SYNC = 3;
    const int MENU_DELAY = 1;
    const int SUBMENU_DELAY_DELAY = 1;
    const int SUBMENU_DELAY_SYNC = 2;
    const int MENU_INTERVAL = 2;
    const int SUBMENU_INTERVAL_SHOTS = 1;
    const int SUBMENU_INTERVAL_DELAY = 2;
    const int SUBMENU_INTERVAL_SYNC = 3;
    const int MENU_TIMINGS = 3;
    const int SUBMENU_TIMINGS_FOCUS_DURATION = 1;
    const int SUBMENU_TIMINGS_TRIGGER_DURATION = 2;
    const int SUBMENU_TIMINGS_SYNC = 3;
    const int MENU_INPUT = 4;
    const int SUBMENU_INPUT_MODE = 1;
    const int SUBMENU_INPUT_MODE_DISABLED = 0;
    const int SUBMENU_INPUT_MODE_TRIGGER = 1;
    const int SUBMENU_INPUT_MODE_TRIGGER_NODELAY = 2;
    const int SUBMENU_INPUT_MODE_PASSTHROUGH = 3;
    const int SUBMENU_INPUT_SYNC = 2;
    const int MENU_SETTINGS = 5;
    const int SUBMENU_SETTINGS_RADIO = 1;
    const int SUBMENU_SETTINGS_RADIO_DISABLED = 0;
    const int SUBMENU_SETTINGS_RADIO_RX_ONLY = 1;
    const int SUBMENU_SETTINGS_RADIO_ENABLED = 2;
    const int SUBMENU_SETTINGS_CHANNEL = 2;
    const int SUBMENU_SETTINGS_BRIGHTNESS = 3;


    void init();
    void setMenu(int menuItemSelected);
    void showMenu();
    void showFooter(bool trigger, bool triggerHold, bool focus, bool focusHold, bool waiting, bool input);
    void showMenuContent();
    bool handleButtons(int forceSync=-1);
    void update(bool refresh, bool refreshFooter, bool trigger, bool triggerHold, bool focus, bool focusHold, bool waiting, bool input);
    void displayTimeButton(unsigned int x, unsigned int y, unsigned int width, unsigned int height, const char* label, unsigned int valueMs, bool selected=false, bool editing=false, int editingCursor=0);
    void displayTime(unsigned int x, unsigned int y, const char* label, unsigned int valueMs, bool selected=false, bool editing=false, int editingCursor=0, OLED::Alignment alignment=OLED::Alignment::LEFT, bool displayFrac=true);
    void incrementTimeButton(unsigned int& valueMs);
    void decrementTimeButton(unsigned int& valueMs);
    void displayIntButton(unsigned int x, unsigned int y, unsigned int width, unsigned int height, const char* label, const char* labelUnit, unsigned int value, unsigned int length, bool selected, bool editing, int editingCursor);
    void incrementIntButton(int& value, unsigned int length);
    void decrementIntButton(int& value, unsigned int length, int min=0);
    void showExitScreen();
    void updateBrightness();
    void copyShadowContext();

}

#endif