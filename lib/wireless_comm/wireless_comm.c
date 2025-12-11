#include <stdio.h>
#include "pico/cyw43_arch.h"

void wifi_init(char* ssid, char* password){ 
  while(cyw43_arch_init()){
    printf("failed to initialized\n");
  }
  printf("initialized\n");

  cyw43_arch_enable_sta_mode();
  
  while(cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, 10000)){
    printf("failed to connect to wifi\n");
  }
  printf("Connected\n");
}
