#ifndef WIRELESS_COMM_H

#define WIRELESS_COMM_H

#include "../uart/sensor_comm/sensor_comm.h"

typedef struct {
    char json[128];
} json_payload_t;

bool wifi_init();

bool connect_to_wifi();
json_payload_t build_sensor_data_json(sensor_data_t *d);




#endif
