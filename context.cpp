#include "context.h"
#include "gui.h"
#include <flash.h>

int Context::_menuItemSelected = 0;
int Context::_submenuItemSelected = 0;
bool Context::_editingItem = false;
int Context::_editingItemCursor = 0;
bool Context::_btnOkPressed = false;
bool Context::_submenuFocusHold = false;
bool Context::_submenuTriggerHold = false;

bool Context::_triggerSync = true;
unsigned int Context::_delayMs = 0;
bool Context::_delaySync = true;
int Context::_intervalNShots = 1;
unsigned int Context::_intervalDelayMs = 1000;
bool Context::_intervalSync = true;
int Context::_inputMode = GUI::SUBMENU_INPUT_MODE_PASSTHROUGH;
bool Context::_inputSync = true;
unsigned int Context::_timingsFocusDurationMs = 0;
unsigned int Context::_timingsTriggerDurationMs = 100;
bool Context::_timingsSync = true;
int Context::_syncChannel = 0;
int Context::_radio = GUI::SUBMENU_SETTINGS_RADIO_ENABLED;
int Context::_brightness = 3;

Core::Time Context::_tFocus = 0;
Core::Time Context::_tTrigger = 0;
bool Context::_inhibitTriggerHold = false;
bool Context::_skipDelay = false;
int Context::_shotsLeft = 0;
unsigned int Context::_countdown = 0;

int Context::_vBat = 0;

unsigned int Context::_shadowDelayMs = 0;
int Context::_shadowIntervalNShots = 1;
unsigned int Context::_shadowIntervalDelayMs = 0;
unsigned int Context::_shadowTimingsFocusDurationMs = 0;
unsigned int Context::_shadowTimingsTriggerDurationMs = 0;

int Context::_rssi = -137;
Core::Time Context::_tReceivedCommand = 0;


void Context::read() {
    uint32_t pageBuffer[Flash::FLASH_PAGE_SIZE_WORDS];
    Flash::readUserPage(pageBuffer);
    // First two words are reserved
    int i = 2;
    if (pageBuffer[i] == 0xFFFFFFFF) {
        // Empty config, initialize it by saving the default config
        save();
    } else {
        _triggerSync = static_cast<bool>(pageBuffer[i++]);
        _delayMs = static_cast<unsigned int>(pageBuffer[i++]);
        _delaySync = static_cast<bool>(pageBuffer[i++]);
        _intervalNShots = static_cast<int>(pageBuffer[i++]);
        _intervalDelayMs = static_cast<unsigned int>(pageBuffer[i++]);
        _intervalSync = static_cast<bool>(pageBuffer[i++]);
        _inputMode = static_cast<int>(pageBuffer[i++]);
        _inputSync = static_cast<bool>(pageBuffer[i++]);
        _timingsFocusDurationMs = static_cast<unsigned int>(pageBuffer[i++]);
        _timingsTriggerDurationMs = static_cast<unsigned int>(pageBuffer[i++]);
        _timingsSync = static_cast<bool>(pageBuffer[i++]);
        _syncChannel = static_cast<int>(pageBuffer[i++]);
        _radio = static_cast<int>(pageBuffer[i++]);
        _brightness = static_cast<int>(pageBuffer[i++]);
    }
}

void Context::save() {
    uint32_t pageBuffer[Flash::FLASH_PAGE_SIZE_WORDS];
    Flash::readUserPage(pageBuffer);
    // First two words are reserved
    int i = 2;
    pageBuffer[i++] = static_cast<uint32_t>(_triggerSync);
    pageBuffer[i++] = static_cast<uint32_t>(_delayMs);
    pageBuffer[i++] = static_cast<uint32_t>(_delaySync);
    pageBuffer[i++] = static_cast<uint32_t>(_intervalNShots);
    pageBuffer[i++] = static_cast<uint32_t>(_intervalDelayMs);
    pageBuffer[i++] = static_cast<uint32_t>(_intervalSync);
    pageBuffer[i++] = static_cast<uint32_t>(_inputMode);
    pageBuffer[i++] = static_cast<uint32_t>(_inputSync);
    pageBuffer[i++] = static_cast<uint32_t>(_timingsFocusDurationMs);
    pageBuffer[i++] = static_cast<uint32_t>(_timingsTriggerDurationMs);
    pageBuffer[i++] = static_cast<uint32_t>(_timingsSync);
    pageBuffer[i++] = static_cast<uint32_t>(_syncChannel);
    pageBuffer[i++] = static_cast<uint32_t>(_radio);
    pageBuffer[i++] = static_cast<uint32_t>(_brightness);
    Flash::writeUserPage(pageBuffer);
}
