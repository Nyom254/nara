#ifndef WIRELESS_COMM_H

#define WIRELESS_COMM_H

void wifi_init();

bool wifi_send_sensor_data(char* data, size_t length);

#endif
