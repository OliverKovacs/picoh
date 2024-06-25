// Oliver Kovacs - 2024 - MIT

#ifndef IR_H
#define IR_H

#include <stdbool.h>
#include <stdint.h>
#include "pico/stdlib.h"

const uint32_t NEC_TX_HDR_ON = 342;         // pulses ~ 9 ms
const uint32_t NEC_TX_HDR_OFF = 4500;       // ms     ~ 4.5 ms
const uint32_t NEC_TX_ON = 21;              // pulses ~ 0.5625 ms
const uint32_t NEC_TX_OFF0 = 562;           // ms     ~ 0.5625 ms
const uint32_t NEC_TX_OFF1 = 1687;          // ms     ~ 1.6875 ms
const uint32_t NEC_TX_FRAME_LEN = 32;

const uint32_t SIRC_TX_HDR_PULSES = 96;     // pulses ~ 2.4 ms
const uint32_t SIRC_TX_HDR_PAUSE = 600;     // ms     ~ 0.6 ms
const uint32_t SIRC_TX_ON0 = 24;            // pulses ~ 0.6 ms
const uint32_t SIRC_TX_ON1 = 48;            // ms     ~ 1.2 ms
const uint32_t SIRC_TX_OFF = 600;           // count  ~ 0.6 ms
const uint32_t SIRC_TX_FRAME_LEN = 12;

const uint32_t IR_TX_PAUSE_38KHZ = 13;      // ms
const uint32_t IR_TX_PAUSE_40KHZ = 12;      // ms, 13 should also work

#define IR_ERR(term) if (term < 0) return term;
#define IR_ERR_TMP(term) {      \
    int32_t tmp = (term);       \
    if (tmp < 0) return term;   \
}

#define NEC_RX_HDR 130000, 150000
#define NEC_RX_HDRP 55000, 75000
#define NEC_RX_HDRP_REP 25000, 40000

#define NEC_RX_OFF1 21000, 27000
#define NEC_RX_OFF0 4500, 9500

#define SIRC_RX_HDR 35000, 40000
#define SIRC_RX_HDRP 6000, 10000

#define SIRC_RX_ON1 17000, 22500    // 175000 
#define SIRC_RX_ON0 7500, 12500

typedef enum IrProto {
    NEC = 0,
    SIRC = 1,
} IrProto;

typedef struct IrData {
    IrProto proto;
    uint8_t bits;
    uint32_t repeat;
    uint32_t bytes;
} IrData;

const uint32_t IR_TIMEOUT_COUNT = 200000;
const uint32_t IR_SUCCESS = 0;
const uint32_t IR_TIMEOUT = -1;
const uint32_t IR_ERROR = -2;

void print_data(IrData *data) {
    printf("%s (%d)\n", data->proto == NEC ? "NEC" : "SIRC", data->bits);
    if (data->repeat) printf("- repeat\n");
    else printf("- %b\n", data->bytes);
}

static inline bool in_range(uint32_t value, uint32_t min, uint32_t max) {
    return min <= value && value <= max;
}

static inline void ir_tx_pulses(uint32_t pin, const size_t count, const uint32_t pause) {
    for (size_t i = 0; i < count; i++) {
        gpio_put(pin, 1);
        busy_wait_us(pause);
        gpio_put(pin, 0);
        busy_wait_us(pause);
    }
}

static inline void ir_tx_nec_header(uint32_t pin) {
    ir_tx_pulses(pin, NEC_TX_HDR_ON, IR_TX_PAUSE_38KHZ);
    busy_wait_us(NEC_TX_HDR_OFF);
}

static inline void ir_tx_sirc_header(uint32_t pin) {
    ir_tx_pulses(pin, SIRC_TX_HDR_PULSES, IR_TX_PAUSE_40KHZ);
    busy_wait_us(SIRC_TX_HDR_PAUSE);
}

static inline void ir_tx_nec_bit(uint32_t pin, bool bit) {
    ir_tx_pulses(pin, NEC_TX_ON, IR_TX_PAUSE_38KHZ);
    busy_wait_us(bit ? NEC_TX_OFF1 : NEC_TX_OFF0);
}

static inline void ir_tx_sirc_bit(uint32_t pin, bool bit) {
    ir_tx_pulses(pin, bit ? SIRC_TX_ON1 : SIRC_TX_ON0, IR_TX_PAUSE_40KHZ);
    busy_wait_us(SIRC_TX_OFF);
}

static inline void ir_tx_nec_close(uint32_t pin) {
    ir_tx_pulses(pin, NEC_TX_ON, IR_TX_PAUSE_38KHZ);
}

