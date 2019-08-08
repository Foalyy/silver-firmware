#include "oled.h"
#include <string.h>
#include <gpio.h>

namespace OLED {

    SPI::Peripheral _spi = 0;

    Rotation _rotation;

    // Display buffer
    uint8_t _displayBuffer[DISPLAY_BUFFER_SIZE];
    uint8_t _displayBufferDirty[WIDTH]; // 1 bit per page

    // Print cursor
    Font::Size _size = Font::Size::MEDIUM;
    unsigned int _cursorX = 0;
    unsigned int _cursorY = 0;
    bool _inverted = false;

    GPIO::Pin _pinDC;
    GPIO::Pin _pinRES;

    void initScreen(SPI::Peripheral spi, GPIO::Pin pinDC, GPIO::Pin pinRES) {
        _spi = spi;
        _pinDC = pinDC;
        _pinRES = pinRES;
        GPIO::enableOutput(_pinDC, GPIO::LOW);
        GPIO::enableOutput(_pinRES, GPIO::HIGH);
        SPI::addPeripheral(_spi);

        // Disable the screen
        disable();

        // Enable the charge pump
        sendCommand(CMD_CHARGE_PUMP);
        sendCommand(0x14);

        // Set horizontal addressing mode
        sendCommand(CMD_ADDRESSING_MODE);
        sendCommand(0x00); // Horizontal addressing mode

        // Set contrast
        setContrast(0x70);
        setInverted(false);

        // Initialize buffer
        clear();
        refresh();

        // Enable the screen
        enable();
    }

    // Turn the screen on
    void enable() {
        sendCommand(CMD_DISPLAY_ON);
    }

    // Turn the screen off
    void disable() {
        sendCommand(CMD_DISPLAY_OFF);
    }

    void setContrast(uint8_t contrast) {
        sendCommand(CMD_CONTRAST);
        sendCommand(contrast);
    }

    // Send a command to the screen's driver
    void sendCommand(uint8_t command) {
        uint8_t buffer[] = {command};
        GPIO::set(_pinDC, GPIO::LOW);
        SPI::transfer(_spi, buffer, 1);
    }

    // Send the data of a row between two x positions
    void sendData(int page, int xStart, int xEnd) {
        if (page < 0 || page >= N_PAGES
                || xStart < 0 || xStart >= WIDTH
                || xEnd < 0 || xEnd >= WIDTH
                || xEnd < xStart) {
            return;
        }

        // Set display zone
        sendCommand(CMD_COLUMN_START_END);
        sendCommand(xStart);
        sendCommand(xEnd);

        sendCommand(CMD_PAGE_START_END);
        sendCommand(page);
        sendCommand(page);

        const int BUFFER_SIZE = 32;
        uint8_t buffer[BUFFER_SIZE];
        int cursor = xStart;
        GPIO::set(_pinDC, GPIO::HIGH);
        while (cursor <= xEnd) {
            int l = xEnd - cursor + 1;
            if (l > BUFFER_SIZE) {
                l = BUFFER_SIZE;
            }
            memcpy(buffer, _displayBuffer + page * WIDTH + cursor, l);
            SPI::transfer(_spi, buffer, l);
            cursor += l;
        }
    }

    // Update the screen by sending all the row segments that have been modified
    // since the last update
    void refresh() {
        for (int page = 0; page < N_PAGES; page++) {
            int start = -1;
            int end = -1;
            for (int x = 0; x < WIDTH; x++) {
                bool dirty = _displayBufferDirty[x] & (1 << page);
                if (start == -1 && dirty) {
                    start = x;
                } else if (start >= 0 && !dirty) {
                    end = x - 1;
                    sendData(page, start, end);
                    start = -1;
                    end = -1;
                }
            }
            if (start >= 0 && end == -1) {
                end = WIDTH - 1;
                sendData(page, start, end);
            }
        }
        memset(_displayBufferDirty, 0x00, WIDTH);
    }

    // Invert the screen (black on white instead of white on black)
    void setColorInverted(bool inverted) {
        if (inverted) {
            sendCommand(CMD_DISPLAY_INVERTED);
        } else {
            sendCommand(CMD_DISPLAY_NORMAL);
        }
    }

    // Clear the whole screen
    void clear() {
        memset(_displayBuffer, 0, DISPLAY_BUFFER_SIZE);
        memset(_displayBufferDirty, 0xFF, WIDTH);
        setCursor(0, 0);
    }

