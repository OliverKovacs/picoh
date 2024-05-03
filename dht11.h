// Oliver Kovacs - 2024 - MIT

#ifndef DHT11_H
#define DHT11_H

#include <stdbool.h>
#include <stdint.h>
#include "pico/stdlib.h"

#define DHT_CHECK_TIMEOUT(r)                        \
{                                                   \
    const int32_t result = (r);                     \
    if (result == DHT_TIMEOUT) return DHT_TIMEOUT;  \
}

typedef enum DhtResult {
    DHT_SUCCESS = 0,
    DHT_TIMEOUT = -1,
    DHT_ERROR = -2,
} DhtResult;

typedef union DhtData {
    uint8_t bytes[5];
    struct {
        uint8_t int_rh;     // relative humidity
        uint8_t dec_rh;     // relative humidity
        uint8_t int_t;      // temperature
        uint8_t dec_t;      // temperature
        uint8_t checksum;
    };
} DhtData;

const uint32_t DHT_WAKEUP_US = 18000;       // 18 ms
const uint32_t DHT_PRE_WAKEUP_US = 10000;   // 10 ms        TODO try to reduce
const uint32_t DHT_POST_WAKEUP_US = 30;     // 20 - 40 us   TODO try to reduce
const uint32_t DHT_RETRY_US = 5000;         // ~ 40 + 80 + 40 * (50 + 70) = 4920 us
const uint32_t DHT_TIMEOUT_COUNT = 10000;
const uint32_t DHT_ONE_COUNT = 500;         // ~ 70 us
const uint32_t DHT_TRIES = 5;

static inline int32_t dht_count(uint32_t pin, bool state) {
    uint32_t count = 0;
    while (gpio_get(pin) == state) {
        if (count++ >= DHT_TIMEOUT_COUNT) return DHT_TIMEOUT;
    }
    return count;
}

static inline DhtResult dht_read_byte(uint32_t pin, uint8_t *byte) {
    *byte = 0;
    for (uint8_t mask = 0x80; mask; mask >>= 1) {
        DHT_CHECK_TIMEOUT(dht_count(pin, false));
        int32_t count = dht_count(pin, true);
        DHT_CHECK_TIMEOUT(count);
        if (count >= DHT_ONE_COUNT) *byte |= mask;
    }
    return DHT_SUCCESS;
}

inline static bool dht_verify_checksum(DhtData *data) {
    uint8_t checksum = 0;
    for (size_t i = 0; i < 4; i++) {
        checksum += data->bytes[i];
    }
    return checksum == data->checksum ? DHT_SUCCESS : DHT_ERROR;
}

static inline DhtResult dht_read_try(uint32_t pin, DhtData *data) {
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, 1);
    busy_wait_us(DHT_PRE_WAKEUP_US);
    
    // send start signal
    gpio_put(pin, 0);
    busy_wait_us(DHT_WAKEUP_US);

    gpio_put(pin, 1);
    gpio_set_dir(pin, GPIO_IN);
    busy_wait_us(DHT_POST_WAKEUP_US);

    // pop response signal
    DHT_CHECK_TIMEOUT(dht_count(pin, false));
    DHT_CHECK_TIMEOUT(dht_count(pin, true));
    
    // start of data transmission
    for (size_t i = 0; i < 5; i++) {
        const DhtResult result = dht_read_byte(pin, data->bytes + i);
        if (result) {
            puts("dht: failed to read byte");
            return result;
        }
    }

    if (dht_verify_checksum(data)) {
        puts("dht: checksum doesn't match");
        return DHT_ERROR;
    } 

    return DHT_SUCCESS;
}

DhtResult dht_read(uint32_t pin, DhtData *data) {
    for (size_t i = 0; i < DHT_TRIES; i++) {
        if (!dht_read_try(pin, data)) return DHT_SUCCESS;
        busy_wait_us(DHT_RETRY_US);
    }
    return DHT_ERROR;
}

float dht_temperature(DhtData *data) {
    return (float)data->int_t + (float)data->dec_t / 10;
}

float dht_humidity(DhtData *data) {
    return (float)data->int_rh + (float)data->dec_rh / 10;
}

# endif /* DHT11_H */
