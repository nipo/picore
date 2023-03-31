add_library(pr_cdc_interface cdc_interface.c)

target_link_libraries(pr_cdc_interface
        PRIVATE
        pico_stdlib
        pico_unique_id
        hardware_timer
	tinyusb_device_unmarked
        PUBLIC
        pr_task
        pr_fifo
        )

target_include_directories (pr_cdc_interface
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)
