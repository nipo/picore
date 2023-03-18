add_library(pr_stdio_fifo stdio_fifo.c)
target_link_libraries(pr_stdio_fifo PRIVATE
        pico_stdlib
        pr_task
        pico_stdio
        PUBLIC
        pr_fifo
        )
target_include_directories (pr_stdio_fifo PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)
