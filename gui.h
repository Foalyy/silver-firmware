#ifndef _GUI_H_
#define _GUI_H_

#include <core.h>
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
    const int MENU_INPUT = 3;
    const int SUBMENU_INPUT_MODE = 1;
    const int SUBMENU_INPUT_MODE_DISABLED = 0;
    const int SUBMENU_INPUT_MODE_TRIGGER = 1;
    const int SUBMENU_INPUT_MODE_TRIGGER_NODELAY = 2;
    const int SUBMENU_INPUT_MODE_PASSTHROUGH = 3;
    const int SUBMENU_INPUT_SYNC = 2;
    const int MENU_SETTINGS = 4;
    const int SUBMENU_SETTINGS_CHANNEL = 1;
    const int SUBMENU_SETTINGS_BRIGHTNESS = 2;
    const int MENU_ADVANCED = 5;
    const int SUBMENU_ADVANCED_FOCUS_DURATION = 1;
    const int SUBMENU_ADVANCED_TRIGGER_DURATION = 2;
    const int SUBMENU_ADVANCED_SYNC = 3;

    void init();
    void setMenu(int menuItemSelected);
    void showMenu();
    void showMenuContent();
    bool handleButtons();
    void update(bool refresh, bool trigger, bool focus, bool waiting, bool input);
    void showExitScreen();

}

#endif