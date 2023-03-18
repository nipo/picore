add_library(pr_error error.c)
target_link_libraries(pr_error PUBLIC
        pico_stdlib
        )
target_include_directories (pr_error PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)
