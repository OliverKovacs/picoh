// Oliver Kovacs - 2024 - MIT

#ifndef MAX7219_H
#define MAX7219_H

#include <stdbool.h>
#include <stdint.h>

/*
VCC - 3V3
GND - GND
DIN - GP mosi_pin 
CS  - GP cs_pin
CLK - GP clk_pin
*/

typedef struct MaxSpiInterface {
    uint32_t mosi_pin;
    uint32_t clk_pin;
    uint32_t cs_pin;
} MaxSpiInterface;

const uint8_t MAX_REG_NOOP = 0x00;
const uint8_t MAX_REG_DECODE_MODE = 0x09;
const uint8_t MAX_REG_INTENSITY = 0x0A;
const uint8_t MAX_REG_SCAN_LIMIT = 0x0B;
const uint8_t MAX_REG_SHUTDOWN = 0x0C;
const uint8_t MAX_REG_DISPLAY_TEST = 0x0F;

const uint32_t MAX_TIMEOUT_US = 1;

inline static void max_write_bit(MaxSpiInterface *spi, bool bit) {
    gpio_put(spi->mosi_pin, bit);
    busy_wait_us(MAX_TIMEOUT_US);
    gpio_put(spi->clk_pin, 1);
    busy_wait_us(MAX_TIMEOUT_US);
    gpio_put(spi->clk_pin, 0);
    busy_wait_us(MAX_TIMEOUT_US);
}

void max_write_data(MaxSpiInterface *spi, uint16_t data) {
    gpio_put(spi->cs_pin, 0);
    busy_wait_us(MAX_TIMEOUT_US);
    uint16_t mask = 0x8000;
    for (size_t i = 0; i < 16; i++) {
        max_write_bit(spi, (data & mask) ? true : false);
        mask >>= 1;
    }
    gpio_put(spi->cs_pin, 1);
    busy_wait_us(MAX_TIMEOUT_US);
}

void max_write_reg(MaxSpiInterface *spi, uint8_t reg, uint8_t value) {
    max_write_data(spi, (reg << 8) | value);
}

void max_clear(MaxSpiInterface *spi) {
    for (uint8_t i = 0; i < 8; i++) {
        max_write_reg(spi, i + 1, 0x00);
    }
}

void max_render_bitmap(MaxSpiInterface *spi, uint8_t bitmap[8]) {
    for (uint8_t i = 0; i < 8; i++) {
        max_write_reg(spi, i + 1, bitmap[i]);
    }
}

void max_init(MaxSpiInterface *spi, uint8_t intensity) {
    gpio_init(spi->mosi_pin);
    gpio_set_dir(spi->mosi_pin, GPIO_OUT);
    gpio_init(spi->clk_pin);
    gpio_set_dir(spi->clk_pin, GPIO_OUT);
    gpio_init(spi->cs_pin);
    gpio_set_dir(spi->cs_pin, GPIO_OUT);
    gpio_put(spi->cs_pin, 1);

    sleep_ms(10);

    max_write_reg(spi, MAX_REG_DISPLAY_TEST, 0x00);
    max_write_reg(spi, MAX_REG_SCAN_LIMIT, 0x07);
    max_write_reg(spi, MAX_REG_DECODE_MODE, 0x00);
    max_write_reg(spi, MAX_REG_INTENSITY, intensity);
    max_write_reg(spi, MAX_REG_SHUTDOWN, 0x01);
    max_clear(spi);
}

#endif /* MAX7219_H */