    // Clear a specific region of the screen
    void clear(unsigned int x, unsigned int y, unsigned int width, unsigned int height) {
        const unsigned int pageStart = y / 8;
        const unsigned int yOffsetStart = y - pageStart * 8;
        const unsigned int pageEnd = (y + height) / 8;
        const unsigned int yOffsetEnd = (y + height) - pageEnd * 8;
        for (unsigned int i = 0; i < width; i++) {
            if (x + i >= WIDTH) {
                break;
            }

            if (pageStart == pageEnd) {
                _displayBuffer[pageStart * WIDTH + x + i] &= (0xFF >> (8 - yOffsetStart) | 0xFF << yOffsetEnd);
                _displayBufferDirty[x + i] |= 1 << pageStart;

            } else {
                _displayBuffer[pageStart * WIDTH + x + i] &= 0xFF >> (8 - yOffsetStart);
                _displayBufferDirty[x + i] |= 1 << pageStart;
                for (unsigned int j = pageStart + 1; j < pageEnd; j++) {
                    _displayBuffer[j * WIDTH + x + i] = 0x00;
                    _displayBufferDirty[x + i] |= 1 << j;
                }
                _displayBuffer[pageEnd * WIDTH + x + i] &= 0xFF << yOffsetEnd;
                _displayBufferDirty[x + i] |= 1 << pageEnd;
            }
        }
    }

    // Modify a single pixel
    void setPixel(unsigned int x, unsigned int y, bool on) {
        const unsigned int page = y / 8;
        const unsigned int yOffset = y - page * 8;
        if (x < WIDTH && y < HEIGHT) {
            if (on) {
                _displayBuffer[page * WIDTH + x] |= (1 << yOffset);
            } else {
                _displayBuffer[page * WIDTH + x] &= ~(uint8_t)(1 << yOffset);
            }
            _displayBufferDirty[x] |= 1 << page;
        }
    }

    void rect(unsigned int x, unsigned int y, unsigned int width, unsigned int height, bool on) {
        for (unsigned int _x = x; _x < x + width; _x++) {
            for (unsigned int _y = y; _y < y + height; _y++) {
                setPixel(_x, _y, on);
            }
        }
    }

    // Print a single character at the current cursor position using the current
    // font size
    void print(char character) {
        print(_cursorX, _cursorY, character);
    }

    // Print a single character at the specified position using the current
    // font size
    void print(unsigned int x, unsigned int y, char character) {
        _cursorX = x;
        _cursorY = y;
        switch (_size) {
            case Font::Size::SMALL:
                printSmall(character);
                break;

            case Font::Size::MEDIUM:
                printMedium(character);
                break;

            case Font::Size::LARGE:
                printLarge(character);
                break;

            default:
                break;
        }
    }

    // Print some text at the current cursor position using the current font
    // size
    void print(const char* text) {
        print(_cursorX, _cursorY, text);
    }

    // Print some text at the specified position using the current font
    // size
    void print(unsigned int x, unsigned int y, const char* text) {
        int i = 0;
        char c = text[i];
        _cursorX = x;
        _cursorY = y;
        unsigned int cursorXInit = x;
        while (c != 0) {
            if (c == '\n') {
                _cursorX = cursorXInit;
                _cursorY += Font::HEIGHT[static_cast<int>(_size)] + 1;
            } else {
                print(_cursorX, _cursorY, c);
            }
            i++;
            c = text[i];
        }
    }

    // Print some text centered on the specified x position
    void printCentered(unsigned int x, unsigned int y, const char* text) {
        int width = textWidth(text);
        if (x - width / 2 >= 0) {
            x -= width / 2;
        } else {
            x = 0;
        }
        print(x, y, text);
    }

    // Return the width in pixels of the specified character
    int charWidth(char c) {
        if (_size == Font::Size::SMALL) {
            return Font::getSmall(c).width;
        } else if (_size == Font::Size::MEDIUM) {
            return Font::getMedium(c).width;
        } else if (_size == Font::Size::LARGE) {
            return Font::getLarge(c).width;
        }
        return 0;
    }

    // Return the width in pixels of the specified text
    int textWidth(const char* text) {
        int i = 0;
        char c = text[i];
        unsigned int maxWidth = 0;
        unsigned int width = 0;
        while (c != 0) {
            if (c == '\n') {
                if (width > maxWidth) {
                    maxWidth = width;
                }
                width = 0;
            } else {
                width += charWidth(c) + 1;
            }
            i++;
            c = text[i];
        }
        if (width > maxWidth) {
            maxWidth = width;
        }
        if (maxWidth > 0) {
            maxWidth--;
        }
        return maxWidth;
    }

