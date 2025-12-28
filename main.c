#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lib/lvgl/lvgl.h"

#include "lib/wireless_comm/wireless_comm.h"
#include "lib/uart/sensor_comm/sensor_comm.h"
#include "lib/spi/display_comm/display_comm.h"
#include "lib/sensor_data/sensor_data.h"

#include "lib/ui/ui.h"

#define HIGH 1
#define LOW 0

static bool led_state = false;
static bool read_sensor_state = false;

bool led_timer_cb(struct repeating_timer *t) {
    led_state = !led_state;
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_state ? HIGH : LOW);
    return true;
}

int main() {
  stdio_init_all();

  if(cyw43_arch_init()){
    printf("failed to initialized\n");
    sleep_ms(500);
    return -1;
  }
  /*
    initialize mutex  shared sensor data
  */
  sensor_data_shared_init();

  /*
    Bluetooth Initialization
  */
  //  initialize bluetooth
  if (!ble_init()) {
    return -1;
  }

  /* 
  Sensor initialization
   */
  sensor_pin_init();
  // start_read_sensor_core1_task();
  /* 
    Display initialization 
  */
  ili9488_init();
  spi_dma_init();
  lv_init();
  static struct repeating_timer lvgl_timer;
  add_repeating_timer_ms(1, lvgl_tick_cb, NULL, &lvgl_timer);
  lv_port_disp_init();

  static struct repeating_timer led_timer;
  add_repeating_timer_ms(
    1000, 
    led_timer_cb, 
    NULL, 
    &led_timer
  );
  ui_init();
  sensor_data_t dat_dummy = {
      .humidity = 50.0f,
      .temperature = 25.0f,
      .conductivity = 500.0f,
      .pH = 6.5f,
      .nitrogen = 10.0f,
      .phosphorus = 5.0f,
      .potassium = 8.0f
  };
  sensor_data_t ui_data;
  uint32_t last_data_seq = 0;
  ui_update_wifi(true);
  ui_update_battery(76);
  sensor_data_set(&dat_dummy);

  while (true) {
    if (sensor_data_get(&ui_data, &last_data_seq)) {
        ui_update_sensor(&ui_data);
    }

    lv_timer_handler();
    cyw43_poll();
    sleep_ms(5);
    tight_loop_contents();
  }

  return 0;
}
