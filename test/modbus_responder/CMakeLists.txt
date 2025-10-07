cmake_minimum_required(VERSION 3.12)

add_executable(modbus_responder main.c usb_descriptors.c)

include_directories(modbus_responder PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(modbus_responder
pico_multicore
pico_stdlib
pico_stdio
pr_tiny_usb
pr_stdio_fifo
pr_fifo
pr_task
pr_uart
pr_cdc_interface
tinyusb_device
pr_modbus)

pico_add_extra_outputs(modbus_responder)
