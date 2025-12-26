#include <stdio.h>
#include "pico/cyw43_arch.h"
#include "../uart/sensor_comm/sensor_comm.h"
#include "wireless_comm.h"
#include "btstack.h"

#include "ble.h"

#define HEARTBEAT_PERIOD_MS 1000

static btstack_timer_source_t heartbeat;
static hci_con_handle_t con_handle = HCI_CON_HANDLE_INVALID;
static uint8_t sensor_data = 8;
static btstack_packet_callback_registration_t btstack_event_callback;

// Advertising data
static uint8_t adv_data[] = {
  // Flags: General Discoverable
  0x02, 0x01, 0x06,
  // Name: Nara
  0x05, 0x09, 'N', 'a','r', 'a'
};
uint16_t att_read_callback(hci_con_handle_t con_handle,
                            uint16_t att_handle, uint16_t offset,
                            uint8_t * buffer, uint16_t buffer_size){
    if(att_handle == ATT_CHARACTERISTIC_0000FF11_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE){
        return att_read_callback_handle_blob((const uint8_t*)&sensor_data, 1, offset, buffer, buffer_size);
    }
    return 0;
}

int att_write_callback(hci_con_handle_t con_handle,
                             uint16_t att_handle, uint16_t transaction_mode,
                             uint16_t offset, const uint8_t * buffer, uint16_t buffer_size){
    UNUSED(con_handle);
    UNUSED(transaction_mode);
    UNUSED(offset);
    if(att_handle == ATT_CHARACTERISTIC_0000FF11_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE){
        if(buffer_size == 1){
            sensor_data = buffer[0];
            printf("Received sensor data via BLE: %d\n", sensor_data);
        }
    }
    return buffer_size;
}

static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    UNUSED(channel);
    if (packet_type != HCI_EVENT_PACKET) return;

    switch (hci_event_packet_get_type(packet)) {
      case BTSTACK_EVENT_STATE:
      {
        if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) return; 
        printf("BTstack up and running. Advertising..\n");
        gap_advertisements_set_data(sizeof(adv_data), adv_data);
        gap_advertisements_enable(1);
        break;
      }
      case HCI_EVENT_DISCONNECTION_COMPLETE:
        con_handle = HCI_CON_HANDLE_INVALID;
        printf("Disconnected Re-enabling advertising\n");
        gap_advertisements_enable(1);
        break;
      case HCI_EVENT_LE_META:
        switch (hci_event_le_meta_get_subevent_code(packet)) {
          case HCI_SUBEVENT_LE_CONNECTION_COMPLETE:
            con_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
            printf("Connected\n");
            break;
          default:
              break;
        }
        break;
      default:
          break;
    }
}



// Pico W Bluetooth Initialiation / Connection
bool ble_init(){ 
  // Initialize L2CAP and GATT
  l2cap_init();
  sm_init();

  att_server_init(profile_data, att_read_callback, att_write_callback);

  // Turn On Bluetooth
  hci_power_control(HCI_POWER_ON);

  // Register Packet Handler
  btstack_event_callback.callback = &packet_handler;
  hci_add_event_handler(&btstack_event_callback);
  printf("initialized\n");
  return true;
}

json_payload_t build_sensor_data_json(sensor_data_t *d) {
  json_payload_t payload;
  snprintf(payload.json, sizeof(payload.json),
        "{"
        "\"humidity\": %.2f,"
        "\"temperature\": %.2f,"
        "\"conductivity\": %.2f,"
        "\"ph\": %.2f,"
        "\"nitrogen\": %.2f,"
        "\"phosphorus\": %.2f,"
        "\"potassium\": %.2f"
        "}"
      , d->humidity, d->temperature, d->conductivity, d->pH, d->nitrogen, d->phosphorus, d->potassium);
    return payload;
}
