add_library(pr_queue queue.c)
target_link_libraries(pr_queue PRIVATE
        pr_task
        pico_stdlib
        pico_malloc
        hardware_sync
        PUBLIC
        pr_error
        )
target_include_directories (pr_queue PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR}
)
