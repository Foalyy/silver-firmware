#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_

#include <stdint.h>

class RingBuffer {
private:
    uint8_t* _buffer = nullptr;
    unsigned int _capacity = 0;
    unsigned int _cursorR = 0;
    unsigned int _cursorW = 0;
    bool _empty = true;
    bool _overflow = false;
    bool _underflow = false;

public:
    // Constructor : must be passed the buffer to use
    RingBuffer(uint8_t* buffer, unsigned int capacity);

    // Get the value of a random byte in the buffer, without
    // changing the cursors
    uint8_t operator[](unsigned int i) const;

    // Read one byte from the internal buffer
    uint8_t read();

    // Read some bytes from the internal buffer
    int read(uint8_t* buffer, unsigned int size);

    // Write one byte into the internal buffer
    void write(uint8_t byte);

    // Write some bytes into the internal buffer
    void write(const uint8_t* buffer, unsigned int size);

    // Number of bytes currently stored in the buffer
    unsigned int size() const;

    // Check if the buffer is empty
    inline bool isEmpty() { return size() == 0; }

    // Check if the specified byte is in the buffer
    int contains(uint8_t byte) const;

    // Internal buffer total capacity
    unsigned int capacity() const;

    // Return true if the internal buffer is overflown
    // (too many writes, not enough reads)
    bool isOverflow() const;

    // Return true if the user attempted to read more bytes than
    // were available
    bool isUnderflow() const;

    // Revert the ring buffer to its initial state : 
    // cursors and overflow/underflow flags are reset
    void reset();

};

#endif