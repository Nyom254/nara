#include <stdio.h>
#include "pico/cyw43_arch.h"
#include "pico/async_context.h"
#include "lwip/altcp_tls.h"
#include "http_client_util.h"
#include "wireless_comm.h"
#include "../uart/sensor_comm/sensor_comm.h"


#define HOST "cba198f2c471a123f9f4548305bb1075.serveousercontent.com"
#define URL_REQUEST "/"

// Pico W Wifi Initialiation / Connection
bool wifi_init(){ 
  if(cyw43_arch_init()){
    printf("failed to initialized\n");
    sleep_ms(500);
    return false;
  }
  sleep_ms(1000);
  printf("initialized\n");

  cyw43_arch_enable_sta_mode();
  
  return true;
}

bool connect_to_wifi(){
  printf("Connecting to WiFi...\n" WIFI_SSID);
  if(cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 5000)){
    printf("failed to connect to wifi\n");
    printf("SSID: \"" WIFI_SSID "\"\n");
    printf("Password: \"" WIFI_PASSWORD "\"\n");
    return false;
  }
  printf("Connected to " WIFI_SSID "\n");
  return true;
}



json_payload_t build_sensor_data_json(sensor_data_t *d) {
  json_payload_t payload;
  snprintf(payload.json, sizeof(payload.json),
        "{"
        "\"humidity\": %.2f,"
        "\"temperature\": %.2f,"
        "\"conductivity\": %.2f,"
        "\"ph\": %.2f,"
        "\"nitrogen\": %.2f,"
        "\"phosphorus\": %.2f,"
        "\"potassium\": %.2f"
        "}"
      , d->humidity, d->temperature, d->conductivity, d->pH, d->nitrogen, d->phosphorus, d->potassium);
    return payload;
}
