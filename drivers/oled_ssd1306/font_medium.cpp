#include "font.h"

namespace Font {

    extern const uint8_t mapMedium[N_CHARS_TOTAL];

    Char8 getMedium(char c) {
        unsigned int i = (int)c - 32;
        if (i >= 0 && i <= N_CHARS_TOTAL) {
            return fontMedium[mapMedium[i]];
        } else {
            return fontMedium[0];
        }
    }

    const uint8_t mapMedium[N_CHARS_TOTAL] = {
        1, // (space)
        64, // !
        65, // "
        66, // #
        67, // $
        68, // %
        69, // &
        70, // '
        71, // (
        72, // )
        73, // *
        74, // +
        75, // ,
        76, // -
        77, // .
        78, // /
        2, // 0
        3, // 1
        4, // 2
        5, // 3
        6, // 4
        7, // 5
        8, // 6
        9, // 7
        10, // 8
        11, // 9
        79, // :
        80, // ;
        81, // <
        82, // =
        83, // >
        84, // ?
        85, // @
        12, // A
        13, // B
        14, // C
        15, // D
        16, // E
        17, // F
        18, // G
        19, // H
        20, // I
        21, // J
        22, // K
        23, // L
        24, // M
        25, // N
        26, // O
        27, // P
        28, // Q
        29, // R
        30, // S
        31, // T
        32, // U
        33, // V
        34, // W
        35, // X
        36, // Y
        37, // Z
        86, // [
        87, // backslash
        88, // ]
        89, // ^
        90, // _
        91, // `
        38, // a
        39, // b
        40, // c
        41, // d
        42, // e
        43, // f
        44, // g
        45, // h
        46, // i
        47, // j
        48, // k
        49, // l
        50, // m
        51, // n
        52, // o
        53, // p
        54, // q
        55, // r
        56, // s
        57, // t
        58, // u
        59, // v
        60, // w
        61, // x
        62, // y
        63, // z
        92, // {
        93, // |
        94, // }
        95 // ~
    };

