cmake_minimum_required(VERSION 3.22.1)
project(glfw_example VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies
find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)

# Find campello_input - in real usage, this would be installed on the system
include(FetchContent)
FetchContent_Declare(
    campello_input
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..
)
FetchContent_MakeAvailable(campello_input)

# Create the example executable
add_executable(glfw_example main.cpp)

# Link libraries
target_link_libraries(glfw_example 
    campello_input
    glfw
    OpenGL::GL
)

# Platform-specific settings
if(APPLE)
    # macOS needs additional frameworks
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    target_link_libraries(glfw_example 
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREVIDEO_LIBRARY}
    )
elseif(WIN32)
    # Windows needs additional libraries for GLFW
    target_link_libraries(glfw_example 
        gdi32
        winmm
        imm32
    )
endif()

# Install target
install(TARGETS glfw_example
    RUNTIME DESTINATION bin
)

# Print instructions
message(STATUS "")
message(STATUS "GLFW Example Configuration:")
message(STATUS "  GLFW found: ${glfw3_VERSION}")
message(STATUS "  OpenGL found: ${OpenGL_VERSION}")
message(STATUS "")
message(STATUS "Build and run:")
message(STATUS "  cmake --build . --target glfw_example")
message(STATUS "  ./glfw_example")
message(STATUS "")
