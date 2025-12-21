#include <stdio.h>
#include <string.h>
#include "pico/cyw43_arch.h"
#include "lwip/apps/mqtt.h"
#include "lwip/dns.h"
#include "mqtt_client.h"

// ===== CONFIG =====
#define MQTT_BROKER_HOST "test.mosquitto.org"
#define MQTT_PORT       8883
#define MQTT_CLIENT_ID  "pico_w_tls_client"
#define MQTT_TOPIC      "pico/sensor"
// ==================

static mqtt_client_t *client;
static ip_addr_t broker_ip;
static bool dns_done;
static bool mqtt_connected;

// ---- CA CERT ----
static const char mqtt_ca_cert[] =
"-----BEGIN CERTIFICATE-----\n"
"MIIFazCCA1OgAwIBAgISA7...\n"
"...\n"
"-----END CERTIFICATE-----\n";

// ---------- Callbacks ----------

static void mqtt_connection_cb(
    mqtt_client_t *c,
    void *arg,
    mqtt_connection_status_t status
) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("[MQTT] TLS connected\n");
        mqtt_connected = true;
    } else {
        printf("[MQTT] TLS connect failed: %d\n", status);
        mqtt_connected = false;
    }
}

static void dns_cb(const char *name, const ip_addr_t *ipaddr, void *arg) {
    if (ipaddr) {
        broker_ip = *ipaddr;
        dns_done = true;
        printf("[DNS] Broker resolved\n");
    }
}

// ---------- Public API ----------

void mqtt_init(void) {
    client = mqtt_client_new();
    dns_done = false;
    mqtt_connected = false;
}

void mqtt_poll(void) {
    if (!cyw43_arch_wifi_is_connected()) {
        mqtt_connected = false;
        dns_done = false;
        return;
    }

    // DNS
    if (!dns_done) {
        err_t err = dns_gethostbyname(
            MQTT_BROKER_HOST,
            &broker_ip,
            dns_cb,
            NULL
        );
        if (err == ERR_OK) dns_done = true;
        return;
    }

    // Connect with TLS
    if (!mqtt_connected && !mqtt_client_is_connected(client)) {

        static struct mqtt_connect_client_info_t ci = {0};
        ci.client_id = MQTT_CLIENT_ID;
        ci.keep_alive = 60;

        // ---- TLS CONFIG ----
        static struct altcp_tls_config *tls_config = NULL;
        if (!tls_config) {
            tls_config = altcp_tls_create_config_client(
                (const uint8_t *)mqtt_ca_cert,
                strlen(mqtt_ca_cert) + 1
            );
        }

        mqtt_client_connect(
            client,
            &broker_ip,
            MQTT_PORT,
            mqtt_connection_cb,
            NULL,
            &ci,
            tls_config
        );
    }
}

bool mqtt_publish_json(const char *json) {
    if (!mqtt_connected) return false;

    err_t err = mqtt_publish(
        client,
        MQTT_TOPIC,
        json,
        strlen(json),
        0,
        0,
        NULL,
        NULL
    );

    return err == ERR_OK;
}