    const Char8 fontMedium[N_CHARS_MEDIUM] = {
        { // unknown character
            4,
            8,
            {
                0b11111111,
                0b11111111,
                0b11111111,
                0b11111111,
            }
        },
        { // space
            3,
            8,
            {
                0b00000000,
                0b00000000,
                0b00000000,
            }
        },
        { // '0'
            4,
            8,
            {
                0b01111110,
                0b10000001,
                0b10000001,
                0b01111110,
            }
        },
        { // '1'
            2,
            8,
            {
                0b00000010,
                0b11111111,
            }
        },
        { // '2'
            4,
            8,
            {
                0b11100001,
                0b10010001,
                0b10001001,
                0b10000110,
            }
        },
        { // '3'
            4,
            8,
            {
                0b01000010,
                0b10001001,
                0b10001001,
                0b01110110,
            }
        },
        { // '4'
            4,
            8,
            {
                0b00111111,
                0b00100000,
                0b11111000,
                0b00100000,
            }
        },
        { // '5'
            4,
            8,
            {
                0b01011111,
                0b10001001,
                0b10001001,
                0b01110001,
            }
        },
        { // '6'
            4,
            8,
            {
                0b01111110,
                0b10010001,
                0b10010001,
                0b01100010,
            }
        },
        { // '7'
            4,
            8,
            {
                0b11000001,
                0b00110001,
                0b00001101,
                0b00000011,
            }
        },
        { // '8'
            4,
            8,
            {
                0b01110110,
                0b10001001,
                0b10001001,
                0b01110110,
            }
        },
        { // '9'
            4,
            8,
            {
                0b10001110,
                0b10010001,
                0b10010001,
                0b01111110,
            }
        },
        { // 'A'
            4,
            8,
            {
                0b11111110,
                0b00010001,
                0b00010001,
                0b11111110,
            }
        },
        { // 'B'
            4,
            8,
            {
                0b11111111,
                0b10001001,
                0b10001001,
                0b01110110,
            }
        },
        { // 'C'
            4,
            8,
            {
                0b01111110,
                0b10000001,
                0b10000001,
                0b01000010,
            }
        },
        { // 'D'
            4,
            8,
            {
                0b11111111,
                0b10000001,
                0b10000001,
                0b01111110,
            }
        },
        { // 'E'
            4,
            8,
            {
                0b11111111,
                0b10001001,
                0b10001001,
                0b10000001,
            }
        },
        { // 'F'
            4,
            8,
            {
                0b11111111,
                0b00001001,
                0b00001001,
                0b00000001,
            }
        },
        { // 'G'
            4,
            8,
            {
                0b01111110,
                0b10000001,
                0b10010001,
                0b01110001,
            }
        },
        { // 'H'
            4,
            8,
            {
                0b11111111,
                0b00001000,
                0b00001000,
                0b11111111,
            }
        },
        { // 'I'
            3,
            8,
            {
                0b10000001,
                0b11111111,
                0b10000001,
            }
        },
        { // 'J'
            4,
            8,
            {
                0b10000000,
                0b10000001,
                0b01111111,
                0b00000001,
            }
        },
        { // 'K'
            4,
            8,
            {
                0b11111111,
                0b00001000,
                0b00110110,
                0b11000001,
            }
        },
        { // 'L'
            4,
            8,
            {
                0b11111111,
                0b10000000,
                0b10000000,
                0b10000000,
            }
        },
        { // 'M'
            5,
            8,
            {
                0b11111111,
                0b00000010,
                0b00000100,
                0b00000010,
                0b11111111,
            }
        },
        { // 'N'
            5,
            8,
            {
                0b11111111,
                0b00000110,
                0b00011000,
                0b01100000,
                0b11111111,
            }
        },
        { // 'O'
            4,
            8,
            {
                0b01111110,
                0b10000001,
                0b10000001,
                0b01111110,
            }
        },
        { // 'P'
            4,
            8,
            {
                0b11111111,
                0b00010001,
                0b00010001,
                0b00001110,
            }
        },
        { // 'Q'
            4,
            8,
            {
                0b01111110,
                0b10000001,
                0b01000001,
                0b10111110,
            }
        },
        { // 'R'
            4,
            8,
            {
                0b11111111,
                0b00010001,
                0b00010001,
                0b11101110,
            }
        },
        { // 'S'
            4,
            8,
            {
                0b10000110,
                0b10001001,
                0b10010001,
                0b01100001,
            }
        },
        { // 'T'
            5,
            8,
            {
                0b00000001,
                0b00000001,
                0b11111111,
                0b00000001,
                0b00000001,
            }
        },
        { // 'U'
            4,
            8,
            {
                0b01111111,
                0b10000000,
                0b10000000,
                0b01111111,
            }
        },
        { // 'V'
            5,
            8,
            {
                0b00011111,
                0b01100000,
                0b10000000,
                0b01110000,
                0b00001111,
            }
        },
        { // 'W'
            5,
            8,
            {
                0b01111111,
                0b10000000,
                0b01100000,
                0b10000000,
                0b01111111,
            }
        },
        { // 'X'
            5,
            8,
            {
                0b11000011,
                0b00110100,
                0b00001000,
                0b00110100,
                0b11000011,
            }
        },
        { // 'Y'
            5,
            8,
            {
                0b00000111,
                0b00011000,
                0b11100000,
                0b00011000,
                0b00000111,
            }
        },
        { // 'Z'
            4,
            8,
            {
                0b11000001,
                0b10110001,
                0b10001101,
                0b10000011,
            }
        },
        { // 'a'
            4,
            8,
            {
                0b01000000,
                0b10101000,
                0b10101000,
                0b11110000,
            }
        },
        { // 'b'
            4,
            8,
            {
                0b11111111,
                0b10001000,
                0b10001000,
                0b01110000,
            }
        },
        { // 'c'
            4,
            8,
            {
                0b01110000,
                0b10001000,
                0b10001000,
                0b10001000,
            }
        },
        { // 'd'
            4,
            8,
            {
                0b01110000,
                0b10001000,
                0b10001000,
                0b11111111,
            }
        },
        { // 'e'
            4,
            8,
            {
                0b01110000,
                0b10101000,
                0b10101000,
                0b10110000,
            }
        },
        { // 'f'
            3,
            8,
            {
                0b11111110,
                0b00001001,
                0b00000001,
            }
        },
        { // 'g'
            4,
            8,
            {
                0b00011000,
                0b10100100,
                0b10100100,
                0b11111000,
            }
        },
        { // 'h'
            4,
            8,
            {
                0b11111111,
                0b00001000,
                0b00001000,
                0b11110000,
            }
        },
        { // 'i'
            1,
            8,
            {
                0b11111010,
            }
        },
        { // 'j'
            3,
            8,
            {
                0b10000000,
                0b10000000,
                0b01111010,
            }
        },
        { // 'k'
            4,
            8,
            {
                0b11111111,
                0b00100000,
                0b11011000,
                0b00000000,
            }
        },
        { // 'l'
            1,
            8,
            {
                0b11111111,
            }
        },
        { // 'm'
            5,
            8,
            {
                0b11111000,
                0b00001000,
                0b11110000,
                0b00001000,
                0b11110000,
            }
        },
        { // 'n'
            4,
            8,
            {
                0b11111000,
                0b00001000,
                0b00001000,
                0b11110000,
            }
        },
        { // 'o'
            4,
            8,
            {
                0b01110000,
                0b10001000,
                0b10001000,
                0b01110000,
            }
        },
        { // 'p'
            4,
            8,
            {
                0b11111000,
                0b00101000,
                0b00101000,
                0b00010000,
            }
        },
        { // 'q'
            4,
            8,
            {
                0b00010000,
                0b00101000,
                0b00101000,
                0b11111000,
            }
        },
        { // 'r'
            3,
            8,
            {
                0b11110000,
                0b00001000,
                0b00001000,
            }
        },
        { // 's'
            4,
            8,
            {
                0b10010000,
                0b10101000,
                0b10101000,
                0b01001000,
            }
        },
        { // 't'
            3,
            8,
            {
                0b00000100,
                0b01111110,
                0b10000100,
            }
        },
        { // 'u'
            4,
            8,
            {
                0b01111000,
                0b10000000,
                0b10000000,
                0b11111000,
            }
        },
        { // 'v'
            5,
            8,
            {
                0b00011000,
                0b01100000,
                0b10000000,
                0b01100000,
                0b00011000,
            }
        },
        { // 'w'
            5,
            8,
            {
                0b01111000,
                0b10000000,
                0b01110000,
                0b10000000,
                0b01111000,
            }
        },
        { // 'x'
            5,
            8,
            {
                0b10001000,
                0b01010000,
                0b00100000,
                0b01010000,
                0b10001000,
            }
        },
        { // 'y'
            4,
            8,
            {
                0b00011000,
                0b10100000,
                0b10100000,
                0b01111000,
            }
        },
        { // 'z'
            4,
            8,
            {
                0b11001000,
                0b10101000,
                0b10011000,
                0b10001000,
            }
        },
        { // '%'
            1,
            8,
            {
                0b11011111,
            }
        },
        { // '"'
            3,
            8,
            {
                0b00000111,
                0b00000000,
                0b00000111,
            }
        },
        { // '#'
            5,
            8,
            {
                0b00100100,
                0b11111111,
                0b00100100,
                0b11111111,
                0b00100100,
            }
        },
        { // '$'
            5,
            8,
            {
                0b00001100,
                0b10010010,
                0b11111111,
                0b10010010,
                0b01100000,
            }
        },
        { // '%'
            4,
            8,
            {
                0b11000011,
                0b00110000,
                0b00001100,
                0b11000011,
            }
        },
        { // '&'
            6,
            8,
            {
                0b01110110,
                0b10001001,
                0b10001001,
                0b01010001,
                0b00100000,
                0b11011000,
            }
        },
        { // '
            1,
            8,
            {
                0b00000111,
            }
        },
        { // (
            3,
            8,
            {
                0b00011000,
                0b01100110,
                0b10000001,
            }
        },
        { // )
            3,
            8,
            {
                0b10000001,
                0b01100110,
                0b00011000,
            }
        },
        { // *
            7,
            8,
            {
                0b00010001,
                0b00001010,
                0b00011111,
                0b00001010,
                0b00010001,
            }
        },
        { // +
            5,
            8,
            {
                0b00100000,
                0b00100000,
                0b11111000,
                0b00100000,
                0b00100000,
            }
        },
        { // ,
            3,
            8,
            {
                0b10000000,
                0b01000000,
                0b00100000,
            }
        },
        { // -
            4,
            8,
            {
                0b00100000,
                0b00100000,
                0b00100000,
                0b00100000,
            }
        },
        { // '.'
            2,
            8,
            {
                0b11000000,
                0b11000000,
            }
        },
        { // '/'
            4,
            8,
            {
                0b11000000,
                0b00110000,
                0b00001100,
                0b00000011,
            }
        },
        { // ':'
            1,
            8,
            {
                0b11001100,
            }
        },
        { // ';'
            2,
            8,
            {
                0b10000000,
                0b01101100,
            }
        },
        { // '<'
            4,
            8,
            {
                0b00010000,
                0b00101000,
                0b01000100,
                0b10000010,
            }
        },
        { // '='
            4,
            8,
            {
                0b01001000,
                0b01001000,
                0b01001000,
                0b01001000,
            }
        },
        { // '>'
            4,
            8,
            {
                0b10000010,
                0b01000100,
                0b00101000,
                0b00010000,
            }
        },
        { // '?'
            5,
            8,
            {
                0b00000010,
                0b00000001,
                0b10110001,
                0b00001001,
                0b00000110,
            }
        },
        { // '@'
            8,
            8,
            {
                0b00111100,
                0b01000010,
                0b10011001,
                0b10100101,
                0b10100101,
                0b10111101,
                0b10100001,
                0b00011110,
            }
        },
        { // '['
            3,
            8,
            {
                0b11111111,
                0b10000001,
                0b10000001,
            }
        },
        { // '\'
            4,
            8,
            {
                0b00000011,
                0b00001100,
                0b00110000,
                0b11000000,
            }
        },
        { // ']'
            3,
            8,
            {
                0b10000001,
                0b10000001,
                0b11111111,
            }
        },
        { // '^'
            5,
            8,
            {
                0b00000100,
                0b00000010,
                0b00000001,
                0b00000010,
                0b00000100,
            }
        },
        { // '_'
            5,
            8,
            {
                0b10000000,
                0b10000000,
                0b10000000,
                0b10000000,
                0b10000000,
            }
        },
        { // '`'
            3,
            8,
            {
                0b00000001,
                0b00000010,
                0b00000100,
            }
        },
        { // '{'
            3,
            8,
            {
                0b00001000,
                0b01111110,
                0b10000001,
            }
        },
        { // '|'
            1,
            8,
            {
                0b11111111,
            }
        },
        { // '}'
            3,
            8,
            {
                0b10000001,
                0b01111110,
                0b00001000,
            }
        },
        { // '~'
            6,
            8,
            {
                0b00100000,
                0b00010000,
                0b00010000,
                0b00100000,
                0b00100000,
                0b00010000,
            }
        }
    };
}
