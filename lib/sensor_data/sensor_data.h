#pragma once

#include "pico/mutex.h"
#include <stdbool.h>


typedef struct{
    float humidity;
    float temperature;
    float conductivity;
    float pH;
    float nitrogen;
    float phosphorus;
    float potassium;
} sensor_data_t;


/* Initialize shared sensor data storage */
void sensor_data_shared_init(void);

/* Update data (writer: sensor task / timer) */
void sensor_data_set(const sensor_data_t *data);

/* Read latest data (reader: UI / BLE / MQTT) */
bool sensor_data_get(sensor_data_t *out, uint32_t *data_seq);

