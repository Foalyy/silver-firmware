#ifndef _SERVO_H_
#define _SERVO_H_

#include <tc.h>
#include <gpio.h>

// This class is a shallow helper to use a TC channel as a PWM generator to control a servomotor
class Servo {
private:
    TC::Channel _tcChannel;
    GPIO::Pin _pin;
    unsigned int _period;
    unsigned int _highTime0;
    unsigned int _highTime100;
    unsigned int _percent;
    bool _disabled;

    static const unsigned int DEFAULT_PERIOD = 10000;
    static const unsigned int DEFAULT_HIGH_TIME_0 = 1000;
    static const unsigned int DEFAULT_HIGH_TIME_100 = 2000;

public:
    // Constructor : must specify the underlying TC channel to use
    Servo(TC::Channel tcChannel, GPIO::Pin pin={GPIO::Port::A, 0xFF});

    // Set the servo position in percent
    // This will translate to differant angles according to the exact servo angular range
    void set(unsigned int percent);

    void disable();

    // Customize the PWM timings
    // Passing no argument resets the default values
    void setPWMTimings(unsigned int highTime0=DEFAULT_HIGH_TIME_0, unsigned int highTime100=DEFAULT_HIGH_TIME_100, unsigned int period=DEFAULT_PERIOD);

};

#endif