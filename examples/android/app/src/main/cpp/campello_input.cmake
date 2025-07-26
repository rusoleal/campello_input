cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

include(FetchContent)

FetchContent_Declare(
        extern_campello_input
        SOURCE_DIR ../../../../../../..
)

FetchContent_GetProperties(extern_campello_input)

if(NOT extern_campello_input_POPULATED)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)

    message(STATUS "Fetching campello_input...")
    FetchContent_Populate(extern_campello_input)
    message(STATUS ${extern_campello_input_SOURCE_DIR})
    include_directories(${extern_campello_input_SOURCE_DIR}/inc)
    add_subdirectory(
            ${extern_campello_input_SOURCE_DIR}
            ${extern_campello_input_BINARY_DIR}
            EXCLUDE_FROM_ALL
    )
endif()