    // Print a signed number in the specified base
    void printInt(int value, int base) {
        printInt(_cursorX, _cursorY, value, base);
    }

    // Print a signed number in the specified base at the specified position
    void printInt(unsigned int x, unsigned int y, int value, int base) {
        _cursorX = x;
        _cursorY = y;
        
        const int N = 10;
        char buffer[N];
        int i = 0;
        bool neg = false;
        if (value == 0) {
            buffer[0] = '0';
        } else {
            if (value < 0) {
                value = -value;
                neg = true;
            }
            for (; i < N; i++) {
                int d = value % base;
                value /= base;
                if (d <= 9) {
                    buffer[i] = '0' + d;
                } else {
                    buffer[i] = 'A' + d - 10;
                }
                if (value == 0) {
                    break;
                }
            }
        }
        if (neg) {
            print('-');
        }
        for (int j = i; j >= 0; j--) {
            print(buffer[j]);
        }
    }

    // Print a single character at the current cursor position using
    // the small font size
    void printSmall(char character) {
        Font::Char5 c = Font::getSmall(character);
        if (_cursorX + c.width >= WIDTH) {
            _cursorX = 1;
            _cursorY += c.height + 1;
        }
        printSmall(_cursorX, _cursorY, c);
        _cursorX += c.width + 1;
    }

    // Print a single character at the specified position using the small font size
    void printSmall(unsigned int x, unsigned int y, char character) {
        printSmall(x, y, Font::getSmall(character));
    }

    // Print a single symbol at the specified position using the small font size
    void printSmall(unsigned int x, unsigned int y, const Font::Char5 c) {
        const unsigned int page = y / 8;
        const unsigned int yOffset = y - page * 8;
        for (unsigned int i = 0; i < c.width; i++) {
            if (x + i >= WIDTH) {
                break;
            }
            if (page < N_PAGES) {
                if (_inverted) {
                    _displayBuffer[page * WIDTH + x + i] &= ~(uint8_t)(c.bitmap[i] << yOffset);
                } else {
                    _displayBuffer[page * WIDTH + x + i] |= (c.bitmap[i] << yOffset);
                }
                _displayBufferDirty[x + i] |= 1 << page;
            }
            if (yOffset + c.height > 8 && page + 1 < N_PAGES) {
                if (_inverted) {
                    _displayBuffer[(page + 1) * WIDTH + x + i] &= ~(uint8_t)(c.bitmap[i] >> (8 - yOffset));
                } else {
                    _displayBuffer[(page + 1) * WIDTH + x + i] |= (c.bitmap[i] >> (8 - yOffset));
                }
                _displayBufferDirty[x + i] |= 1 << (page + 1);
            }
        }
    }

    // Print a single character at the current cursor position using
    // the medium font size
    void printMedium(char character) {
        Font::Char8 c = Font::getMedium(character);
        if (_cursorX + c.width >= WIDTH) {
            _cursorX = 1;
            _cursorY += c.height + 1;
        }
        printMedium(_cursorX, _cursorY, c);
        _cursorX += c.width + 1;
    }

    // Print a single character at the specified position using the medium font size
    void printMedium(unsigned int x, unsigned int y, char character) {
        printMedium(x, y, Font::getMedium(character));
    }

    // Print a single symbol at the specified position using the medium font size
    void printMedium(unsigned int x, unsigned int y, const Font::Char8 c) {
        const unsigned int page = y / 8;
        const unsigned int yOffset = y - page * 8;
        for (unsigned int i = 0; i < c.width; i++) {
            if (x + i >= WIDTH) {
                break;
            }
            if (page < N_PAGES) {
                if (_inverted) {
                    _displayBuffer[page * WIDTH + x + i] &= ~(uint8_t)(c.bitmap[i] << yOffset);
                } else {
                    _displayBuffer[page * WIDTH + x + i] |= (c.bitmap[i] << yOffset);
                }
                _displayBufferDirty[x + i] |= 1 << page;
            }
            if (yOffset + c.height > 8 && page + 1 < N_PAGES) {
                if (_inverted) {
                    _displayBuffer[(page + 1) * WIDTH + x + i] &= ~(uint8_t)(c.bitmap[i] >> (8 - yOffset));
                } else {
                    _displayBuffer[(page + 1) * WIDTH + x + i] |= (c.bitmap[i] >> (8 - yOffset));
                }
                _displayBufferDirty[x + i] |= 1 << (page + 1);
            }
        }
    }

