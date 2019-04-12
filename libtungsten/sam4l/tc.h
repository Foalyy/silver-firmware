#ifndef _TC_H_
#define _TC_H_

#include <stdint.h>
#include "gpio.h"
#include "error.h"

// Timers/Counters
// This module manages timers, which are counting registers that are automatically
// incremented (or decremented) by a specified clock and can trigger events
// based on the output of comparators.
// For example, they are to generate a PWM signal to control a servomotor or to
// execute a callback function after a specified delay.
namespace TC {

    // Peripheral memory space base address
    const uint32_t TC_BASE = 0x40010000;
    const uint32_t TC_SIZE = 0x4000; // Meaning that TC1 is at TC_BASE + TC_SIZE = 0x40014000


    // Registers addresses
    const uint32_t OFFSET_COUNTER_SIZE = 0x040;
    const uint32_t OFFSET_CCR0 =     0x000; // Counter 0 Control Register
    const uint32_t OFFSET_CMR0 =     0x004; // Counter 0 Mode Register
    const uint32_t OFFSET_SMMR0 =    0x008; // Counter 0 Stepper Motor Mode Register
    const uint32_t OFFSET_CV0 =      0x010; // Counter 0 Counter Value
    const uint32_t OFFSET_RA0 =      0x014; // Counter 0 Register A
    const uint32_t OFFSET_RB0 =      0x018; // Counter 0 Register B
    const uint32_t OFFSET_RC0 =      0x01C; // Counter 0 Register C
    const uint32_t OFFSET_SR0 =      0x020; // Counter 0 Status Register
    const uint32_t OFFSET_IER0 =     0x024; // Interrupt Enable Register
    const uint32_t OFFSET_IDR0 =     0x028; // Counter 0 Interrupt Disable Register
    const uint32_t OFFSET_IMR0 =     0x02C; // Counter 0 Interrupt Mask Register
    const uint32_t OFFSET_CCR1 =     0x040; // Counter 1 Control Register
    const uint32_t OFFSET_CMR1 =     0x044; // Counter 1 Mode Register
    const uint32_t OFFSET_SMMR1 =    0x048; // Counter 1 Stepper Motor Mode Register
    const uint32_t OFFSET_CV1 =      0x050; // Counter 1 Counter Value
    const uint32_t OFFSET_RA1 =      0x054; // Counter 1 Register A
    const uint32_t OFFSET_RB1 =      0x058; // Counter 1 Register B
    const uint32_t OFFSET_RC1 =      0x05C; // Counter 1 Register C
    const uint32_t OFFSET_SR1 =      0x060; // Counter 1 Status Register
    const uint32_t OFFSET_IER1 =     0x064; // Counter 1 Interrupt Enable Register
    const uint32_t OFFSET_IDR1 =     0x068; // Counter 1 Interrupt Disable Register
    const uint32_t OFFSET_IMR1 =     0x06C; // Counter 1 Interrupt Mask Register
    const uint32_t OFFSET_CCR2 =     0x080; // Counter 2 Control Register
    const uint32_t OFFSET_CMR2 =     0x084; // Counter 2 Mode Register
    const uint32_t OFFSET_SMMR2 =    0x088; // Ch 2 Stepper Motor Mode Register
    const uint32_t OFFSET_CV2 =      0x090; // Counter 2 Counter Value
    const uint32_t OFFSET_RA2 =      0x094; // Counter 2 Register A
    const uint32_t OFFSET_RB2 =      0x098; // Counter 2 Register B
    const uint32_t OFFSET_RC2 =      0x09C; // Counter 2 Register C
    const uint32_t OFFSET_SR2 =      0x0A0; // Counter 2 Status Register
    const uint32_t OFFSET_IER2 =     0x0A4; // Counter 2 Interrupt Enable Register
    const uint32_t OFFSET_IDR2 =     0x0A8; // Counter 2 Interrupt Disable Register
    const uint32_t OFFSET_IMR2 =     0x0AC; // Counter 2 Interrupt Mask Register
    const uint32_t OFFSET_BCR =      0x0C0; // Block Control Register
    const uint32_t OFFSET_BMR =      0x0C4; // Block Mode Register
    const uint32_t OFFSET_WPMR =     0x0E4; // Write Protect Mode Register
    const uint32_t OFFSET_FEATURES = 0x0F8; // Features Register


