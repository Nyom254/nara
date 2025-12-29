#include <stdio.h>
#include "pico/cyw43_arch.h"
#include "../uart/sensor_comm/sensor_comm.h"
#include "wireless_comm.h"
#include "btstack.h"
#include "../sensor_data/sensor_data.h"
#include "ble.h"

#define HEARTBEAT_PERIOD_MS 1000

static btstack_timer_source_t heartbeat;
static hci_con_handle_t con_handle = HCI_CON_HANDLE_INVALID;
static btstack_packet_callback_registration_t btstack_event_callback;

static volatile bool ble_connected = false;
static volatile bool ble_state_changed = false;

typedef struct __attribute__((packed)) {
    int16_t humidity_x10;      // 68.2% → 682
    int16_t temperature_x10;   // 27.3°C → 273
    uint16_t conductivity;    // µS/cm
    int16_t ph_x100;           // 6.75 → 675
    uint16_t nitrogen;
    uint16_t phosphorus;
    uint16_t potassium;
} sensor_ble_packet_t;

static void sensor_to_ble_packet(sensor_ble_packet_t *p,
                                 const sensor_data_t *d) {
    p->humidity_x10    = (int16_t)(d->humidity * 10);
    p->temperature_x10 = (int16_t)(d->temperature * 10);
    p->conductivity    = (uint16_t)d->conductivity;
    p->ph_x100         = (int16_t)(d->pH * 100);
    p->nitrogen        = (uint16_t)d->nitrogen;
    p->phosphorus      = (uint16_t)d->phosphorus;
    p->potassium       = (uint16_t)d->potassium;
}


// Advertising data
static uint8_t adv_data[] = {
  // Flags: General Discoverable
  0x02, 0x01, 0x06,
  // Name: nara
  0x05, 0x09, 'n', 'a','r', 'a'
};
uint16_t att_read_callback(hci_con_handle_t con_handle,
                            uint16_t att_handle, uint16_t offset,
                            uint8_t * buffer, uint16_t buffer_size){
    if(att_handle == ATT_CHARACTERISTIC_0887f28c_0000_40b5_9f88_a8bfd08a2aa6_01_VALUE_HANDLE){
      sensor_data_t snapshot;
      sensor_ble_packet_t pkt;
      if (!sensor_data_get(&snapshot, NULL)) {
          return 0; // no data yet
      }
      sensor_to_ble_packet(&pkt, &snapshot);
      return att_read_callback_handle_blob((const uint8_t*)&pkt, sizeof(sensor_ble_packet_t), offset, buffer, buffer_size);
    }
    return 0;
}

int att_write_callback(hci_con_handle_t con_handle,
                             uint16_t att_handle, uint16_t transaction_mode,
                             uint16_t offset, const uint8_t * buffer, uint16_t buffer_size){
    UNUSED(con_handle);
    UNUSED(att_handle);
    UNUSED(transaction_mode);
    UNUSED(offset);
    UNUSED(buffer);
    UNUSED(buffer_size);
    return ATT_ERROR_WRITE_NOT_PERMITTED;
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
        ble_connected = false;
        ble_state_changed = true;
        printf("Disconnected Re-enabling advertising\n");
        gap_advertisements_enable(1);
        break;
      case HCI_EVENT_LE_META:
        switch (hci_event_le_meta_get_subevent_code(packet)) {
          case HCI_SUBEVENT_LE_CONNECTION_COMPLETE:
            con_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
            ble_connected = true;
            ble_state_changed = true;
            gap_advertisements_enable(0);
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

bool ble_is_connected(void) {
    return ble_connected;
}

bool ble_is_state_changed(void) {
    if (ble_state_changed) {
        ble_state_changed = false;
        return true;
    }
    return false;
}