    // Print a single character at the current cursor position using
    // the large font size
    void printLarge(char character) {
        Font::Char16 c = Font::getLarge(character);
        if (_cursorX + c.width >= WIDTH) {
            _cursorX = 1;
            _cursorY += c.height + 1;
        }
        printLarge(_cursorX, _cursorY, c);
        _cursorX += c.width + 2;
    }

    // Print a single character at the specified position using the large font size
    void printLarge(unsigned int x, unsigned int y, char character) {
        printLarge(x, y, Font::getLarge(character));
    }
    
    // Print a single symbol at the specified position using the large font size
    void printLarge(unsigned int x, unsigned int y, const Font::Char16 c) {
        const unsigned int page = y / 8;
        const unsigned int yOffset = y - page * 8;
        for (unsigned int i = 0; i < c.width; i++) {
            if (x + i >= WIDTH) {
                break;
            }
            if (page < N_PAGES) {
                _displayBuffer[page * WIDTH + x + i] |= (c.bitmap[i] << yOffset);
                _displayBufferDirty[x + i] |= 1 << page;
            }
            if (yOffset + c.height > 8 && page + 1 < N_PAGES) {
                _displayBuffer[(page + 1) * WIDTH + x + i] |= (c.bitmap[i] >> (8 - yOffset));
                _displayBufferDirty[x + i] |= 1 << (page + 1);
            }
            if (yOffset + c.height > 16 && page + 2 < N_PAGES) {
                _displayBuffer[(page + 2) * WIDTH + x + i] |= (c.bitmap[i] >> (16 - yOffset));
                _displayBufferDirty[x + i] |= 1 << (page + 2);
            }
        }
    }
    
    // Print a single symbol at the specified position using the xlarge font size
    void printXLarge(unsigned int x, unsigned int y, const Font::Char32 c) {
        const unsigned int page = y / 8;
        const unsigned int yOffset = y - page * 8;
        for (unsigned int i = 0; i < c.width; i++) {
            if (x + i >= WIDTH) {
                break;
            }
            if (page < N_PAGES) {
                _displayBuffer[page * WIDTH + x + i] |= (c.bitmap[i] << yOffset);
                _displayBufferDirty[x + i] |= 1 << page;
            }
            if (yOffset + c.height > 8 && page + 1 < N_PAGES) {
                _displayBuffer[(page + 1) * WIDTH + x + i] |= (c.bitmap[i] >> (8 - yOffset));
                _displayBufferDirty[x + i] |= 1 << (page + 1);
            }
            if (yOffset + c.height > 16 && page + 2 < N_PAGES) {
                _displayBuffer[(page + 2) * WIDTH + x + i] |= (c.bitmap[i] >> (16 - yOffset));
                _displayBufferDirty[x + i] |= 1 << (page + 2);
            }
            if (yOffset + c.height > 24 && page + 3 < N_PAGES) {
                _displayBuffer[(page + 3) * WIDTH + x + i] |= (c.bitmap[i] >> (24 - yOffset));
                _displayBufferDirty[x + i] |= 1 << (page + 3);
            }
            if (yOffset + c.height > 32 && page + 4 < N_PAGES) {
                _displayBuffer[(page + 4) * WIDTH + x + i] |= (c.bitmap[i] >> (32 - yOffset));
                _displayBufferDirty[x + i] |= 1 << (page + 4);
            }
        }
    }
    
