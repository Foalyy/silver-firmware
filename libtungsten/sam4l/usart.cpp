#include "usart.h"
#include "core.h"
#include "gpio.h"
#include "pm.h"
#include "dma.h"

namespace USART {

    // Internal functions
    void rxBufferFullHandler();

    // Clocks
    const int PM_CLK[] = {PM::CLK_USART0, PM::CLK_USART1, PM::CLK_USART2, PM::CLK_USART3};

    // Interrupt handlers
    extern uint8_t INTERRUPT_PRIORITY;
    uint32_t _interruptHandlers[N_PORTS][N_INTERRUPTS];
    const int _interruptBits[N_INTERRUPTS] = {CSR_RXRDY, CSR_TXRDY, CSR_OVRE, CSR_PARE};
    void interruptHandlerWrapper();

    // Package-dependant, defined in pins_sam4l_XX.cpp
    // Can be modified using setPin()
    extern struct GPIO::Pin PINS_RX[];
    extern struct GPIO::Pin PINS_TX[];
    extern struct GPIO::Pin PINS_CTS[];
    extern struct GPIO::Pin PINS_RTS[];

    // Ports
    struct USART {
        unsigned long baudrate;
        bool hardwareFlowControl;
        CharLength charLength;
        Parity parity;
        StopBit stopBit;
        uint8_t rxBuffer[BUFFER_SIZE];
        uint8_t txBuffer[BUFFER_SIZE];
        int rxBufferCursorR;
        int rxBufferCursorW;
        int txBufferCursor;
        int rxDMAChannel;
        int txDMAChannel;
    };
    struct USART _ports[N_PORTS];

    int _rxDMAChannelsToPorts[DMA::N_CHANNELS_MAX];

    bool _portsEnabled[N_PORTS] = {false, false, false, false};


    void enable(Port port, unsigned long baudrate, bool hardwareFlowControl, CharLength charLength, Parity parity, StopBit stopBit) {
        const uint32_t REG_BASE = USART_BASE + static_cast<int>(port) * USART_REG_SIZE;
        struct USART* p = &(_ports[static_cast<int>(port)]);

        // Port configuration
        p->hardwareFlowControl = hardwareFlowControl;
        p->charLength = charLength;
        p->parity = parity;
        p->stopBit = stopBit;

        // Initialize the buffers
        for (int i = 0; i < BUFFER_SIZE; i++) {
            p->rxBuffer[i] = 0;
            p->txBuffer[i] = 0;
        }
        p->rxBufferCursorR = 0;
        p->rxBufferCursorW = 0;
        p->txBufferCursor = 0;

        // Set the pins in peripheral mode
        GPIO::enablePeripheral(PINS_RX[static_cast<int>(port)]);
        GPIO::enablePeripheral(PINS_TX[static_cast<int>(port)]);
        if (hardwareFlowControl) {
            GPIO::enablePeripheral(PINS_RTS[static_cast<int>(port)]);
            GPIO::enablePeripheral(PINS_CTS[static_cast<int>(port)]);
        }

        // Enable the clock
        PM::enablePeripheralClock(PM_CLK[static_cast<int>(port)]);

        // WPMR (Write Protect Mode Register) : disable the Write Protect
        (*(volatile uint32_t*)(REG_BASE + OFFSET_WPMR)) = WPMR_KEY | WPMR_DISABLE;

        // MR (Mode Register) : set the USART configuration
        (*(volatile uint32_t*)(REG_BASE + OFFSET_MR))
            = (hardwareFlowControl ? MODE_HARDWARE_HANDSHAKE : MODE_NORMAL) << MR_MODE // Hardware flow control
            | (static_cast<int>(charLength) & 0b11) << MR_CHRL  // Character length <= 8
            | (static_cast<int>(charLength) >> 2) << MR_MODE9   // Character length == 9
            | static_cast<int>(parity) << MR_PAR                // Parity
            | static_cast<int>(stopBit) << MR_NBSTOP;           // Length of stop bit

        // BRGR (Baud Rate Generator Register) : configure the baudrate generator
        // Cf datasheet 24.6.4 : baudrate = mainclock / (16 * CD), with FP to fine-tune by steps of 1/8
        p->baudrate = baudrate;
        const unsigned long clk = PM::getModuleClockFrequency(PM_CLK[static_cast<int>(port)]);
        const uint64_t cd100 = 100 * clk / 16 / baudrate; // cd100 = cd * 100
        const uint32_t cd = cd100 / 100;
        const uint64_t fp100 = (cd100 % 100) * 8;
        uint8_t fp = fp100 / 100;
        if (fp100 - fp * 100 >= 50) {
            fp++; // Round
        }
        if (cd == 0 || cd > 0xFFFF) {
            Error::happened(Error::Module::USART, ERR_BAUDRATE_OUT_OF_RANGE, Error::Severity::CRITICAL);
            return;
        }
        (*(volatile uint32_t*)(REG_BASE + OFFSET_BRGR))
            = cd << BRGR_CD
            | fp << BRGR_FP;

        // CR (Control Register) : enable RX and TX
        (*(volatile uint32_t*)(REG_BASE + OFFSET_CR))
            = 1 << CR_RXEN
            | 1 << CR_TXEN;

        // WPMR (Write Protect Mode Register) : re-enable the Write Protect
        (*(volatile uint32_t*)(REG_BASE + OFFSET_WPMR)) = WPMR_KEY | WPMR_ENABLE;

        // Set up the DMA channels and related interrupts
        p->rxDMAChannel = DMA::newChannel(static_cast<DMA::Device>(static_cast<int>(DMA::Device::USART0_RX) + static_cast<int>(port)), DMA::Size::BYTE);
        p->txDMAChannel = DMA::newChannel(static_cast<DMA::Device>(static_cast<int>(DMA::Device::USART0_TX) + static_cast<int>(port)), DMA::Size::BYTE);
        _rxDMAChannelsToPorts[p->rxDMAChannel] = static_cast<int>(port);
        DMA::startChannel(p->rxDMAChannel, (uint32_t)(p->rxBuffer), BUFFER_SIZE);
        //DMA::reloadChannel(p->rxDMAChannel, (uint32_t)(p->rxBuffer), BUFFER_SIZE);
        DMA::enableInterrupt(p->rxDMAChannel, &rxBufferFullHandler, DMA::Interrupt::TRANSFER_FINISHED);

        _portsEnabled[static_cast<int>(port)] = true;
    }

