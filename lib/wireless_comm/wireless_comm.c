#include <stdio.h>
#include "pico/cyw43_arch.h"
#include "pico/async_context.h"
#include "lwip/altcp_tls.h"
#include "http_client_util.h"


#define HOST "captive.apple.com"
#define URL_REQUEST "/hotspot-detect.html"


#define TLS_ROOT_CERT "-----BEGIN CERTIFICATE-----\n\
MIICiTCCAg+gAwIBAgIQH0evqmIAcFBUTAGem2OZKjAKBggqhkjOPQQDAzCBhTEL\n\
MAkGA1UEBhMCR0IxGzAZBgNVBAgTEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UE\n\
BxMHU2FsZm9yZDEaMBgGA1UEChMRQ09NT0RPIENBIExpbWl0ZWQxKzApBgNVBAMT\n\
IkNPTU9ETyBFQ0MgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMDgwMzA2MDAw\n\
MDAwWhcNMzgwMTE4MjM1OTU5WjCBhTELMAkGA1UEBhMCR0IxGzAZBgNVBAgTEkdy\n\
ZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UEBxMHU2FsZm9yZDEaMBgGA1UEChMRQ09N\n\
T0RPIENBIExpbWl0ZWQxKzApBgNVBAMTIkNPTU9ETyBFQ0MgQ2VydGlmaWNhdGlv\n\
biBBdXRob3JpdHkwdjAQBgcqhkjOPQIBBgUrgQQAIgNiAAQDR3svdcmCFYX7deSR\n\
FtSrYpn1PlILBs5BAH+X4QokPB0BBO490o0JlwzgdeT6+3eKKvUDYEs2ixYjFq0J\n\
cfRK9ChQtP6IHG4/bC8vCVlbpVsLM5niwz2J+Wos77LTBumjQjBAMB0GA1UdDgQW\n\
BBR1cacZSBm8nZ3qQUfflMRId5nTeTAOBgNVHQ8BAf8EBAMCAQYwDwYDVR0TAQH/\n\
BAUwAwEB/zAKBggqhkjOPQQDAwNoADBlAjEA7wNbeqy3eApyt4jf/7VGFAkK+qDm\n\
fQjGGoe9GKhzvSbKYAydzpmfz1wPMOG+FDHqAjAU9JM8SaczepBGR7NjfRObTrdv\n\
GDeAU/7dIOA1mjbRxwG55tzd8/8dLDoWV9mSOdY=\n\
-----END CERTIFICATE-----\n"
// Pico W Wifi Initialiation / Connection
void wifi_init(){ 

  cyw43_arch_enable_sta_mode();
  
  printf("Connecting to WiFi...\n" WIFI_SSID);
  if(cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 1000)){
    printf("failed to connect to wifi\n");
    printf(WIFI_SSID "\n");
    printf(WIFI_PASSWORD "\n");
  } else {
    printf("Connected\n");
  }
}

// Send sensor data over WiFi to server
bool wifi_send_sensor_data(char* data, size_t length){
  
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

    if(pass == 0){
      return true;
    }
  return false;
}
