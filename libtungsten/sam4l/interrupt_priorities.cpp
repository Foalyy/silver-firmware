#include <stdint.h>
#include "ast.h"
#include "bpm.h"
#include "dma.h"
#include "eic.h"
#include "gpio.h"
#include "i2c.h"
#include "pm.h"
#include "tc.h"
#include "trng.h"
#include "usart.h"
#include "usb.h"

// Note : a high number means a lower priority.
// The highest priority is 1.

namespace AST {
    uint8_t INTERRUPT_PRIORITY = 100;
}

namespace BPM {
    uint8_t INTERRUPT_PRIORITY = 2;
}

namespace DMA {
    uint8_t INTERRUPT_PRIORITY = 5;
}

namespace EIC {
    uint8_t INTERRUPT_PRIORITY = 50;
}

namespace GPIO {
    uint8_t INTERRUPT_PRIORITY = 50;
}

namespace I2C {
    uint8_t INTERRUPT_PRIORITY = 10;
}

namespace PM {
    uint8_t INTERRUPT_PRIORITY = 2;
}

namespace SPI {
    uint8_t INTERRUPT_PRIORITY = 10;
}

namespace TC {
    uint8_t INTERRUPT_PRIORITY = 40;
}

namespace TRNG {
    uint8_t INTERRUPT_PRIORITY = 50;
}

namespace USART {
    uint8_t INTERRUPT_PRIORITY = 20;
}

namespace USB {
    uint8_t INTERRUPT_PRIORITY = 8;
}

namespace WDT {
    uint8_t INTERRUPT_PRIORITY = 3;
}