    void disable(Port port) {
        // Don't do anything if this port is not enabled
        if (!_portsEnabled[static_cast<int>(port)]) {
            return;
        }
        _portsEnabled[static_cast<int>(port)] = false;

        const uint32_t REG_BASE = USART_BASE + static_cast<int>(port) * USART_REG_SIZE;
        struct USART* p = &(_ports[static_cast<int>(port)]);

        // Disable the DMA channels
        DMA::stopChannel(p->rxDMAChannel);
        DMA::stopChannel(p->txDMAChannel);

        // WPMR (Write Protect Mode Register) : disable the Write Protect
        (*(volatile uint32_t*)(REG_BASE + OFFSET_WPMR)) = WPMR_KEY | WPMR_DISABLE;

        // CR (Control Register) : disable RX and TX
        (*(volatile uint32_t*)(REG_BASE + OFFSET_CR))
            = 1 << CR_RXDIS
            | 1 << CR_TXDIS;

        // WPMR (Write Protect Mode Register) : re-enable the Write Protect
        (*(volatile uint32_t*)(REG_BASE + OFFSET_WPMR)) = WPMR_KEY | WPMR_ENABLE;

        // Disable the clock
        PM::disablePeripheralClock(PM_CLK[static_cast<int>(port)]);

        // Free the pins
        GPIO::disablePeripheral(PINS_RX[static_cast<int>(port)]);
        GPIO::disablePeripheral(PINS_TX[static_cast<int>(port)]);
        if (p->hardwareFlowControl) {
            GPIO::disablePeripheral(PINS_RTS[static_cast<int>(port)]);
            GPIO::disablePeripheral(PINS_CTS[static_cast<int>(port)]);
        }
    }

    void enableInterrupt(Port port, void (*handler)(), Interrupt interrupt) {
        const uint32_t REG_BASE = USART_BASE + static_cast<int>(port) * USART_REG_SIZE;

        // Save the user handler
        _interruptHandlers[static_cast<int>(port)][static_cast<int>(interrupt)] = (uint32_t)handler;

        // IER (Interrupt Enable Register) : enable the requested interrupt
        (*(volatile uint32_t*)(REG_BASE + OFFSET_IER))
            = 1 << IER_RXRDY;

        // Enable the interrupt in the NVIC
        Core::Interrupt interruptChannel = static_cast<Core::Interrupt>(static_cast<int>(Core::Interrupt::USART0) + static_cast<int>(port));
        Core::setInterruptHandler(interruptChannel, interruptHandlerWrapper);
        Core::enableInterrupt(interruptChannel, INTERRUPT_PRIORITY);
    }

