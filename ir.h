// Oliver Kovacs - 2024 - MIT

#ifndef IR_H
#define IR_H

#include <stdbool.h>
#include <stdint.h>
#include "pico/stdlib.h"

const uint32_t IR_NEC_HEADER_PULSES = 342;     // 9 ms
const uint32_t IR_NEC_HEADER_PAUSE = 4500;     // 4.5 ms
const uint32_t IR_NEC_DATA_PULSES = 21;        // 0.5625 ms
const uint32_t IR_NEC_DATA_PAUSE_0 = 562;      // 0.5625 ms
const uint32_t IR_NEC_DATA_PAUSE_1 = 1687;     // 1.6875 ms
const uint32_t IR_NEC_FRAME_LEN = 32;

const uint32_t IR_SONY_HEADER_PULSES = 96;     // 2.4 ms
const uint32_t IR_SONY_HEADER_PAUSE = 600;     // 0.6 ms
const uint32_t IR_SONY_DATA_PULSES_0 = 24;     // 0.6 ms
const uint32_t IR_SONY_DATA_PULSES_1 = 48;     // 1.2 ms
const uint32_t IR_SONY_DATA_PAUSE = 600;       // 0.6 ms
const uint32_t IR_SONY_FRAME_LEN = 12;

const uint32_t IR_TX_PAUSE_38KHZ = 13;
const uint32_t IR_TX_PAUSE_40KHZ = 12;         // 13 should also work

const uint32_t IR_NEC_COMMANDS[32] = {
    0b11111100000000111110111100000000,
    0b11111101000000101110111100000000,
    0b11111110000000011110111100000000,
    0b11111111000000001110111100000000,
    0b11111000000001111110111100000000,
    0b11111001000001101110111100000000,
    0b11111010000001011110111100000000,
    0b11111011000001001110111100000000,
    0b11110100000010111110111100000000,
    0b11110101000010101110111100000000,
    0b11110110000010011110111100000000,
    0b11110111000010001110111100000000,
    0b11110000000011111110111100000000,
    0b11110001000011101110111100000000,
    0b11110010000011011110111100000000,
    0b11110011000011001110111100000000,
    0b11101100000100111110111100000000,
    0b11101101000100101110111100000000,
    0b11101110000100011110111100000000,
    0b11101111000100001110111100000000,
    0b11101000000101111110111100000000,
    0b11101001000101101110111100000000,
    0b11101010000101011110111100000000,
    0b11101011000101001110111100000000,
};

const uint32_t IR_SONY_COMMANDS[1] = {
    0b100000010101,
};

static inline void ir_tx_pulses(uint32_t pin, const size_t count, const uint32_t pause) {
    for (size_t i = 0; i < count; i++) {
        gpio_put(pin, 1);
        busy_wait_us(pause);
        gpio_put(pin, 0);
        busy_wait_us(pause);
    }
}

static inline void ir_tx_nec_header(uint32_t pin) {
    ir_tx_pulses(pin, IR_NEC_HEADER_PULSES, IR_TX_PAUSE_38KHZ);
    busy_wait_us(IR_NEC_HEADER_PAUSE);
}

static inline void ir_tx_sony_header(uint32_t pin) {
    ir_tx_pulses(pin, IR_SONY_HEADER_PULSES, IR_TX_PAUSE_40KHZ);
    busy_wait_us(IR_SONY_HEADER_PAUSE);
}

static inline void ir_tx_nec_bit(uint32_t pin, bool bit) {
    ir_tx_pulses(pin, IR_NEC_DATA_PULSES, IR_TX_PAUSE_38KHZ);
    busy_wait_us(bit ? IR_NEC_DATA_PAUSE_1 : IR_NEC_DATA_PAUSE_0);
}

static inline void ir_tx_sony_bit(uint32_t pin, bool bit) {
    ir_tx_pulses(pin, bit ? IR_SONY_DATA_PULSES_1 : IR_SONY_DATA_PULSES_0, IR_TX_PAUSE_40KHZ);
    busy_wait_us(IR_SONY_DATA_PAUSE);
}

static inline void ir_tx_nec_close(uint32_t pin) {
    ir_tx_pulses(pin, IR_NEC_DATA_PULSES, IR_TX_PAUSE_38KHZ);
}

static inline void ir_tx_nec(uint32_t pin, uint32_t data) {
    ir_tx_nec_header(pin);
    for (size_t i = 0; i < IR_NEC_FRAME_LEN; i++) {
        ir_tx_nec_bit(pin, data & 1);
        data >>= 1;
    }
    ir_tx_nec_close(pin);
}

static inline void ir_tx_sony(uint32_t pin, uint32_t data) {
    ir_tx_sony_header(pin);
    for (size_t i = 0; i < IR_SONY_FRAME_LEN; i++) {
        ir_tx_sony_bit(pin, data & 1);
        data >>= 1;
    }
}

static inline void ir_tx_init(uint32_t pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
}

// ir_tx_benchmark(pin, 380000, IR_TX_PAUSE_38KHZ);
// ir_tx_benchmark(pin, 400000, IR_TX_PAUSE_40KHZ);
static inline void ir_tx_benchmark(uint32_t pin, const uint32_t pulses, size_t pause) {
    uint64_t begin = time_us_64();
    ir_tx_pulses(pin, pulses, pause);
    uint64_t end = time_us_64();
    uint64_t delta = end - begin;
    printf("benchmark: %llu us\n", delta);
}

#endif /* IR_H */
