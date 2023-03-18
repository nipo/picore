add_executable(ws2812_blinker main.c)

target_link_libraries(ws2812_blinker
                      hardware_clocks
                      hardware_gpio
                      pico_stdlib
                      pr_pixel_led
                      )


pico_add_extra_outputs(ws2812_blinker)

include_directories(ws2812_blinker PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
