#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include <core.h>

namespace Context {

    extern int _menuItemSelected;
    extern int _submenuItemSelected;
    extern bool _editingItem;
    extern int _editingItemCursor;
    extern bool _btnOkPressed;
    extern bool _submenuFocusHold;
    extern bool _submenuTriggerHold;

    extern bool _triggerSync;
    extern unsigned int _delayMs;
    extern bool _delaySync;
    extern int _intervalNShots;
    extern unsigned int _intervalDelayMs;
    extern bool _intervalSync;
    extern int _inputMode;
    extern bool _inputSync;
    extern unsigned int _timingsFocusDurationMs;
    extern unsigned int _timingsTriggerDurationMs;
    extern bool _timingsSync;
    extern int _syncChannel;
    extern int _brightness;

    extern Core::Time _tFocus;
    extern Core::Time _tTrigger;
    extern bool _skipDelay;

    void read();
    void save();

}

#endif