    // Subregisters
    const uint32_t CMR_TCCLKS = 0;
    const uint32_t CMR_CLKI = 3;
    const uint32_t CMR_BURST = 4;
    const uint32_t CMR_CPCSTOP = 6;
    const uint32_t CMR_CPCDIS = 7;
    const uint32_t CMR_EEVTEDG = 8;
    const uint32_t CMR_EEVT = 10;
    const uint32_t CMR_ENETRG = 12;
    const uint32_t CMR_WAVSEL = 13;
    const uint32_t CMR_WAVE = 15;
    const uint32_t CMR_ACPA = 16;
    const uint32_t CMR_ACPC = 18;
    const uint32_t CMR_AEEVT = 20;
    const uint32_t CMR_ASWTRG = 22;
    const uint32_t CMR_BCPB = 24;
    const uint32_t CMR_BCPC = 26;
    const uint32_t CMR_BEEVT = 28;
    const uint32_t CMR_BSWTRG = 30;
    const uint32_t CMR_LDBSTOP = 6;
    const uint32_t CMR_LDBDIS = 7;
    const uint32_t CMR_ETRGEDG = 8;
    const uint32_t CMR_ABETRG = 10;
    const uint32_t CMR_CPCTRG = 14;
    const uint32_t CMR_LDRA = 16;
    const uint32_t CMR_LDRB = 18;
    const uint32_t CCR_CLKEN = 0;
    const uint32_t CCR_CLKDIS = 1;
    const uint32_t CCR_SWTRG = 2;
    const uint32_t SR_COVFS = 0;
    const uint32_t SR_LOVRS = 1;
    const uint32_t SR_CPAS = 2;
    const uint32_t SR_CPBS = 3;
    const uint32_t SR_CPCS = 4;
    const uint32_t SR_LDRAS = 5;
    const uint32_t SR_LDRBS = 6;
    const uint32_t SR_ETRGS = 7;
    const uint32_t SR_CLKSTA = 16;
    const uint32_t SR_MTIOA = 17;
    const uint32_t SR_MTIOB = 18;
    const uint32_t BCR_SYNC = 0;
    const uint32_t WPMR_WPEN = 0;
    const uint32_t WPMR_WPKEY = 8;


    // Constants
    const uint32_t UNLOCK_KEY = 0x54494D;
    const uint8_t TC0 = 0;
    const uint8_t TC1 = 1;
    const uint8_t CH0 = 0;
    const uint8_t CH1 = 1;
    const uint8_t CH2 = 2;
    const uint8_t TIOA = 0;
    const uint8_t TIOB = 1;

    enum class Unit {
        MILLISECONDS,
        MICROSECONDS
    };
    
    // The actual number of TCs available is package-dependant
    // and is defined in pins_sam4l_XX.cpp
    extern const uint8_t N_TC;
    const uint8_t MAX_N_TC = 2;
    const uint8_t N_COUNTERS_PER_TC = 3;
    const uint8_t N_CHANNELS_PER_COUNTER = 2;
    const uint8_t N_EXTERNAL_CLOCKS_PER_TC = 3;

    struct Counter {
        uint8_t tc;
        uint8_t n;
    };

    struct Channel {
        Counter counter;
        uint8_t line;
    };

    enum class SourceClock {
        GENERIC_CLOCK,
        PBA_OVER_2,
        PBA_OVER_8,
        PBA_OVER_32,
        PBA_OVER_128,
        CLK0,
        CLK1,
        CLK2,
    };

    enum class PinFunction {
        OUT,
        CLK
    };

