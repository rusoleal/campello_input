cmake_minimum_required(VERSION 3.22.1)
project(minimal_example VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find campello_input - in real usage, this would be installed on the system
# For this example, we use FetchContent to pull from the parent directory
include(FetchContent)

# In your own project, you would use:
# find_package(campello_input 0.1.0 REQUIRED)
# target_link_libraries(minimal_example campello_input::campello_input)

# For this example, we build from the parent directory
FetchContent_Declare(
    campello_input
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..
)
FetchContent_MakeAvailable(campello_input)

# Create the example executable
add_executable(minimal_example main.cpp)

# Link against campello_input
target_link_libraries(minimal_example campello_input)

# Set platform-specific properties
if(APPLE)
    # macOS/iOS-specific settings
    set_target_properties(minimal_example PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
    )
elseif(ANDROID)
    # Android builds would use the android example project instead
    message(WARNING "For Android, use the examples/android project instead")
endif()

# Install target
install(TARGETS minimal_example
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION bin
)
