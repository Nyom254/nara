cmake_minimum_required(VERSION 3.28)

# Initialize the SDK based on PICO_SDK_PATH
include(pico_sdk_import.cmake)

project(nara C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

set(LV_BUILD_CONF_PATH lv_conf.h)
add_subdirectory(lib/lvgl)

add_executable(jinawi_kw
  main.c
  lib/wireless_comm/wireless_comm.c
  lib/uart/sensor_comm/sensor_comm.c
  lib/spi/display_comm/display_comm.c
  lib/ui/ui.c
)

pico_enable_stdio_usb(jinawi_kw 1)
pico_enable_stdio_uart(jinawi_kw 1)

target_include_directories(jinawi_kw PRIVATE 
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/lib/lvgl
)

target_link_libraries(jinawi_kw
  pico_stdlib
  pico_cyw43_arch_none
  pico_btstack_ble
  pico_btstack_cyw43
  hardware_uart
  hardware_spi
  lvgl
)

# Generate header file from the .gatt file
pico_btstack_make_gatt_header(jinawi_kw PRIVATE ${CMAKE_CURRENT_LIST_DIR}/lib/wireless_comm/ble.gatt)

# target_compile_definitions(jinawi_kw PRIVATE
# )

# produce additional file to upload to pico
pico_add_extra_outputs(jinawi_kw)
