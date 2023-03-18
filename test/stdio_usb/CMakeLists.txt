add_executable(stdio_usb main.c usb_descriptors.c)

target_link_libraries(stdio_usb
                      pico_stdlib
                      pico_stdio
                      pico_unique_id
                      pr_tiny_usb
                      pr_cdc_interface
                      tinyusb_device
                      pr_stdio_fifo
                      )

pico_add_extra_outputs(stdio_usb)

include_directories(stdio_usb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