    // Print a single symbol at the specified position using the xxlarge font size
    void printXXLarge(unsigned int x, unsigned int y, const Font::Char64 c) {
        const unsigned int page = y / 8;
        const unsigned int yOffset = y - page * 8;
        for (unsigned int i = 0; i < c.width; i++) {
            if (x + i >= WIDTH) {
                break;
            }
            if (page < N_PAGES) {
                _displayBuffer[page * WIDTH + x + i] |= (c.bitmap[i] << yOffset);
                _displayBufferDirty[x + i] |= 1 << page;
            }
            if (yOffset + c.height > 8 && page + 1 < N_PAGES) {
                _displayBuffer[(page + 1) * WIDTH + x + i] |= (c.bitmap[i] >> (8 - yOffset));
                _displayBufferDirty[x + i] |= 1 << (page + 1);
            }
            if (yOffset + c.height > 16 && page + 2 < N_PAGES) {
                _displayBuffer[(page + 2) * WIDTH + x + i] |= (c.bitmap[i] >> (16 - yOffset));
                _displayBufferDirty[x + i] |= 1 << (page + 2);
            }
            if (yOffset + c.height > 24 && page + 3 < N_PAGES) {
                _displayBuffer[(page + 3) * WIDTH + x + i] |= (c.bitmap[i] >> (24 - yOffset));
                _displayBufferDirty[x + i] |= 1 << (page + 3);
            }
            if (yOffset + c.height > 32 && page + 4 < N_PAGES) {
                _displayBuffer[(page + 4) * WIDTH + x + i] |= (c.bitmap[i] >> (32 - yOffset));
                _displayBufferDirty[x + i] |= 1 << (page + 4);
            }
            if (yOffset + c.height > 40 && page + 5 < N_PAGES) {
                _displayBuffer[(page + 5) * WIDTH + x + i] |= (c.bitmap[i] >> (40 - yOffset));
                _displayBufferDirty[x + i] |= 1 << (page + 5);
            }
            if (yOffset + c.height > 48 && page + 6 < N_PAGES) {
                _displayBuffer[(page + 6) * WIDTH + x + i] |= (c.bitmap[i] >> (48 - yOffset));
                _displayBufferDirty[x + i] |= 1 << (page + 6);
            }
            if (yOffset + c.height > 56 && page + 7 < N_PAGES) {
                _displayBuffer[(page + 7) * WIDTH + x + i] |= (c.bitmap[i] >> (56 - yOffset));
                _displayBufferDirty[x + i] |= 1 << (page + 7);
            }
            if (yOffset + c.height > 64 && page + 8 < N_PAGES) {
                _displayBuffer[(page + 8) * WIDTH + x + i] |= (c.bitmap[i] >> (64 - yOffset));
                _displayBufferDirty[x + i] |= 1 << (page + 8);
            }
        }
    }

    // Draw a progress bar
    void progressbar(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char percent) {
        if (width < 5 || height < 3) {
            return;
        }

        unsigned int percentwidth100 = width - 2;
        if (height > 4) {
            percentwidth100 -= 2;
        }
        unsigned int percentwidth = percent * percentwidth100 / 100;

        for (unsigned int y_ = 0; y_ < height; y_++) {
            if (height > 4 && (y_ == 0 || y_ == height - 1)) {
                continue;
            }
            setPixel(x, y + y_);
        }
        for (unsigned int x_ = 1; x_ < width - 1; x_++) {
            if (height > 4 && (x_ == 1 || x_ == width - 2)) {
                continue;
            }
            setPixel(x + x_, y);
            if (x_ - (height <= 4 ? 0 : 1) <= percentwidth) {
                for (unsigned int y_ = 1; y_ < height - 1; y_++) {
                    if (height > 4 && (y_ == 1 || y_ == height - 2)) {
                        continue;
                    }
                    setPixel(x + x_, y + y_);
                }
            }
            setPixel(x + x_, y + height - 1);
        }
        for (unsigned int y_ = 0; y_ < height; y_++) {
            if (height > 4 && (y_ == 0 || y_ == height - 1)) {
                continue;
            }
            setPixel(x + width - 1, y + y_);
        }
        if (height >= 5) {
            setPixel(x + 1, y);
            setPixel(x + width - 2, y);
            setPixel(x + 1, y + height - 1);
            setPixel(x + width - 2, y + height - 1);
        }
    }

