add_library(pr_crc32 crc32.c)
target_link_libraries(pr_crc32 PRIVATE
        pico_stdlib
        )
target_include_directories (pr_crc32 PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)
