add_library(pr_struct_compose struct_compose.c)
target_include_directories (pr_struct_compose PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)
