#ifndef IR_DATA_H
#define IR_DATA_H

#include <stdint.h>

const uint32_t IR_NEC_REMOTE1[32] = {
    0b11111111000000001110111100000000,
    0b11111110000000011110111100000000,
    0b11111101000000101110111100000000,
    0b11111100000000111110111100000000,
    0b11111011000001001110111100000000,
    0b11111010000001011110111100000000,
    0b11111001000001101110111100000000,
    0b11111000000001111110111100000000,
    0b11110111000010001110111100000000,
    0b11110110000010011110111100000000,
    0b11110101000010101110111100000000,
    0b11110100000010111110111100000000,
    0b11110011000011001110111100000000,
    0b11110010000011011110111100000000,
    0b11110001000011101110111100000000,
    0b11110000000011111110111100000000,
    0b11101111000100001110111100000000,
    0b11101110000100011110111100000000,
    0b11101101000100101110111100000000,
    0b11101100000100111110111100000000,
    0b11101011000101001110111100000000,
    0b11101010000101011110111100000000,
    0b11101001000101101110111100000000,
    0b11101000000101111110111100000000,
};

const uint32_t IR_NEC_REMOTE2[21] = {
    0b10110010010011011111111100000000,
    0b10101011010101001111111100000000,
    0b11101001000101101111111100000000,
    0b10110011010011001111111100000000,
    0b11111010000001011111111100000000,
    0b11110011000011001111111100000000,
    0b11110101000010101111111100000000,
    0b10111111010000001111111100000000,
    0b11100001000111101111111100000000,
    0b11101101000100101111111100000000,
    0b11111101000000101111111100000000,
    0b11100011000111001111111100000000,
    0b11110110000010011111111100000000,
    0b11100010000111011111111100000000,
    0b11100000000111111111111100000000,
    0b11110010000011011111111100000000,
    0b11100110000110011111111100000000,
    0b11100100000110111111111100000000,
    0b11101110000100011111111100000000,
    0b11101010000101011111111100000000,
    0b11101000000101111111111100000000,
};

const uint32_t IR_SIRC_REMOTE1[1] = {
    0b100000010101,
};

#endif /* IR_DATA_H */