static inline void ir_tx_nec(uint32_t pin, uint32_t data) {
    ir_tx_nec_header(pin);
    for (size_t i = 0; i < NEC_TX_FRAME_LEN; i++) {
        ir_tx_nec_bit(pin, data & 1);
        data >>= 1;
    }
    ir_tx_nec_close(pin);
}

static inline void ir_tx_sirc(uint32_t pin, uint32_t data) {
    ir_tx_sirc_header(pin);
    for (size_t i = 0; i < SIRC_TX_FRAME_LEN; i++) {
        ir_tx_sirc_bit(pin, data & 1);
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

static inline int32_t ir_rx_wait(uint32_t pin, bool state, int32_t timeout) {
    int32_t count = 0;
    while (gpio_get(pin) != state) {
        count++;
        if (count >= timeout && timeout != -1) return IR_TIMEOUT;
    }
    return count;
}

static inline int32_t ir_rx_count(uint32_t pin, bool state) {
    return ir_rx_wait(pin, state, IR_TIMEOUT_COUNT);
}

int32_t ir_rx_nec(uint32_t pin, IrData *data) {
    data->proto = NEC;

    int32_t hdr = ir_rx_count(pin, false);
    IR_ERR(hdr);

    if (in_range(hdr, NEC_RX_HDRP_REP)) {
        IR_ERR_TMP(ir_rx_count(pin, true));
        data->repeat = true;
        data->bits = 0;
        return IR_SUCCESS;
    }
    if (!in_range(hdr, NEC_RX_HDRP)) return IR_ERROR;

    data->bytes = 0;
    for (size_t i = 0; i < 32; i++) {
        IR_ERR_TMP(ir_rx_count(pin, true));

        int32_t count = ir_rx_count(pin, false);
        IR_ERR(count);

        if (in_range(count, NEC_RX_OFF1)) data->bytes |= (1 << i);
        else if (!in_range(count, NEC_RX_OFF0)) return IR_ERROR;
    }

    IR_ERR_TMP(ir_rx_count(pin, true));
    data->repeat = false;
    data->bits = 32;
    return IR_SUCCESS;
}

int32_t ir_rx_sirc_block(uint32_t pin, uint32_t *bytes, int32_t from, int32_t to) {
    for (size_t i = from; i < to; i++) {
        int32_t count = ir_rx_count(pin, true);
        IR_ERR(count);

        if (in_range(count, SIRC_RX_ON1)) *bytes |= (1 << i);
        else if (!in_range(count, SIRC_RX_ON0)) return IR_ERROR;

        IR_ERR_TMP(ir_rx_count(pin, false));
    }

    int32_t count = ir_rx_count(pin, true);
    IR_ERR(count);

    if (in_range(count, SIRC_RX_ON1)) *bytes |= (1 << to);
    else if (!in_range(count, SIRC_RX_ON0)) return IR_ERROR;

    return IR_SUCCESS;
}

int32_t ir_rx_sirc(uint32_t pin, IrData *data) {
    data->proto = SIRC;
    data->repeat = false;
    data->bytes = 0;

    int32_t hdr = ir_rx_count(pin, false);
    IR_ERR(hdr);
    if (!in_range(hdr, SIRC_RX_HDRP)) return IR_ERROR;

    IR_ERR_TMP(ir_rx_sirc_block(pin, &data->bytes, 0, 11));
    
    if (ir_rx_wait(pin, false, 325000) == IR_TIMEOUT) {
        data->bits = 12;
        return IR_SUCCESS;
    }

    IR_ERR_TMP(ir_rx_sirc_block(pin, &data->bytes, 12, 14));
    
    if (ir_rx_wait(pin, false, 125000) == IR_TIMEOUT) {
        data->bits = 15;
        return IR_SUCCESS;
    }

    IR_ERR_TMP(ir_rx_sirc_block(pin, &data->bytes, 15, 19));
    
    data->bits = 20;
    return IR_SUCCESS;
}

int32_t ir_rx(uint32_t pin, IrData *data, int32_t timeout) {
    IR_ERR_TMP(ir_rx_wait(pin, false, timeout));
    
    int32_t count = ir_rx_count(pin, true);
    IR_ERR(count);

    if (in_range(count, NEC_RX_HDR)) return ir_rx_nec(pin, data);
    if (in_range(count, SIRC_RX_HDR)) return ir_rx_sirc(pin, data);
    return IR_ERROR;
}

#endif /* IR_H */
