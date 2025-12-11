cmake_minimum_required(VERSION 3.28)

# Initialize the SDK based on PICO_SDK_PATH
include(pico_sdk_import.cmake)

project(wireless_con C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()


add_executable(jinawi_kw
  main.c
  lib//wireless_comm/wireless_comm.c
)

pico_enable_stdio_usb(jinawi_kw 1)
pico_enable_stdio_uart(jinawi_kw 1)

target_include_directories(jinawi_kw PRIVATE $(CMAKE_CURRENT_LIST_DIR))


target_link_libraries(jinawi_kw
  pico_stdlib
  pico_cyw43_arch_lwip_threadsafe_background
)

target_compile_definitions(jinawi_kw PRIVATE
    WIFI_SSID=\"${WIFI_SSID}\"
    WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
)

# produce additional file to upload to pico
pico_add_extra_outputs(jinawi_kw)
