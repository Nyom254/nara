#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

#include "sensor_comm.h"
#include "../sensor_data/sensor_data.h"

/* ================= GPIO ================= */

#define GPIO_ON  1
#define GPIO_OFF 0

/* ================= UART ================= */

#define UART_ID uart0
#define BAUD_RATE 9600

#define UART_TX_PIN 12
#define UART_RX_PIN 13

/* ================= RS485 ================= */

#define MAX485_DERE_PIN 11  // DE + RE tied together

/* ================= SENSOR ================= */

#define SENSOR_SLAVE_ADDRESS 0x05
#define RECEIVED_DATA_SIZE  19
#define REQUEST_DATA_LEN    8

#define RX_BUF_SIZE 19
#define MODBUS_FRAME_GAP_US 4000
#define RX_TIMEOUT_MS 3000
#define SENSOR_PERIOD_MS 1000

/* ================= STATE ================= */

typedef enum {
    SENSOR_IDLE,
    SENSOR_WAIT_RESPONSE
} sensor_state_t;

/* ================= GLOBALS ================= */

static volatile uint8_t rx_buffer[RX_BUF_SIZE];
static volatile size_t  rx_index = 0;
static volatile bool    frame_ready = false;
static volatile absolute_time_t rx_deadline;

static sensor_state_t sensor_state = SENSOR_IDLE;
static absolute_time_t sensor_timeout_deadline;
static absolute_time_t sensor_period_deadline;

static sensor_data_t sensor_temp;

/* ================= MODBUS REQUEST ================= */

static uint8_t data_to_sensor[REQUEST_DATA_LEN] = {
    0x05, // slave
    0x03, // read holding registers
    0x00, // start addr hi
    0x00, // start addr lo
    0x00, // reg count hi
    0x07, // reg count lo
    0x00, // CRC lo
    0x00  // CRC hi
};

/* ================= CRC ================= */

static uint16_t modbus_crc16(const uint8_t *data, uint16_t len) {
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t b = 0; b < 8; b++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

/* ================= UART RX IRQ ================= */

void on_uart_rx(void) {
    while (uart_is_readable(UART_ID)) {
        uint8_t ch = uart_getc(UART_ID);

        if (rx_index < RX_BUF_SIZE) {
            rx_buffer[rx_index++] = ch;
        } else {
            frame_ready = true;
            rx_index = RX_BUF_SIZE;
        }

        rx_deadline = make_timeout_time_us(MODBUS_FRAME_GAP_US);
    }
}

/* ================= UART INIT ================= */

void sensor_pin_init(void) {
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(UART_ID, false);

    irq_set_exclusive_handler(UART0_IRQ, on_uart_rx);
    irq_set_enabled(UART0_IRQ, true);
    uart_set_irq_enables(UART_ID, true, false);

    gpio_init(MAX485_DERE_PIN);
    gpio_set_dir(MAX485_DERE_PIN, GPIO_OUT);
    gpio_put(MAX485_DERE_PIN, GPIO_OFF);

    sensor_period_deadline = make_timeout_time_ms(SENSOR_PERIOD_MS);
}

/* ================= SEND REQUEST ================= */

static void send_data_to_sensor(void) {
    uint16_t crc = modbus_crc16(data_to_sensor, REQUEST_DATA_LEN - 2);
    data_to_sensor[6] = crc & 0xFF;
    data_to_sensor[7] = crc >> 8;

    gpio_put(MAX485_DERE_PIN, GPIO_ON);
    uart_write_blocking(UART_ID, data_to_sensor, REQUEST_DATA_LEN);
    uart_tx_wait_blocking(UART_ID);
    gpio_put(MAX485_DERE_PIN, GPIO_OFF);
}

/* ================= FRAME GAP CHECK ================= */

static void modbus_rx_poll(void) {
    if (rx_index > 0 && !frame_ready) {
        if (time_reached(rx_deadline)) {
            frame_ready = true;
        }
    }
}

/* ================= PARSER ================= */

static bool parse_sensor_frame(sensor_data_t *data) {
    if (rx_index != RECEIVED_DATA_SIZE) return false;

    if (rx_buffer[0] != SENSOR_SLAVE_ADDRESS ||
        rx_buffer[1] != 0x03 ||
        rx_buffer[2] != 0x0E) {
        return false;
    }

    uint16_t crc_rx = rx_buffer[17] | (rx_buffer[18] << 8);
    uint16_t crc_calc = modbus_crc16(rx_buffer, 17);

    if (crc_rx != crc_calc) return false;

    uint16_t hum  = (rx_buffer[3] << 8) | rx_buffer[4];
    int16_t  temp = (rx_buffer[5] << 8) | rx_buffer[6];
    uint16_t cond = (rx_buffer[7] << 8) | rx_buffer[8];
    uint16_t ph   = (rx_buffer[9] << 8) | rx_buffer[10];
    uint16_t n    = (rx_buffer[11] << 8) | rx_buffer[12];
    uint16_t p    = (rx_buffer[13] << 8) | rx_buffer[14];
    uint16_t k    = (rx_buffer[15] << 8) | rx_buffer[16];

    data->humidity     = hum / 10.0f;
    data->temperature  = temp / 10.0f;
    data->conductivity = cond;
    data->pH           = ph / 10.0f;
    data->nitrogen     = n;
    data->phosphorus   = p;
    data->potassium    = k;

    return true;
}

/* ================= NON-BLOCKING TASK ================= */

void sensor_task(void) {
    switch (sensor_state) {

    case SENSOR_IDLE:
        if (time_reached(sensor_period_deadline)) {
            rx_index = 0;
            frame_ready = false;
            rx_deadline = make_timeout_time_us(MODBUS_FRAME_GAP_US);

            send_data_to_sensor();

            sensor_timeout_deadline =
                make_timeout_time_ms(RX_TIMEOUT_MS);

            sensor_state = SENSOR_WAIT_RESPONSE;
        }
        break;

    case SENSOR_WAIT_RESPONSE:
        modbus_rx_poll();

        if (frame_ready) {
            if (parse_sensor_frame(&sensor_temp)) {
                sensor_data_set(&sensor_temp);
            }

            sensor_period_deadline =
                make_timeout_time_ms(SENSOR_PERIOD_MS);
            sensor_state = SENSOR_IDLE;
        }
        else if (time_reached(sensor_timeout_deadline)) {
            sensor_period_deadline =
                make_timeout_time_ms(SENSOR_PERIOD_MS);
            sensor_state = SENSOR_IDLE;
        }
        break;
    }
}
