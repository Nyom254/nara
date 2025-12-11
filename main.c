#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lib/wireless_comm/wireless_comm.h"

#define HIGH 1
#define LOW 0



int main() {
  stdio_init_all();
  if(cyw43_arch_init()){
    printf("failed to initialized\n");
  }
  printf("initialized\n");
  
  wifi_init();

  while (true) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, HIGH);
    sleep_ms(500);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, LOW);
    sleep_ms(500);
  }

  return 0;
}
