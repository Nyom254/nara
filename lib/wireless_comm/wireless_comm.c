#include <stdio.h>
#include "pico/cyw43_arch.h"
#include "pico/async_context.h"
#include "lwip/altcp_tls.h"
#include "http_client_util.h"
#include "wireless_comm.h"
#include "../uart/sensor_comm/sensor_comm.h"


#define HOST "c8498509b5dd844af0af080068bfdce6.serveousercontent.com"
#define URL_REQUEST "/"


#define TLS_ROOT_CERT "-----BEGIN CERTIFICATE-----\n\
MIIEGzCCA6GgAwIBAgIQY2a1GNWPz7YRdF/s3+hcVDAKBggqhkjOPQQDAzBLMQsw\n\
CQYDVQQGEwJBVDEQMA4GA1UEChMHWmVyb1NTTDEqMCgGA1UEAxMhWmVyb1NTTCBF\n\
Q0MgRG9tYWluIFNlY3VyZSBTaXRlIENBMB4XDTI1MTEwMjAwMDAwMFoXDTI2MDEz\n\
MTIzNTk1OVowIDEeMBwGA1UEAxMVc2VydmVvdXNlcmNvbnRlbnQuY29tMFkwEwYH\n\
KoZIzj0CAQYIKoZIzj0DAQcDQgAEOsAH6KML4k1XMVqrzjsRc0/ApT7dxixATGlO\n\
ts3W4pcwT6XI9VEgNrfwBCzjnbAfUtE0msxrNA0jFfD/4XwXjqOCApAwggKMMB8G\n\
A1UdIwQYMBaAFA9r5kvOOUeu9n6QHnnwMJGSyF+jMB0GA1UdDgQWBBQIbpic/PXG\n\
j0nyp8vOqF7sltwSAzAOBgNVHQ8BAf8EBAMCB4AwDAYDVR0TAQH/BAIwADATBgNV\n\
HSUEDDAKBggrBgEFBQcDATBJBgNVHSAEQjBAMDQGCysGAQQBsjEBAgJOMCUwIwYI\n\
KwYBBQUHAgEWF2h0dHBzOi8vc2VjdGlnby5jb20vQ1BTMAgGBmeBDAECATCBiAYI\n\
KwYBBQUHAQEEfDB6MEsGCCsGAQUFBzAChj9odHRwOi8vemVyb3NzbC5jcnQuc2Vj\n\
dGlnby5jb20vWmVyb1NTTEVDQ0RvbWFpblNlY3VyZVNpdGVDQS5jcnQwKwYIKwYB\n\
BQUHMAGGH2h0dHA6Ly96ZXJvc3NsLm9jc3Auc2VjdGlnby5jb20wggEEBgorBgEE\n\
AdZ5AgQCBIH1BIHyAPAAdwCWl2S/VViXrfdDh2g3CEJ36fA61fak8zZuRqQ/D8qp\n\
xgAAAZpGJdoOAAAEAwBIMEYCIQCeU8GYjzCFjN0oGjl0BDdsRKlnsuSK0PZwIsJ1\n\
DhlhnQIhAKIh/Nrydg2yM/HoMElBQBjiZBzGaDgy5y+dOi7deppTAHUA0W6ppWgH\n\
fmY1oD83pd28A6U8QRIU1IgY9ekxsyPLlQQAAAGaRiXadgAABAMARjBEAiAFLam9\n\
uTcJ2+2PXW9FnWW1oPsk0BuHkwpZjMKzquPjdwIgDnesAMWChAcfYB8UOxxWjVMA\n\
2UofT8xEDqu7Mjy7+0wwOQYDVR0RBDIwMIIVc2VydmVvdXNlcmNvbnRlbnQuY29t\n\
ghcqLnNlcnZlb3VzZXJjb250ZW50LmNvbTAKBggqhkjOPQQDAwNoADBlAjBzDkws\n\
PFeY3Pb0Fc0VlYjV+3WtJm241B4y8SjpH/k9WldQObUYGn/8CPFtmyRiXqYCMQDa\n\
TrqytPpSKKSOaoAeL7ux1gWmgzxYiErj406KTWlS9GyNIFzMVrXa9i9WUSEvJ/g=\n\
-----END CERTIFICATE-----\n"
// Pico W Wifi Initialiation / Connection
bool wifi_init(){ 
  if(cyw43_arch_init()){
    printf("failed to initialized\n");
    sleep_ms(500);
    return false;
  }
  sleep_ms(1000);
  printf("initialized\n");

  cyw43_arch_enable_sta_mode();
  
  return true;
}

bool connect_to_wifi(){
  printf("Connecting to WiFi...\n" WIFI_SSID);
  if(cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 5000)){
    printf("failed to connect to wifi\n");
    printf("SSID: \"" WIFI_SSID "\"\n");
    printf("Password: \"" WIFI_PASSWORD "\"\n");
    return false;
  }
  printf("Connected to " WIFI_SSID "\n");
  return true;
}




// Send sensor data over WiFi to server
bool wifi_send_sensor_data(void){
  printf("Sending sensor data to server...\n");
  
  // This should work
  static const uint8_t cert_ok[] = TLS_ROOT_CERT;
  EXAMPLE_HTTP_REQUEST_T req = {0};
  req.hostname = HOST;
  req.url = URL_REQUEST;
  req.headers_fn = http_client_header_print_fn;
  req.recv_fn = http_client_receive_print_fn;
  req.tls_config = altcp_tls_create_config_client(cert_ok, sizeof(cert_ok));
  int pass = http_client_request_sync(cyw43_arch_async_context(), &req);

  altcp_tls_free_config(req.tls_config);
  printf("HTTP request completed with result: %d\n", pass);
  if(pass == 0){
    return true;
  }
  return false;
}

// static err_t post_headers_fn(http_client_t *client, void *arg) {
//     http_client_header(client, "Content-Type", "application/json");
//     return ERR_OK;
// }

// bool wifi_post_sensor_data(const char *json_body) {
//     EXAMPLE_HTTP_REQUEST_T req = {0};
//     req.hostname   = HOST;
//     req.url        = URL_REQUEST;
//     req.method     = HTTP_POST;
//     req.headers_fn = post_headers_fn;
//     req.body       = json_body;
//     req.body_len   = strlen(json_body);
//     req.recv_fn    = http_client_receive_print_fn;

//     int err = http_client_request_sync(
//         cyw43_arch_async_context(),
//         &req
//     );

//     return (err == 0);
// }

json_payload_t build_sensor_json(sensor_data_t *d) {
  json_payload_t payload;
  snprintf(payload.json, sizeof(payload.json),
        "{"
        "\"humidity\": 0,"
        "\"temperature\": 100,"
        "\"conductivity\": 100,"
        "\"ph\": 10"
        "}"
      );
    return payload;
}
