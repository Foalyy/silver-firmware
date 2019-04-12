#ifndef _OLED_H_
#define _OLED_H_

#include <spi.h>
#include "font.h"

namespace OLED {

    // Display buffer
    const int WIDTH = 128;
    const int HEIGHT = 64;
    const int N_PAGES = HEIGHT / 8;
    const int DISPLAY_BUFFER_SIZE = N_PAGES * WIDTH;

    // Commands
    const uint8_t CMD_DISPLAY_OFF = 0xAE;
    const uint8_t CMD_DISPLAY_ON = 0xAF;
    const uint8_t CMD_DISPLAY_NORMAL = 0xA6;
    const uint8_t CMD_DISPLAY_INVERTED = 0xA7;
    const uint8_t CMD_CONTRAST = 0x81;
    const uint8_t CMD_CHARGE_PUMP = 0x8D;
    const uint8_t CMD_ADDRESSING_MODE = 0x20;
    const uint8_t CMD_COLUMN_START_END = 0x21;
    const uint8_t CMD_PAGE_START_END = 0x22;
    const uint8_t CMD_SEGMENT_REMAP_OFF = 0xA0;
    const uint8_t CMD_SEGMENT_REMAP_ON = 0xA1;
    const uint8_t CMD_COM_SCAN_DIRECTION_NORMAL = 0xC0;
    const uint8_t CMD_COM_SCAN_DIRECTION_INVERT = 0xC8;

    enum class Rotation {
        R0,
        R90,
        R180,
        R270,
    };

    void initScreen(SPI::Peripheral spi, GPIO::Pin pinDC, GPIO::Pin pinRES);
    void enable();
    void disable();
    void setContrast(uint8_t contrast);
    void sendCommand(uint8_t command);
    void refresh();
    void setColorInverted(bool inverted=true);
    void clear();
    void clear(unsigned int x, unsigned int y, unsigned int width, unsigned int height);
    void setPixel(unsigned int x, unsigned int y, bool on=true);
    void resetPixel(unsigned int x, unsigned int y);
    void print(char character);
    void print(unsigned int x, unsigned int y, char character);
    void print(const char* text);
    void print(unsigned int x, unsigned int y, const char* text);
    void printCentered(unsigned int x, unsigned int y, const char* text);
    int textWidth(const char* characters);
    void printInt(unsigned int x, unsigned int y, int value, int base=10);
    void printInt(int value, int base=10);
    void printSmall(char character);
    void printSmall(unsigned int x, unsigned int y, char character);
    void printSmall(unsigned int x, unsigned int y, const Font::Char5 c);
    void printMedium(char character);
    void printMedium(unsigned int x, unsigned int y, char character);
    void printMedium(unsigned int x, unsigned int y, const Font::Char8 c);
    void printLarge(char character);
    void printLarge(unsigned int x, unsigned int y, char character);
    void printLarge(unsigned int x, unsigned int y, const Font::Char16 c);
    void printXLarge(unsigned int x, unsigned int y, const Font::Char32 c);
    void printXXLarge(unsigned int x, unsigned int y, const Font::Char64 c);
    void progressbar(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char percent);
    void button(unsigned int x, unsigned int y, unsigned int width, unsigned int height, const char* label, bool selected=false, bool pressed=false, bool arrowLeft=false, bool arrowRight=false);
    void checkbox(unsigned int x, unsigned int y, unsigned int width, unsigned int height, const char* label, bool selected, bool pressed, bool checked);
    void setCursor(unsigned int x, unsigned int y);
    void setCursorX(unsigned int x);
    void setCursorY(unsigned int y);
    void moveCursor(unsigned int x, unsigned int y);
    void moveCursorX(unsigned int x);
    void moveCursorY(unsigned int y);
    void setSize(Font::Size size);
    void setInverted(bool inverted);
    void setRotation(Rotation rotation);
    unsigned int getFontHeight();
    unsigned int getFontHeight(Font::Size size);
    unsigned int getFontWidth();
    unsigned int getFontWidth(Font::Size size);

}

#endif