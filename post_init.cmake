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

# Doxygen documentation (only for top-level builds)
if (PICORE_TOP_LEVEL_PROJECT)
    option(BUILD_DOCS "Build API documentation with Doxygen" OFF)

    if (BUILD_DOCS)
        find_package(Doxygen)

        if (DOXYGEN_FOUND)
            # Set Doxygen options
            set(DOXYGEN_PROJECT_NAME "Pico-RE")
            set(DOXYGEN_PROJECT_BRIEF "Pico Runtime Environment")
            set(DOXYGEN_PROJECT_NUMBER "")
            set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/docs")
            set(DOXYGEN_GENERATE_HTML YES)
            set(DOXYGEN_GENERATE_LATEX NO)
            set(DOXYGEN_EXTRACT_ALL YES)
            set(DOXYGEN_EXTRACT_PRIVATE NO)
            set(DOXYGEN_EXTRACT_STATIC YES)
            set(DOXYGEN_RECURSIVE YES)
            set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${CMAKE_CURRENT_LIST_DIR}/readme.md")
            set(DOXYGEN_JAVADOC_AUTOBRIEF YES)
            set(DOXYGEN_OPTIMIZE_OUTPUT_FOR_C YES)
            set(DOXYGEN_TYPEDEF_HIDES_STRUCT YES)
            set(DOXYGEN_SHOW_INCLUDE_FILES YES)
            set(DOXYGEN_SORT_MEMBER_DOCS NO)
            set(DOXYGEN_WARN_IF_UNDOCUMENTED YES)
            set(DOXYGEN_WARN_NO_PARAMDOC YES)
            set(DOXYGEN_FILE_PATTERNS "*.h" "*.c" "*.dox")
            set(DOXYGEN_EXCLUDE_PATTERNS "*/build/*")
            set(DOXYGEN_EXAMPLE_PATH "${CMAKE_CURRENT_LIST_DIR}/test")
            set(DOXYGEN_IMAGE_PATH "${CMAKE_CURRENT_LIST_DIR}/doc")

            # Create Doxygen target
            doxygen_add_docs(
                docs
                ${CMAKE_CURRENT_LIST_DIR}/src
                ${CMAKE_CURRENT_LIST_DIR}/doc
                ${CMAKE_CURRENT_LIST_DIR}/readme.md
                COMMENT "Generating API documentation with Doxygen"
            )

            message(STATUS "Doxygen found: documentation target 'docs' available")
            message(STATUS "Build docs with: cmake --build . --target docs")
        else()
            message(WARNING "Doxygen not found, documentation cannot be built")
        endif()
    endif()
endif()
