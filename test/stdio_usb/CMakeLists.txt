add_executable(stdio_usb main.c usb_descriptors.c)

target_link_libraries(stdio_usb
                      hardware_clocks
                      pico_stdlib
                      pico_stdio
                      pico_unique_id
                      pr_fifo
                      pr_task
                      pr_tiny_usb
                      pr_cdc_interface
                      pr_stdio_fifo
                      tinyusb_device
                      )

pico_add_extra_outputs(stdio_usb)

include_directories(stdio_usb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
