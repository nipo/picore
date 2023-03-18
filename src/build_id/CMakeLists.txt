add_custom_target(generate_uuid_header
    COMMAND ${CMAKE_COMMAND}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/generate_uuid_header.cmake
    COMMENT "Generating build UUID"
)

set_source_files_properties(
    ${CMAKE_CURRENT_BINARY_DIR}/build_uuid.h
    PROPERTIES GENERATED TRUE
    )

add_library(pr_build_id build_id.c)
add_dependencies(pr_build_id generate_uuid_header)

target_link_libraries(pr_build_id PRIVATE
        pico_stdlib
        pico_binary_info
        pr_binary_info_reader
        )

target_include_directories (pr_build_id PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_include_directories (pr_build_id PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}
)
