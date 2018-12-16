#ifndef _CARBIDE_H_
#define _CARBIDE_H_

#include <gpio.h>

#if PACKAGE != 64
#warning "You are compiling for the Carbide with an incorrect package, please set PACKAGE=64 in your Makefile"
#endif

namespace Carbide {

    // Pins definition
    const GPIO::Pin PIN_LED_R = GPIO::PA00;
    const GPIO::Pin PIN_LED_G = GPIO::PA01;
    const GPIO::Pin PIN_LED_B = GPIO::PA02;
    const GPIO::Pin PIN_BUTTON = GPIO::PA04;

    // Predefined CPU frequencies
    enum class CPUFreq {
        FREQ_4MHZ,
        FREQ_8MHZ,
        FREQ_12MHZ,
        FREQ_24MHZ,
        FREQ_36MHZ,
        FREQ_48MHZ
    };

    // Helper functions
    void init(bool autoBootloaderReset=false);
    void setCPUFrequency(CPUFreq frequency);
    void warningHandler(Error::Module module, int userModule, Error::Code code);
    void criticalHandler(Error::Module module, int userModule, Error::Code code);
    inline void initLedR() { GPIO::enableOutput(PIN_LED_R, GPIO::HIGH); }
    inline void setLedR(bool on=true) { GPIO::set(PIN_LED_R, !on); } // Inverted : pin must be LOW to turn the LED on
    inline void initLedG() { GPIO::enableOutput(PIN_LED_G, GPIO::HIGH); }
    inline void setLedG(bool on=true) { GPIO::set(PIN_LED_G, !on); }
    inline void initLedB() { GPIO::enableOutput(PIN_LED_B, GPIO::HIGH); }
    inline void setLedB(bool on=true) { GPIO::set(PIN_LED_B, !on); }
    inline void initLeds() { initLedR(); initLedG(); initLedB(); }
    inline void initButton() { GPIO::enableInput(PIN_BUTTON, GPIO::Pulling::PULLUP); }
    inline bool isButtonPressed() { return !GPIO::get(PIN_BUTTON); } // Inverted : the pin is LOW when the button is pressed (pullup)
    void onButtonPressed(void (*handler)(), bool released=false);
    inline bool buttonRisingEdge() { return GPIO::fallingEdge(PIN_BUTTON); } // Rising/falling are also inverted for the same reasons
    inline bool buttonFallingEdge() { return GPIO::risingEdge(PIN_BUTTON); }
    inline void waitButtonPressed() { while (!buttonRisingEdge()); }

}

#endif