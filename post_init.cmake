add_library(picore_included INTERFACE)
target_compile_definitions(picore_included INTERFACE
        -DPICORE=1
        )

pico_add_platform_library(picore_included)

# note as we're a .cmake included by the SDK, we're relative to the pico-sdk build
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src ${CMAKE_BINARY_DIR}/picore/src)

if (PICORE_TESTS_ENABLED OR PICORE_TOP_LEVEL_PROJECT)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/test ${CMAKE_BINARY_DIR}/picore/test)
endif ()
