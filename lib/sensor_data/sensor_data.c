#include "sensor_data.h"

/* Private globals */
static sensor_data_t g_sensor_data = {0};
static mutex_t g_sensor_mutex;
static volatile uint32_t g_data_seq;

void sensor_data_shared_init(void) {
    // mutex initialization
    mutex_init(&g_sensor_mutex);
    return;
}

void sensor_data_set(const sensor_data_t *data) {
    mutex_enter_blocking(&g_sensor_mutex);
    g_sensor_data = *data;
    g_data_seq++;
    mutex_exit(&g_sensor_mutex);
}

bool sensor_data_get(sensor_data_t *out, uint32_t *data_seq) {

    bool changed = false;
    mutex_enter_blocking(&g_sensor_mutex);
    if (!data_seq || *data_seq != g_data_seq) {
        if (out) {
            *out = g_sensor_data;
        }
        if (data_seq) {
            *data_seq = g_data_seq;
        }
        changed = true;
    }
    mutex_exit(&g_sensor_mutex);

    return changed;
}