    void interruptHandlerWrapper() {
        // Get the port number through the current interrupt number
        int port = static_cast<int>(Core::currentInterrupt()) - static_cast<int>(Core::Interrupt::USART0);
        const uint32_t REG_BASE = USART_BASE + port * USART_REG_SIZE;

        // Call the user handler of every interrupt that is enabled and pending
        for (int i = 0; i < N_INTERRUPTS; i++) {
            if ((*(volatile uint32_t*)(REG_BASE + OFFSET_IMR)) & (1 << _interruptBits[i]) // Interrupt is enabled
                    && (*(volatile uint32_t*)(REG_BASE + OFFSET_CSR)) & (1 << _interruptBits[i])) { // Interrupt is pending
                void (*handler)() = (void (*)())_interruptHandlers[port][i];
                if (handler != nullptr) {
                    handler();
                }
            }
        }

        // Clear the interrupts
        (*(volatile uint32_t*)(REG_BASE + OFFSET_CR))
            = 1 << CR_RSTSTA; // Reset status bits
    }

    void rxBufferFullHandler() {
        // Get the port that provoqued this interrupt
        int channel = static_cast<int>(Core::currentInterrupt()) - static_cast<int>(Core::Interrupt::DMA0);
        int portNumber = _rxDMAChannelsToPorts[channel];
        struct USART* p = &(_ports[portNumber]);

        // Reload the DMA channel
        int length = 0;
        int cur = p->rxBufferCursorW;
        if (p->rxBufferCursorR == p->rxBufferCursorW) {
            // Buffer is full, the DMA channel will be disabled
            // If hardware flow control is enabled this will pause the transfer by raising RTS,
            // otherwise if more data arrives it will trigger an overflow
            length = 0;

        } else if (p->rxBufferCursorR > p->rxBufferCursorW) {
            // Reload until the cursor
            length = p->rxBufferCursorR - p->rxBufferCursorW;
            p->rxBufferCursorW = p->rxBufferCursorR;

        } else { // p->rxBufferCursorR < p->rxBufferCursorW
            // Reload until the end of the linear buffer
            length = BUFFER_SIZE - p->rxBufferCursorW;
            p->rxBufferCursorW = 0;
        }

        if (length == 0) {
            DMA::stopChannel(p->rxDMAChannel);
        } else {
            DMA::reloadChannel(p->rxDMAChannel, (uint32_t)(p->rxBuffer + cur), length);
        }
    }

    int available(Port port) {
        struct USART* p = &(_ports[static_cast<int>(port)]);

        // Get the position of the second cursors from the DMA
        int cursor2 = p->rxBufferCursorW - DMA::getCounter(p->rxDMAChannel);

        // Get the position of the last char inserted into the buffer by DMA
        int length = cursor2 - p->rxBufferCursorR;
        if (length == 0) {
            // The buffer is either complety empty or complety full
            if (DMA::getCounter(p->rxDMAChannel) == 0) {
                // Complety full
                return BUFFER_SIZE;
            } else {
                // Complety empty
                return 0;
            }
        } else if (length > 0) {
            return length;
        } else {
            return BUFFER_SIZE + length; // length is negative here
        }
    }

    bool contains(Port port, char byte) {
        struct USART* p = &(_ports[static_cast<int>(port)]);

        // Check if the received buffer contains the specified byte
        int avail = available(port);
        for (int i = 0; i < avail; i++) {
            if (p->rxBuffer[(p->rxBufferCursorR + i) % BUFFER_SIZE] == byte) {
                return true;
            }
        }
        return false;
    }

    char peek(Port port) {
        struct USART* p = &(_ports[static_cast<int>(port)]);

        if (available(port) > 0) {
            // Return the next char in the port's RX buffer
            return p->rxBuffer[p->rxBufferCursorR];
        } else {
            return 0;
        }
    }

    bool peek(Port port, const char* test, int size) {
        struct USART* p = &(_ports[static_cast<int>(port)]);

        if (available(port) >= size) {
            // Check whether the /size/ next chars in the buffer are equal to test
            for (int i = 0; i < size; i++) {
                if (p->rxBuffer[(p->rxBufferCursorR + i) % BUFFER_SIZE] != test[i]) {
                    return false;
                }
            }
            return true;
        } else {
            return false;
        }
    }

