add_library(pr_fifo fifo.c)
target_link_libraries(pr_fifo PRIVATE
        pr_task
        pico_stdlib
        pico_malloc
        hardware_sync
        )
target_include_directories (pr_fifo PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR}
)