    // Quick helpers for each counter and channel
    const Counter TC0_0 = {TC::TC0, TC::CH0};
    const Channel TC0_0A = {TC::TC0, TC::CH0, TC::TIOA};
    const Channel TC0_0B = {TC::TC0, TC::CH0, TC::TIOB};
    const Counter TC0_1 = {TC::TC0, TC::CH1};
    const Channel TC0_1A = {TC::TC0, TC::CH1, TC::TIOA};
    const Channel TC0_1B = {TC::TC0, TC::CH1, TC::TIOB};
    const Counter TC0_2 = {TC::TC0, TC::CH2};
    const Channel TC0_2A = {TC::TC0, TC::CH2, TC::TIOA};
    const Channel TC0_2B = {TC::TC0, TC::CH2, TC::TIOB};
    const Counter TC1_0 = {TC::TC1, TC::CH0};
    const Channel TC1_0A = {TC::TC1, TC::CH0, TC::TIOA};
    const Channel TC1_0B = {TC::TC1, TC::CH0, TC::TIOB};
    const Counter TC1_1 = {TC::TC1, TC::CH1};
    const Channel TC1_1A = {TC::TC1, TC::CH1, TC::TIOA};
    const Channel TC1_1B = {TC::TC1, TC::CH1, TC::TIOB};
    const Counter TC1_2 = {TC::TC1, TC::CH2};
    const Channel TC1_2A = {TC::TC1, TC::CH2, TC::TIOA};
    const Channel TC1_2B = {TC::TC1, TC::CH2, TC::TIOB};

    // Error codes
    const Error::Code ERR_INVALID_TC = 0x0001;


    // Simple counter mode
    void enableSimpleCounter(Counter counter, uint16_t maxValue=0xFFFF, SourceClock sourceClock=SourceClock::PBA_OVER_8, unsigned long sourceClockFrequency=0, bool invertClock=false, bool upDown=false);

    // PWM mode
    bool enablePWM(Channel channel, float period=0, float highTime=0, bool output=true, SourceClock sourceClock=SourceClock::PBA_OVER_8, unsigned long sourceClockFrequency=0);
    bool setPeriod(Counter counter, float period);
    bool setHighTime(Channel channel, float highTime);
    bool setDutyCycle(Channel channel, int percent);
    void enableOutput(Channel channel);
    void disableOutput(Channel channel);

    // Measure mode
    void enableMeasure(Counter counter, SourceClock sourceClock=SourceClock::PBA_OVER_8, unsigned long sourceClockFrequency=0);
    void measure(Counter counter, bool continuous=false);
    uint16_t measuredPeriodRaw(Counter counter);
    unsigned long measuredPeriod(Counter counter);
    uint16_t measuredHighTimeRaw(Counter counter);
    unsigned long measuredHighTime(Counter counter);
    unsigned int measuredDutyCycle(Counter counter);
    bool isMeasureOverflow(Counter counter);

    // Low-level counter functions
    bool setRX(Channel channel, unsigned int rx);
    bool setRC(Counter counter, unsigned int rc);
    uint16_t counterValue(Counter counter);
    uint16_t raValue(Counter counter);
    uint16_t rbValue(Counter counter);
    uint16_t rcValue(Counter counter);
    unsigned long sourceClockFrequency(Counter counter);

    // Timing functions
    void wait(Counter counter, unsigned long delay, Unit unit=Unit::MILLISECONDS, SourceClock sourceClock=SourceClock::PBA_OVER_8, unsigned long sourceClockFrequency=0);
    void execDelayed(Counter counter, void (*handler)(), unsigned long delay, Unit unit=Unit::MILLISECONDS, bool repeat=false, SourceClock sourceClock=SourceClock::PBA_OVER_8, unsigned long sourceClockFrequency=0);

    // Functions common to all modes
    void start(Counter counter);
    void stop(Counter counter);
    void sync();
    void setPin(Channel channel, PinFunction function, GPIO::Pin pin);

}


#endif