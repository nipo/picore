add_library(pr_uart uart.c)
target_link_libraries(pr_uart PRIVATE
        pico_stdlib
        pr_task
        pr_fifo
        hardware_uart
        )
target_include_directories (pr_uart PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)
