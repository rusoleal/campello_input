
# Windows/Xbox GDK platform using GameInput API

set(PLATFORM_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gdk/device.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gdk/input_gdk_system.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gdk/gamepad_gdk.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gdk/keyboard_gdk.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gdk/mouse_gdk.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gdk/haptics_gdk.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/input_system.cpp
)

set(PLATFORM_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gdk/inc
)

set(PLATFORM_LIBRARIES
    GameInput
)

set(PLATFORM_DEFS
    CAMPHELLO_INPUT_GDK
    NOMINMAX  # Prevent Windows headers from defining min/max macros
    WIN32_LEAN_AND_MEAN
)

list(APPEND INPUT_SOURCES ${PLATFORM_SOURCES})
list(APPEND INPUT_INCLUDE_DIRS ${PLATFORM_INCLUDE_DIRS})
list(APPEND INPUT_LIBRARIES ${PLATFORM_LIBRARIES})
list(APPEND INPUT_DEFS ${PLATFORM_DEFS})

message(STATUS "Windows/Xbox input: Using GameInput API (GDK)")
