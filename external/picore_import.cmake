# This is a copy of <PICORE_PATH>/external/picore_import.cmake

# This can be dropped into an external project to help locate picore
# It should be include()ed prior to project()

if (DEFINED ENV{PICORE_PATH} AND (NOT PICORE_PATH))
    set(PICORE_PATH $ENV{PICORE_PATH})
    message("Using PICORE_PATH from environment ('${PICORE_PATH}')")
endif ()

if (DEFINED ENV{PICORE_FETCH_FROM_GIT} AND (NOT PICORE_FETCH_FROM_GIT))
    set(PICORE_FETCH_FROM_GIT $ENV{PICORE_FETCH_FROM_GIT})
    message("Using PICORE_FETCH_FROM_GIT from environment ('${PICORE_FETCH_FROM_GIT}')")
endif ()

if (DEFINED ENV{PICORE_FETCH_FROM_GIT_PATH} AND (NOT PICORE_FETCH_FROM_GIT_PATH))
    set(PICORE_FETCH_FROM_GIT_PATH $ENV{PICORE_FETCH_FROM_GIT_PATH})
    message("Using PICORE_FETCH_FROM_GIT_PATH from environment ('${PICORE_FETCH_FROM_GIT_PATH}')")
endif ()

if (NOT PICORE_PATH)
    if (PICORE_FETCH_FROM_GIT)
        include(FetchContent)
        set(FETCHCONTENT_BASE_DIR_SAVE ${FETCHCONTENT_BASE_DIR})
        if (PICORE_FETCH_FROM_GIT_PATH)
            get_filename_component(FETCHCONTENT_BASE_DIR "${PICORE_FETCH_FROM_GIT_PATH}" REALPATH BASE_DIR "${CMAKE_SOURCE_DIR}")
        endif ()
        FetchContent_Declare(
                PICORE
                GIT_REPOSITORY https://github.com/nipo/picore
                GIT_TAG master
        )
        if (NOT PICORE)
            message("Downloading PICORE")
            FetchContent_Populate(PICORE)
            set(PICORE_PATH ${PICORE_SOURCE_DIR})
        endif ()
        set(FETCHCONTENT_BASE_DIR ${FETCHCONTENT_BASE_DIR_SAVE})
    else ()
        if (PICO_SDK_PATH AND EXISTS "${PICO_SDK_PATH}/../picore")
            set(PICORE_PATH ${PICO_SDK_PATH}/../picore)
            message("Defaulting PICORE_PATH as sibling of PICO_SDK_PATH: ${PICORE_PATH}")
        else()
            message(FATAL_ERROR
                    "PICORE location was not specified. Please set PICORE_PATH or set PICORE_FETCH_FROM_GIT to on to fetch from git."
                    )
        endif()
    endif ()
endif ()

set(PICORE_PATH "${PICORE_PATH}" CACHE PATH "Path to the PICORE")
set(PICORE_FETCH_FROM_GIT "${PICORE_FETCH_FROM_GIT}" CACHE BOOL "Set to ON to fetch copy of PICORE from git if not otherwise locatable")
set(PICORE_FETCH_FROM_GIT_PATH "${PICORE_FETCH_FROM_GIT_PATH}" CACHE FILEPATH "location to download EXTRAS")

get_filename_component(PICORE_PATH "${PICORE_PATH}" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
if (NOT EXISTS ${PICORE_PATH})
    message(FATAL_ERROR "Directory '${PICORE_PATH}' not found")
endif ()

set(PICORE_PATH ${PICORE_PATH} CACHE PATH "Path to the PICORE" FORCE)

add_subdirectory(${PICORE_PATH} picore)
