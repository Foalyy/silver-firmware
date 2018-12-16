#ifndef _TC_H_
#define _TC_H_

#include <stdint.h>
#include "gpio.h"

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
    const uint32_t OFFSET_CHANNEL_SIZE = 0x040;
    const uint32_t OFFSET_CCR0 =     0x000; // Channel 0 Control Register
    const uint32_t OFFSET_CMR0 =     0x004; // Channel 0 Mode Register
    const uint32_t OFFSET_SMMR0 =    0x008; // Channel 0 Stepper Motor Mode Register
    const uint32_t OFFSET_CV0 =      0x010; // Channel 0 Counter Value
    const uint32_t OFFSET_RA0 =      0x014; // Channel 0 Register A
    const uint32_t OFFSET_RB0 =      0x018; // Channel 0 Register B
    const uint32_t OFFSET_RC0 =      0x01C; // Channel 0 Register C
    const uint32_t OFFSET_SR0 =      0x020; // Channel 0 Status Register
    const uint32_t OFFSET_IER0 =     0x024; // Interrupt Enable Register
    const uint32_t OFFSET_IDR0 =     0x028; // Channel 0 Interrupt Disable Register
    const uint32_t OFFSET_IMR0 =     0x02C; // Channel 0 Interrupt Mask Register
    const uint32_t OFFSET_CCR1 =     0x040; // Channel 1 Control Register
    const uint32_t OFFSET_CMR1 =     0x044; // Channel 1 Mode Register
    const uint32_t OFFSET_SMMR1 =    0x048; // Channel 1 Stepper Motor Mode Register
    const uint32_t OFFSET_CV1 =      0x050; // Channel 1 Counter Value
    const uint32_t OFFSET_RA1 =      0x054; // Channel 1 Register A
    const uint32_t OFFSET_RB1 =      0x058; // Channel 1 Register B
    const uint32_t OFFSET_RC1 =      0x05C; // Channel 1 Register C
    const uint32_t OFFSET_SR1 =      0x060; // Channel 1 Status Register
    const uint32_t OFFSET_IER1 =     0x064; // Channel 1 Interrupt Enable Register
    const uint32_t OFFSET_IDR1 =     0x068; // Channel 1 Interrupt Disable Register
    const uint32_t OFFSET_IMR1 =     0x06C; // Channel 1 Interrupt Mask Register
    const uint32_t OFFSET_CCR2 =     0x080; // Channel 2 Control Register
    const uint32_t OFFSET_CMR2 =     0x084; // Channel 2 Mode Register
    const uint32_t OFFSET_SMMR2 =    0x088; // Ch 2 Stepper Motor Mode Register
    const uint32_t OFFSET_CV2 =      0x090; // Channel 2 Counter Value
    const uint32_t OFFSET_RA2 =      0x094; // Channel 2 Register A
    const uint32_t OFFSET_RB2 =      0x098; // Channel 2 Register B
    const uint32_t OFFSET_RC2 =      0x09C; // Channel 2 Register C
    const uint32_t OFFSET_SR2 =      0x0A0; // Channel 2 Status Register
    const uint32_t OFFSET_IER2 =     0x0A4; // Channel 2 Interrupt Enable Register
    const uint32_t OFFSET_IDR2 =     0x0A8; // Channel 2 Interrupt Disable Register
    const uint32_t OFFSET_IMR2 =     0x0AC; // Channel 2 Interrupt Mask Register
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
    const uint8_t N_CHANNELS = 3;
    const uint8_t N_LINES = 2;

    struct Channel {
        uint8_t tc;
        uint8_t subchannel;
        uint8_t line;
    };

    enum class PinFunction {
        OUT,
        CLK
    };

    // Quick helpers for each channel
    const Channel TC0_A0 = {TC::TC0, TC::CH0, TC::TIOA};
    const Channel TC0_B0 = {TC::TC0, TC::CH0, TC::TIOB};
    const Channel TC0_A1 = {TC::TC0, TC::CH1, TC::TIOA};
    const Channel TC0_B1 = {TC::TC0, TC::CH1, TC::TIOB};
    const Channel TC0_A2 = {TC::TC0, TC::CH2, TC::TIOA};
    const Channel TC0_B2 = {TC::TC0, TC::CH2, TC::TIOB};
    const Channel TC1_A0 = {TC::TC1, TC::CH0, TC::TIOA};
    const Channel TC1_B0 = {TC::TC1, TC::CH0, TC::TIOB};
    const Channel TC1_A1 = {TC::TC1, TC::CH1, TC::TIOA};
    const Channel TC1_B1 = {TC::TC1, TC::CH1, TC::TIOB};
    const Channel TC1_A2 = {TC::TC1, TC::CH2, TC::TIOA};
    const Channel TC1_B2 = {TC::TC1, TC::CH2, TC::TIOB};


    // Module API
    void init(const Channel& channel, unsigned int period=0, unsigned int highTime=0, bool output=false);
    void setPeriod(const Channel& channel, double period);
    void setHighTime(const Channel& channel, double highTime);
    void setDutyCycle(const Channel& channel, int percent);
    void enableOutput(const Channel& channel);
    void disableOutput(const Channel& channel);
    void setRC(const Channel& channel, uint16_t rc);
    void setRX(const Channel& channel, uint16_t rx);
    uint32_t getCounterValue(const Channel& channel);
    void wait(const Channel& channel, unsigned long value, Unit unit=Unit::MILLISECONDS);
    void execDelayed(const Channel& channel, void (*handler)(), unsigned long delay, bool repeat=false, Unit unit=Unit::MILLISECONDS);
    void setPin(const Channel& channel, PinFunction function, GPIO::Pin pin);

}


#endif