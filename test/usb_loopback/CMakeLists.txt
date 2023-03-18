add_executable(usb_loopback main.c usb_descriptors.c)

include_directories(usb_loopback PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(usb_loopback PUBLIC
                      pico_multicore
                      pico_stdlib
                      pr_fifo
                      pr_task
                      pr_tiny_usb
                      pr_cdc_interface
                      tinyusb_device
                      )

pico_add_extra_outputs(usb_loopback)
