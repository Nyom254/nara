#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lib/wireless_comm/wireless_comm.h"
#include "lib/uart/sensor_comm/sensor_comm.h"
#include "lib/spi/display_comm/display_comm.h"
#include "lib/lvgl/lvgl.h"


#define HIGH 1
#define LOW 0



static bool led_state = false;

bool led_timer_cb(struct repeating_timer *t) {
    led_state = !led_state;
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_state ? HIGH : LOW);
    return true;
}



int main() {
  stdio_init_all();
  

  // initialize wifi
  while(!wifi_init()) {
    sleep_ms(500);
  }
  // keep trying to connect to wifi until successful
  // while(!connect_to_wifi());
  // start lwip for http client
  // json_payload_t payload = build_sensor_json(NULL);
  // wifi_post_sensor_data(payload.json);
  // if(!wifi_send_sensor_data()) {
  //   printf("Failed to send sensor data\n");
  // } else {
  //   printf("Sensor data sent successfully\n");
  // }

  sensor_pin_init();
  sensor_data_t sensor_data;

  // ili9488_init();
  // spi_dma_init();
  // lv_init();
  // static struct repeating_timer lvgl_timer;
  // add_repeating_timer_ms(1, lvgl_tick_cb, NULL, &lvgl_timer);
  // lv_port_disp_init();

  // create_widgets();

  static struct repeating_timer led_timer;
  add_repeating_timer_ms(
    500, 
    led_timer_cb, 
    NULL, 
    &led_timer
  );
  


  while (true) {
    scanf("Press Enter to read sensor data...\n");
    if (read_sensor_data(&sensor_data)) {
      printf("Humidity: %.2f %%\n", sensor_data.humidity);
      printf("Temperature: %.2f °C\n", sensor_data.temperature);
      printf("Conductivity: %u µS/cm\n", sensor_data.conductivity);
      printf("pH: %.2f\n", sensor_data.pH);
    } else {
      printf("Failed to read sensor data\n");
    }

    // lv_timer_handler();
    sleep_ms(5);

    tight_loop_contents();
  }

  return 0;
}
