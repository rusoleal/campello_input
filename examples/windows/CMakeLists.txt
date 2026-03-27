cmake_minimum_required(VERSION 3.22.1)
project(campello_input_windows_example VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows-specific settings
if(NOT WIN32)
    message(FATAL_ERROR "This example is only for Windows")
endif()

# Include campello_input from parent directory
include(FetchContent)
FetchContent_Declare(
    campello_input
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..
)
FetchContent_MakeAvailable(campello_input)

# Create executable
add_executable(${PROJECT_NAME} WIN32
    src/main.cpp
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    campello_input
)

# Windows subsystem (not console)
set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
)

# Install
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# Print instructions
message(STATUS "")
message(STATUS "Windows Example Configuration:")
message(STATUS "  Build with: cmake --build . --target ${PROJECT_NAME}")
message(STATUS "  Run with: ./${PROJECT_NAME}.exe")
message(STATUS "")
