add_executable(task_demo main.c usb_descriptors.c)

target_link_libraries(task_demo
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

pico_add_extra_outputs(task_demo)

include_directories(task_demo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
