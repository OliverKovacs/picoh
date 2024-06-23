#ifndef MQTT_H
#define MQTT_H

#include <stdint.h>
#include <stdio.h>
#include "lwip/apps/mqtt.h"

#define MQTT_ERR(str, term) {   \
    int32_t tmp = (term);       \
    if (tmp != ERR_OK) {        \
        printf(str, tmp);       \
    }                           \
}

static ip_addr_t ip_addr;
static int32_t inpub_id;

static void mqtt_connection_cb(
    mqtt_client_t *client,
    void *arg,
    mqtt_connection_status_t status
);

static void mqtt_incoming_data_cb(
    void *arg,
    const uint8_t *data,
    uint16_t len,
    uint8_t flags
) {
    printf(
        "mqtt: incoming publish payload with length %d, flags %u\n",
        len,
        (uint32_t)flags
    );

    if (flags & MQTT_DATA_FLAG_LAST) {
        // last fragment of payload received
        // or whole part if payload fits receive buffer see MQTT_VAR_HEADER_BUFFER_LEN

        // switch (inpub_id) {}
        // use data
    }
    else {
        // TODO handle fragmented payload, store in buffer, write to file or whatever
    }
}

static void mqtt_incoming_publish_cb(
    void *arg,
    const char *topic,
    uint32_t topic_len
) {
    printf(
        "mqtt: incoming publish at topic %s with total length %u\n",
        topic,
        (uint32_t)topic_len
    );
    
    // decode topic string into a user defined reference
    inpub_id = 0;
}

void lwip_mqtt_connect(mqtt_client_t *client) {
    ip4addr_aton(MQTT_HOST, &ip_addr);

    struct mqtt_connect_client_info_t ci;
    memset(&ci, 0, sizeof(ci));
    ci.client_id = MQTT_ID;
    ci.client_user = MQTT_USER;
    ci.client_pass = MQTT_PASS;

    cyw43_arch_lwip_begin();
    err_t err = mqtt_client_connect(
        client, &ip_addr,
        MQTT_PORT,
        mqtt_connection_cb,
        NULL,
        &ci
    );
    cyw43_arch_lwip_end();

    MQTT_ERR("mqtt_client_connect: err: %d\n", err);
}

static void mqtt_sub_request_cb(void *arg, err_t result) {
    MQTT_ERR("mqtt: subscribe result: %d\n", result);
}

static void mqtt_pub_request_cb(void *arg, err_t result) {
    MQTT_ERR("mqtt: publish result: %d\n", result);
}

static void mqtt_connection_cb(
    mqtt_client_t *client,
    void *arg,
    mqtt_connection_status_t status
) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("mqtt_connection_cb: successfully connected\n");
        
        // setup callback for incoming publish requests
        cyw43_arch_lwip_begin();
        mqtt_set_inpub_callback(
            client,
            mqtt_incoming_publish_cb,
            mqtt_incoming_data_cb,
            arg
        );
        cyw43_arch_lwip_end();
        
        // cyw43_arch_lwip_begin();
        // err_t err = mqtt_subscribe(
        //     client,
        //     "topic",                // topic
        //     1,                      // qos
        //     mqtt_sub_request_cb,    // callback
        //     arg
        // );
        // cyw43_arch_lwip_end();
        //
        // MQTT_ERR("mqtt_subscribe: err: %d\n", err);
    } else {
        printf("mqtt: connection result: %d\n", status);

        // try to reconnect
        lwip_mqtt_connect(client);
    }
}

void lwip_mqtt_publish(
    mqtt_client_t *client,
    char *topic,
    char *payload,
    void *arg
) {
    uint8_t qos = 2;
    uint8_t retain = 0;

    cyw43_arch_lwip_begin();
    err_t err = mqtt_publish(
        client,
        topic,
        payload,
        strlen(payload),
        qos,
        retain,
        mqtt_pub_request_cb,
        arg
    );
    cyw43_arch_lwip_end();

    MQTT_ERR("mqtt_publish: err: %d\n", err);
}

# endif /* MQTT_H */
