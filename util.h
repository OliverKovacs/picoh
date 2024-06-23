#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include <stdio.h>
#include "pico/cyw43_arch.h"

int32_t util_try_connect() {
    return cyw43_arch_wifi_connect_timeout_ms(
        SSID,
        PASS,
        CYW43_AUTH_WPA2_AES_PSK,
        10000
    );
}

int32_t util_connect() {
    if (cyw43_arch_init_with_country(CYW43_COUNTRY_AUSTRIA)) {
        puts("failed to initialise");
        return -1;
    }
    puts("initialised");

    cyw43_arch_enable_sta_mode();
    
    int32_t err = util_try_connect();
    while (err) {
        printf("failed to connect: err %d\n", err);
        sleep_ms(1000);
        err = util_try_connect();
    }
    puts("connected");

    return 0;
}

# endif /* UTIL_H */
