#pragma once
#include "lib/lvgl/lvgl.h"
#include "lib/uart/sensor_comm/sensor_comm.h"
/* UI lifecycle */
void ui_init(void);

/* UI updates */
void ui_update_sensor(const sensor_data_t * s);
void ui_update_battery(uint8_t percent);
void ui_update_wifi(bool connected);
