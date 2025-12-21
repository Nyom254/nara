cmake_minimum_required(VERSION 3.28)

# Initialize the SDK based on PICO_SDK_PATH
include(pico_sdk_import.cmake)

project(wireless_con C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()


# Http client utility library for lwIP + mbedTLS
# pico_add_library(lwip_http_util NOFLAG)
# target_sources(lwip_http_util INTERFACE
#         ${CMAKE_CURRENT_LIST_DIR}/lib/wireless_comm/http/http_client_util.c
#         )
# pico_mirrored_target_link_libraries(lwip_http_util INTERFACE
#         pico_lwip_http
#         pico_lwip_mbedtls
#         pico_mbedtls
#         )
# target_include_directories(lwip_http_util INTERFACE
#         ${CMAKE_CURRENT_LIST_DIR}
#         )

# MQTT client utility library for lwIP + mbedTLS
pico_add_library(lwip_mqtt_util NOFLAG)
target_sources(lwip_mqtt_util INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/lib/wireless_comm/mqtt/mqtt_client.c
        )
target_link_libraries(lwip_mqtt_util INTERFACE
        pico_lwip_mqtt
        pico_lwip_mbedtls
        pico_mbedtls
        )
target_include_directories(lwip_mqtt_util INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
        )

set(LV_BUILD_CONF_PATH lv_conf.h)
add_subdirectory(lib/lvgl)



add_executable(jinawi_kw
  main.c
  lib/wireless_comm/wireless_comm.c
  lib/uart/sensor_comm/sensor_comm.c
  lib/spi/display_comm/display_comm.c
  img/heart1.c
  img/heart2.c
)

pico_enable_stdio_usb(jinawi_kw 1)
pico_enable_stdio_uart(jinawi_kw 1)

target_include_directories(jinawi_kw PRIVATE 
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/lib/lvgl
        )


target_link_libraries(jinawi_kw
  pico_stdlib
  pico_cyw43_arch_lwip_threadsafe_background
  pico_lwip
  pico_lwip_mbedtls
  pico_mbedtls
  lwip_mqtt_util
  hardware_uart
  hardware_spi
  lvgl
)

target_compile_definitions(jinawi_kw PRIVATE
    WIFI_SSID=\"${WIFI_SSID}\"
    WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
    ALTCP_MBEDTLS_AUTHMODE=MBEDTLS_SSL_VERIFY_REQUIRED
)

# produce additional file to upload to pico
pico_add_extra_outputs(jinawi_kw)