    // Draw a button
    void button(unsigned int x, unsigned int y, unsigned int width, unsigned int height, const char* label, bool selected, bool pressed, bool arrowLeft, bool arrowRight) {
        if (width < 5 || height < 5) {
            return;
        }

        // Background
        if (selected || pressed) {
            for (unsigned int x_ = 0; x_ < width; x_++) {
                if (pressed) {
                    if (x_ == 0 || x_ == width - 1) {
                        for (unsigned int y_ = 2; y_ < height - 2; y_++) {
                            setPixel(x + x_, y + y_);
                        }
                    } else if (x_ == 1 || x_ == width - 2) {
                        for (unsigned int y_ = 1; y_ < height - 1; y_++) {
                            setPixel(x + x_, y + y_);
                        }
                    } else {
                        for (unsigned int y_ = 0; y_ < height; y_++) {
                            setPixel(x + x_, y + y_);
                        }
                    }
                } else {
                    if (x_ == 0 || x_ == width - 1) {
                        for (unsigned int y_ = 2; y_ < height - 2; y_++) {
                            setPixel(x + x_, y + y_);
                        }
                    } else if (x_ == 1 || x_ == width - 2) {
                        setPixel(x + x_, y + 1);
                        setPixel(x + x_, y + height - 2);
                    } else {
                        setPixel(x + x_, y);
                        setPixel(x + x_, y + height - 1);
                    }
                }
            }
        }

        bool savedInverted = _inverted;
        _inverted = pressed;

        // Label
        int labelHeight = 0;
        if (_size == Font::Size::SMALL) {
            labelHeight = 5;
        } else if (_size == Font::Size::MEDIUM) {
            labelHeight = 8;
        } else if (_size == Font::Size::LARGE) {
            labelHeight = 16;
        }
        printCentered(x + width / 2, y + (height - labelHeight) / 2, label);

        // Arrows
        if (arrowLeft) {
            for (int x_ = 0; x_ < 3; x_++) {
                setPixel(x + 5 + x_, y + height / 2 - x_);
                setPixel(x + 5 + x_, y + height / 2 + x_);
            }
        }
        if (arrowRight) {
            for (int x_ = 0; x_ < 3; x_++) {
                setPixel(x + width - 5 - x_, y + height / 2 - x_);
                setPixel(x + width - 5 - x_, y + height / 2 + x_);
            }
        }

        _inverted = savedInverted;
    }

    // Draw a checkbox
    void checkbox(unsigned int x, unsigned int y, unsigned int width, unsigned int height, const char* label, bool selected, bool pressed, bool checked) {
        // Draw base button
        button(x, y, width, height, label, selected, pressed);

        // Draw checkbox
        bool savedInverted = _inverted;
        _inverted = pressed;
        const int margin = 2;
        int l = height - 2 - 2 * margin;
        for (int x_ = 0; x_ < l; x_++) {
            if (x_ == 0 || x_ == l - 1) {
                for (int y_ = 0; y_ < l; y_++) {
                    setPixel(x + 1 + margin + x_, y + 1 + margin + y_, !pressed);
                }
            } else if (x_ > 0 && x_ < l - 1) {
                setPixel(x + 1 + margin + x_, y + 1 + margin, !pressed);
                setPixel(x + 1 + margin + x_, y + 1 + margin + l - 1, !pressed);
                if (checked && x_ > 1 && x_ < l - 2) {
                    for (int y_ = 2; y_ < l - 2; y_++) {
                        setPixel(x + 1 + margin + x_, y + 1 + margin + y_, !pressed);
                    }
                }
            }
        }
        _inverted = savedInverted;
    }

    unsigned int cursorX() {
        return _cursorX;
    }

    unsigned int cursorY() {
        return _cursorY;
    }

    void setCursor(unsigned int x, unsigned int y) {
        _cursorX = x;
        _cursorY = y;
    }

    void setCursorX(unsigned int x) {
        _cursorX = x;
    }

    void setCursorY(unsigned int y) {
        _cursorY = y;
    }

    void moveCursor(unsigned int x, unsigned int y) {
        _cursorX += x;
        _cursorY += y;
    }

    void moveCursorX(unsigned int x) {
        _cursorX += x;
    }

    void moveCursorY(unsigned int y) {
        _cursorY += y;
    }

    void setSize(Font::Size size) {
        _size = size;
    }

    void setInverted(bool inverted) {
        _inverted = inverted;
    }

    // Rotate the screen to accomodate its mounting direction
    void setRotation(Rotation rotation) {
        _rotation = rotation;
        if (rotation == Rotation::R180 || rotation == Rotation::R270) {
            sendCommand(CMD_SEGMENT_REMAP_ON);
            sendCommand(CMD_COM_SCAN_DIRECTION_INVERT);
        } else {
            sendCommand(CMD_SEGMENT_REMAP_OFF);
            sendCommand(CMD_COM_SCAN_DIRECTION_NORMAL);
        }
    }

    unsigned int getFontHeight() {
        return getFontHeight(_size);
    }

    unsigned int getFontHeight(Font::Size size) {
        return Font::HEIGHT[static_cast<int>(size)];
    }

    unsigned int getFontWidth() {
        return getFontWidth(_size);
    }

    unsigned int getFontWidth(Font::Size size) {
        return Font::WIDTH[static_cast<int>(size)];
    }

}