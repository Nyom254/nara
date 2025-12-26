#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lib/lvgl/lvgl.h"

#include "lib/wireless_comm/wireless_comm.h"
#include "lib/uart/sensor_comm/sensor_comm.h"
#include "lib/spi/display_comm/display_comm.h"

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

bool read_sensor_timer_cb(struct repeating_timer *t) {
    read_sensor_state = !read_sensor_state;
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
  sensor_data_t sensor_data;
  
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
  static struct repeating_timer read_sensor_timer;
  add_repeating_timer_ms(
    1000, 
    read_sensor_timer_cb, 
    NULL, 
    &read_sensor_timer
  );

  ui_init();

  /* When sensor data arrives */
  sensor_data_t data_dummy = {
      .temperature = 27.3,
      .humidity = 68.2,
      .conductivity = 1.42,
      .pH = 6.7,
      .nitrogen = 120,
      .phosphorus = 60,
      .potassium = 180
  };

  ui_update_sensor(&data_dummy);
  ui_update_wifi(true);
  ui_update_battery(76);


  while (true) {

    // if (read_sensor_state) {
    //   read_sensor_state = false;
    //   if (read_sensor_data(&sensor_data)) {
    //     printf("Humidity: %.2f %%\n", sensor_data.humidity);
    //   printf("Temperature: %.2f °C\n", sensor_data.temperature);
    //   printf("Conductivity: %u µS/cm\n", sensor_data.conductivity);
    //   printf("pH: %.2f\n", sensor_data.pH);
    //   printf("Nitrogen: %.2f\n", sensor_data.nitrogen);
    //   printf("Phosphorus: %.2f\n", sensor_data.phosphorus);
    //   printf("Potassium: %.2f\n", sensor_data.potassium);
    //   json_payload_t json_payload = build_sensor_data_json(&sensor_data);
    //   if(mqtt_publish_json(json_payload.json)){
    //     printf("MQTT Publish Success\n");
    //   } else {
    //     printf("MQTT Publish Failed\n");
    //   };

    // } else {
    //   printf("Failed to read sensor data\n");
    // }
  // }
    lv_timer_handler();

    cyw43_poll();
    sleep_ms(5);

    tight_loop_contents();
  }

  return 0;
}
