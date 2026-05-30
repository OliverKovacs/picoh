// Oliver Kovacs - 2024 - MIT

#ifndef DHT11_H
#define DHT11_H

#include <stdbool.h>
#include <stdint.h>
#include "pico/stdlib.h"

enum dht_result {
    DHT_SUCCESS = 0,
    DHT_ETIMEOUT = 1,
    DHT_ECHECKSUM = 2,
    DHT_EMAXRETRY = 4,
};

typedef struct DhtData {
    uint8_t bytes[5];       // [ int_rh dec_rh int_t dec_t checksum ]
} DhtData;

const uint32_t DHT_PRE_WAKEUP = 10000;      // 10 ms        TODO try to reduce
const uint32_t DHT_WAKEUP = 18000;          // 18 ms
const uint32_t DHT_POST_WAKEUP = 30;        // 20 - 40 us   TODO try to reduce
const uint32_t DHT_RETRY = 5000;            // ~ 40 + 80 + 40 * (50 + 70) = 4920 us
const uint32_t DHT_TIMEOUT_COUNT = 10000;
const uint32_t DHT_ONE_COUNT = 500;         // ~ 70 us
const uint32_t DHT_TRIES = 5;

static int32_t dht_count(uint32_t pin, bool state)
{
    uint32_t count = 0;
    while (gpio_get(pin) == state) {
        if (count++ >= DHT_TIMEOUT_COUNT)
            return -DHT_ETIMEOUT;
    }
    return count;
}

static int32_t dht_pop(uint32_t pin)
{
    const int32_t count = dht_count(pin, false);
    return count < 0
        ? count
        : dht_count(pin, true);
}

static int32_t dht_read_byte(uint32_t pin, uint8_t *byte)
{
    *byte = 0;
    for (uint8_t mask = 0x80; mask; mask >>= 1) {
        const int32_t count = dht_pop(pin);
        if (count < 0)
            return count;

        *byte |= (uint32_t)count >= DHT_ONE_COUNT ? mask : 0;
    }
    return DHT_SUCCESS;
}

static bool dht_is_checksum_valid(DhtData *data)
{
    uint8_t checksum = 0;
    for (size_t i = 0; i < 4; i++)
        checksum += data->bytes[i];

    return checksum == data->bytes[4];
}

int32_t dht_read_once(uint32_t pin, DhtData *data)
{
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, 1);
    busy_wait_us(DHT_PRE_WAKEUP);

    // send start signal
    gpio_put(pin, 0);
    busy_wait_us(DHT_WAKEUP);

    gpio_put(pin, 1);
    gpio_set_dir(pin, GPIO_IN);
    busy_wait_us(DHT_POST_WAKEUP);

    // pop response signal
    const int32_t count = dht_pop(pin);
    if (count < 0)
        return count;

    // start of data transmission
    for (size_t i = 0; i < 5; i++) {
        const int32_t err = dht_read_byte(pin, data->bytes + i);
        if (err)
            return err;
    }

    if (!dht_is_checksum_valid(data))
        return -DHT_ECHECKSUM;

    return DHT_SUCCESS;
}

int32_t dht_read(uint32_t pin, DhtData *data)
{
    for (size_t i = 0; i < DHT_TRIES; i++) {
        if (!dht_read_once(pin, data))
            return DHT_SUCCESS;

        busy_wait_us(DHT_RETRY);
    }
    return -DHT_EMAXRETRY;
}

float dht_temperature(DhtData *data)
{
    return (float)data->bytes[2] + (float)data->bytes[3] / 10;
}

float dht_humidity(DhtData *data)
{
    return (float)data->bytes[0] + (float)data->bytes[1] / 10;
}

#endif /* DHT11_H */
