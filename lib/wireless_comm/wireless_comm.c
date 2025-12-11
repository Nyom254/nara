#include <stdio.h>
#include "pico/cyw43_arch.h"

void wifi_init(){ 

  cyw43_arch_enable_sta_mode();
  
  while(cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 1000)){
    printf("failed to connect to wifi\n");
  }
  printf("Connected\n");
}
