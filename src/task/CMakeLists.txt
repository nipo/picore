add_library(pr_task task.c)
target_link_libraries(pr_task PRIVATE
        pico_stdlib
        pico_time
        hardware_sync
        )
target_include_directories (pr_task PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)
