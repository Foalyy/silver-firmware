#ifndef _FONT_H_
#define _FONT_H_

#include <stdint.h>

namespace Font {
    struct Char5 {
        const unsigned int width;
        const unsigned int height;
        const uint8_t bitmap[5];
    };
    struct Char8 {
        const unsigned int width;
        const unsigned int height;
        const uint8_t bitmap[8];
    };
    struct Char16 {
        const unsigned int width;
        const unsigned int height;
        const uint16_t bitmap[16];
    };
    struct Char32 {
        const unsigned int width;
        const unsigned int height;
        const uint32_t bitmap[32];
    };
    struct Char64 {
        const unsigned int width;
        const unsigned int height;
        const uint64_t bitmap[64];
    };

    enum class Size {
        SMALL=0,
        MEDIUM=1,
        LARGE=2,

        N_SIZES
    };

    //const unsigned int HEIGHT_SMALL = 5;
    //const unsigned int HEIGHT_MEDIUM = 8;
    //const unsigned int HEIGHT_LARGE = 16;
    //const unsigned int HEIGHT_XLARGE = 32;
    const unsigned int HEIGHT[] = {5, 8, 16, 32};
    const unsigned int WIDTH[] = {5, 5, 10, 14};

    const unsigned int N_CHARS_TOTAL = 96;
    const unsigned int N_CHARS_SMALL = 96;
    const unsigned int N_CHARS_MEDIUM = 96;
    const unsigned int N_CHARS_LARGE = 68;
    extern const Char5 fontSmall[N_CHARS_SMALL];
    extern const Char8 fontMedium[N_CHARS_MEDIUM];
    extern const Char16 fontLarge[N_CHARS_LARGE];

    Char5 getSmall(char c);
    Char8 getMedium(char c);
    Char16 getLarge(char c);

}

#endif