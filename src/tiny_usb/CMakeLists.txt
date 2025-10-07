add_library(pr_tiny_usb tiny_usb.c)

target_link_libraries(pr_tiny_usb PUBLIC
        pico_stdlib
        pico_unique_id
        pico_bootrom
        hardware_watchdog
        pr_crc32
        pico_usb_reset_interface_headers
        pr_task
        )

target_include_directories (pr_tiny_usb PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)
