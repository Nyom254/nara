#ifndef SENSOR_COMM_H
#define SENSOR_COMM_H

typedef struct{
    float humidity;
    float temperature;
    float conductivity;
    float pH;
    float nitrogen;
    float phosphorus;
    float potassium;
} sensor_data_t;


void sensor_pin_init();

bool read_sensor_data(sensor_data_t* data);


#endif
