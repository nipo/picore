add_library(pr_pixel_led pixel_led.c)

pico_generate_pio_header(pr_pixel_led ${CMAKE_CURRENT_LIST_DIR}/pixel_led.pio)

target_link_libraries(pr_pixel_led PRIVATE
        pico_stdlib
        PUBLIC
        hardware_pio
        pr_task
        )


target_include_directories (pr_pixel_led PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)
