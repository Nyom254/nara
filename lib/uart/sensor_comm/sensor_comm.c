#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "sensor_comm.h"

#define GPIO_ON 1
#define GPIO_OFF 0

// UART DEFINE
#define UART_ID uart0
#define BAUD_RATE 9600
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

#define UART_TX_PIN 16
#define UART_RX_PIN 17


// MAX485 CONTROL PINS
#define MAX485_DERE_PIN 22  // Data Enable Pin

// TOTAL BYTES RECEIVED FROM SENSOR
#define RECEIVED_DATA_SIZE 13

// REQUEST DATA TO SENSOR LENGTH
#define REQUEST_DATA_LEN 8

#define RX_BUF_SIZE 13
#define MODBUS_FRAME_GAP_US 4000   // ~3.5 char times @9600

#define RX_TIMEOUT_MS 3000     // Total timeout for receiving data

static volatile uint8_t rx_buffer[RX_BUF_SIZE];
static volatile size_t rx_index = 0;
static volatile bool frame_ready = false;
static absolute_time_t rx_deadline;


// REQUIRED SENSOR SENT DATA FORMAT
uint8_t data_to_sensor[REQUEST_DATA_LEN] = {
    0x03, // Slave Address 
    0x03, // Function Code
    0x00, // Starting Address High Byte
    0x00, // starting Address Low Byte
    0x00, // No. of Registers High Byte
    0x04, // No. of Registers Low Byte
    0x00, // CRC Low Byte (to be filled)
    0x00  // CRC High Byte (to be filled)
};

// MODBUS CRC16 CALCULATION
uint16_t modbus_crc16(uint8_t *data, uint16_t len) {
    uint16_t crc = 0xFFFF;
  uint8_t i, j = 0;
  while (j < len) {
    crc ^= data[j];
    for (i = 0; i < 8; i++) {
      if (crc & 0x01) {
        crc >>= 1;
        crc ^= 0xA001;
      } else
        crc >>= 1;
    }
    j++;
  }
  return crc;
}

    // SEND REQUEST DATA TO SENSOR
void send_data_to_sensor() {
    uint16_t crc = modbus_crc16(data_to_sensor, sizeof(data_to_sensor) - 2);
    data_to_sensor[6] = crc & 0xFF;         // CRC Low Byte
    data_to_sensor[7] = (crc >> 8) & 0xFF;      // CRC High Byte

    sleep_ms(500); // Wait before sending

    gpio_put(MAX485_DERE_PIN, 1);
    uart_write_blocking(UART_ID, data_to_sensor, REQUEST_DATA_LEN);
    uart_tx_wait_blocking(UART_ID); 
    gpio_put(MAX485_DERE_PIN, 0);
    print_hex_array(data_to_sensor, REQUEST_DATA_LEN);
}

void modbus_rx_poll(void) {
    if (rx_index > 0 && !frame_ready) {
        if (time_reached(rx_deadline)) {
            frame_ready = true; // frame ended by silence
        }
    }
}


// PARSE RECEIVED DATA FROM SENSOR
bool parse_sensor_frame(sensor_data_t *data) {
    if (rx_index != 13) return false;
    if (rx_buffer[0] != 0x03 || rx_buffer[1] != 0x03 || rx_buffer[2] != 0x08) {
        printf("Invalid Function Code or Byte Count\n");
        return false;
    }

    uint16_t crc_rx =
        rx_buffer[11] | (rx_buffer[12] << 8);

    uint16_t crc_calc =
        modbus_crc16(rx_buffer, 11);

    if (crc_rx != crc_calc) {
        printf("CRC Mismatch: RX: %04X, CALC: %04X\n", crc_rx, crc_calc);
        return false;
    }

    uint16_t hum =
        (rx_buffer[3] << 8) | rx_buffer[4];
    int16_t temp =
        (int16_t)((rx_buffer[5] << 8) | rx_buffer[6]);
    uint16_t cond =
        (rx_buffer[7] << 8) | rx_buffer[8];
    uint16_t ph =
        (rx_buffer[9] << 8) | rx_buffer[10];

    data->humidity     = hum / 10.0f;
    data->temperature  = temp / 10.0f;
    data->conductivity = cond;
    data->pH           = ph / 10.0f;

    return true;
}



static int chars_rxed = 0;


void print_hex_array(const uint8_t *data, size_t size) {
    
    // 1. Iterate through the array
    for (size_t i = 0; i < size; i++) {
        // 2. Print each byte using the hex format specifier
        printf("%02X", data[i]);
        
        // Add a space for readability, except after the last byte
        if (i < size - 1) {
            printf(" ");
        }
    }
    printf("\n");
}

// uart rx interrupt handler
void on_uart_rx(void) {
    while (uart_is_readable(UART_ID)) {
        uint8_t ch = uart_getc(UART_ID);
        if (rx_index < RX_BUF_SIZE) {
            rx_buffer[rx_index++] = ch;
        }
        rx_deadline = make_timeout_time_us(MODBUS_FRAME_GAP_US);
    }
}


void sensor_pin_init() {
    // init uart pins
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // uart config
    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(UART_ID, false);
    // uart rx irq handler 
    irq_set_exclusive_handler(UART0_IRQ, on_uart_rx);
    irq_set_enabled(UART0_IRQ, true);
    uart_set_irq_enables(UART_ID, true, false);

    gpio_init(MAX485_DERE_PIN);
    gpio_set_dir(MAX485_DERE_PIN, GPIO_OUT);
    gpio_put(MAX485_DERE_PIN, GPIO_OFF); // Initialize to receive mode
}


bool read_sensor_data(sensor_data_t* data) {
    rx_index = 0;
    frame_ready = false;

    send_data_to_sensor(); // Send request to sensor

    absolute_time_t deadline = make_timeout_time_ms(RX_TIMEOUT_MS);

    while (!time_reached(deadline)) {
        modbus_rx_poll();
        if (frame_ready) {
            printf("Frame received with %d bytes\n", rx_index);
            print_hex_array((const uint8_t*)rx_buffer, rx_index);
            bool ok = parse_sensor_frame(data);
            return ok;
        }
        tight_loop_contents(); // Allow other tasks to run
    }
    printf("Timeout waiting for sensor data\n");
    return false; // timeout
}