    // Read one byte
    char read(Port port) {
        struct USART* p = &(_ports[static_cast<int>(port)]);

        if (available(port) > 0) {
            // Read the next char in the port's Rx buffer
            char c = p->rxBuffer[p->rxBufferCursorR];

            // Increment the cursor
            p->rxBufferCursorR++;
            if (p->rxBufferCursorR == BUFFER_SIZE) {
                p->rxBufferCursorR = 0;
            }

            // If the Rx DMA channel was stopped because the buffer was full,
            // there is now room available, so it can be restarted
            if (!DMA::isEnabled(p->rxDMAChannel)) {
                int cur = p->rxBufferCursorW;
                p->rxBufferCursorW++;
                if (p->rxBufferCursorW == BUFFER_SIZE) {
                    p->rxBufferCursorW = 0;
                }
                DMA::startChannel(p->rxDMAChannel, (uint32_t)(p->rxBuffer + cur), 1);
            }
            
            // Return the character that was read
            return c;

        } else {
            return 0;
        }
    }

    // Read up to /size/ bytes
    // If buffer is nullptr, the bytes are simply poped from the buffer
    int read(Port port, char* buffer, int size, bool readUntil, char end) {
        struct USART* p = &(_ports[static_cast<int>(port)]);

        int avail = available(port);
        int n = 0;
        for (int i = 0; i < size && i < avail; i++) {
            // Copy the next char from the port's Rx buffer to the user buffer
            char c = p->rxBuffer[p->rxBufferCursorR];
            if (buffer != nullptr) {
                buffer[i] = c;
            }

            // Keep track of the number of bytes written
            n++;

            // Increment the cursor
            p->rxBufferCursorR++;
            if (p->rxBufferCursorR == BUFFER_SIZE) {
                p->rxBufferCursorR = 0;
            }
            
            // If the "read until" mode is selected, exit the loop if the selected byte is found
            if (readUntil && c == end) {
                break;
            }
        }

        // If the Rx DMA channel was stopped because the buffer was full,
        // there is now room available, so it can be restarted
        if (!DMA::isEnabled(p->rxDMAChannel)) {
            int cur = p->rxBufferCursorW;
            int length = 0;
            if (p->rxBufferCursorR > p->rxBufferCursorW) {
                length = p->rxBufferCursorR - p->rxBufferCursorW;
            } else {
                length = BUFFER_SIZE - p->rxBufferCursorW;
            }
            DMA::startChannel(p->rxDMAChannel, (uint32_t)(p->rxBuffer + cur), length);
        }

        return n;
    }

    // Read up to n bytes until the specified byte is found
    int readUntil(Port port, char* buffer, int size, char end) {
        return read(port, buffer, size, true, end);
    }

    // Read an int on n bytes (LSByte first, max n = 8 bytes)
    // and return it as an unsigned long
    unsigned long readInt(Port port, int nBytes, bool wait) {
        if (nBytes > 8) {
            nBytes = 8;
        }

        if (wait) {
            while (available(port) < nBytes);
        }

        unsigned long result = 0;
        for (int i = 0; i < nBytes; i++) {
            uint8_t c = read(port);
            result |= c << (i * 8);
        }
        return result;
    }

    int write(Port port, const char* buffer, int size, bool async) {
        struct USART* p = &(_ports[static_cast<int>(port)]);

        // Wait until any previous write is finished
        waitWriteFinished(port);

        // If size is not specified, write at most BUFFER_SIZE characters
        int n = size;
        if (n < 0 || n > BUFFER_SIZE) {
            n = BUFFER_SIZE;
        }

        // Copy the user buffer into the Tx buffer
        for (int i = 0; i < n; i++) {
            // If size is not specified, stops at the end of the string
            if (size < 0 && buffer[i] == 0) {
                n = i;
                break;
            }
            p->txBuffer[i] = buffer[i];
        }

        // Start the DMA
        DMA::startChannel(p->txDMAChannel, (uint32_t)(p->txBuffer), n);

        // In synchronous mode, wait until this transfer is finished
        if (!async) {
            waitWriteFinished(port);
        }

        // Return the number of bytes written
        return n;
    }

    int write(Port port, char byte, bool async) {
        struct USART* p = &(_ports[static_cast<int>(port)]);

        // Wait until any previous write is finished
        waitWriteFinished(port);
        
        // Write a single byte
        p->txBuffer[0] = byte;
        DMA::startChannel(p->txDMAChannel, (uint32_t)(p->txBuffer), 1);

        // In synchronous mode, wait until this transfer is finished
        if (!async) {
            waitWriteFinished(port);
        }

        return 1;
    }

