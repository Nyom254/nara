// mqtt_client.h
#pragma once
#include <stdbool.h>

void mqtt_init(void);
void mqtt_poll(void);
bool mqtt_publish_json(const char *json);
