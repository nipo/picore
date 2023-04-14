add_library(pr_endian endian.c)
target_include_directories (pr_endian PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)