    int write(Port port, int number, uint8_t base, bool async) {
        // Write a human-readable number in the given base
        const int bufferSize = 32; // Enough to write a 32-bits word in binary
        char buffer[bufferSize];
        int cursor = 0;
        if (base < 2) {
            return 0;
        }

        // Special case : number = 0
        if (number == 0) {
            return write(port, '0');
        }

        // Minus sign
        if (number < 0) {
            buffer[cursor] = '-';
            cursor++;
            number = -number;
        }

        // Compute the number in reverse
        int start = cursor;
        for (; cursor < bufferSize && number > 0; cursor++) {
            char c = number % base;
            if (c < 10) {
                c += '0';
            } else {
                c += 'A' - 10;
            }
            buffer[cursor] = c;
            number = number / base;
        }

        // Reverse the result
        for (int i = 0; i < (cursor - start) / 2; i++) {
            char c = buffer[start + i];
            buffer[start + i] = buffer[cursor - i - 1];
            buffer[cursor - i - 1] = c;
        }

        buffer[cursor] = 0;
        cursor++;
        return write(port, buffer, cursor, async);
    }

    int write(Port port, bool boolean, bool async) {
        // Write a boolean value
        if (boolean) {
            return write(port, "true", 4, async);
        } else {
            return write(port, "false", 5, async);
        }
    }

    int writeLine(Port port, const char* buffer, int size, bool async) {
        int written = write(port, buffer, size, async);
        written += write(port, "\r\n", 2, async);
        return written;
    }

    int writeLine(Port port, char byte, bool async) {
        int written = write(port, byte, async);
        written += write(port, "\r\n", 2, async);
        return written;
    }

    int writeLine(Port port, int number, uint8_t base, bool async) {
        int written = write(port, number, base, async);
        written += write(port, "\r\n", 2, async);
        return written;
    }

    int writeLine(Port port, bool boolean, bool async) {
        int written = write(port, boolean, async);
        written += write(port, "\r\n", 2, async);
        return written;
    }

    void flush(Port port) {
        struct USART* p = &(_ports[static_cast<int>(port)]);

        // Stop the Rx channel
        DMA::stopChannel(p->rxDMAChannel);

        // Empty the reception buffer
        p->rxBufferCursorR = 0;
        p->rxBufferCursorW = 0;

        // Start the channel again
        DMA::startChannel(p->rxDMAChannel, (uint32_t)(p->rxBuffer), BUFFER_SIZE);
    }

    bool isWriteFinished(Port port) {
        const uint32_t REG_BASE = USART_BASE + static_cast<int>(port) * USART_REG_SIZE;
        struct USART* p = &(_ports[static_cast<int>(port)]);

        // Check if the DMA has finished the transfer and if the Tx buffer is empty
        return DMA::isFinished(p->txDMAChannel) && (*(volatile uint32_t*)(REG_BASE + OFFSET_CSR)) & (1 << CSR_TXEMPTY);
    }

    void waitWriteFinished(Port port) {
        // Wait for the transfer to finish
        while (!isWriteFinished(port));
    }

    void waitReadFinished(Port port, unsigned long timeout) {
        // Wait until no more bytes is received
        struct USART* p = &(_ports[static_cast<int>(port)]);
        unsigned long tStart = Core::time();
        int n = available(port);
        while (n == 0) {
            n = available(port);
            if (timeout > 0 && Core::time() - tStart > timeout) {
                return;
            }
        }
        unsigned long byteDuration = 1000000UL / p->baudrate * 8; // in us
        while (1) {
            Core::waitMicroseconds(5 * byteDuration); // Wait for 5 times the duration of a byte
            int n2 = available(port);
            if (n2 == n) {
                // No byte received during the delay, the transfer looks finished
                return;
            }
            n = n2;
        }
    }

    void setPin(Port port, PinFunction function, GPIO::Pin pin) {
        switch (function) {
            case PinFunction::RX:
                PINS_RX[static_cast<int>(port)] = pin;
                break;

            case PinFunction::TX:
                PINS_TX[static_cast<int>(port)] = pin;
                break;

            case PinFunction::RTS:
                PINS_RTS[static_cast<int>(port)] = pin;
                break;

            case PinFunction::CTS:
                PINS_CTS[static_cast<int>(port)] = pin;
                break;
        }
    }

}