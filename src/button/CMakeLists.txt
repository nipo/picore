add_library(pr_button button.c)

target_link_libraries(pr_button PRIVATE
        pico_stdlib
        PUBLIC
        pr_task
        )

target_include_directories (pr_button PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)
