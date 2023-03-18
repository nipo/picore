add_executable(binary_info main.c)

target_link_libraries(binary_info
                      pico_stdlib
                      pico_stdio
                      pico_unique_id
                      pr_build_id
                      pr_binary_info_reader
                      tinyusb_device_unmarked
                      )

pico_enable_stdio_usb(binary_info 1)
pico_enable_stdio_uart(binary_info 0)

pico_add_extra_outputs(binary_info)
