
# Windows/Xbox GDK platform using GameInput API

add_library(${PROJECT_NAME} STATIC
    src/gdk/device.cpp
    src/gdk/input_gdk_system.cpp
    src/gdk/gamepad_gdk.cpp
    src/gdk/keyboard_gdk.cpp
    src/gdk/mouse_gdk.cpp
    src/gdk/haptics_gdk.cpp
    src/input_system.cpp
)

# Set library versioning
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
)

target_link_libraries(${PROJECT_NAME}
    GameInput
)

target_include_directories(${PROJECT_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>"
)

target_include_directories(${PROJECT_NAME} PRIVATE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/gdk/inc>"
)

target_include_directories(${PROJECT_NAME} PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>"
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    CAMPHELLO_INPUT_GDK
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)

message(STATUS "Windows/Xbox input: Using GameInput API (GDK)")
