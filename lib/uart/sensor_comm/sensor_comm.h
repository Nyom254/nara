typedef struct{
    float humidity;
    float temperature;
    float conductivity;
    float pH;
} sensor_data_t;


void sensor_pin_init();

bool read_sensor_data(sensor_data_t* data);


