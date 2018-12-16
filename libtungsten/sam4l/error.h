#ifndef _ERROR_H_
#define _ERROR_H_

#include <stdint.h>

namespace Error {

    enum class Module {
        CORE,
        DMA,
        SCIF,
        BSCIF,
        PM,
        BPM,
        ADC,
        DAC,
        FLASH,
        GPIO,
        I2C,
        SPI,
        TC,
        USART,
        WDT,
        EIC,
        USB,
        CUSTOM
    };

    using Code = uint16_t;

    enum class Severity {
        INFO = 0,
        WARNING = 1,
        CRITICAL = 2
    };
    const int N_SEVERITY = 3;

    struct Error {
        unsigned long time;
        Module module;
        int userModule;
        Code code;
        Severity severity;
    };

    const int MAX_ERROR_NUMBER = 16;

    void init();
    void setHandler(Severity severity, void (*handler)(Module module, int userModule, Code code));
    void happened(Module module, Code code, Severity severity=Severity::CRITICAL);
    void happened(int userModule, int code, Severity severity=Severity::CRITICAL);
    unsigned int getNumber();
    const Error& get(unsigned int n);
    const Error& getLast();

}

#endif