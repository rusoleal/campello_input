# Linux platform using evdev

# Find required system libraries
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBEVDEV REQUIRED libevdev)
pkg_check_modules(LIBUDEV REQUIRED libudev)

set(PLATFORM_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/linux/input_linux_system.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/linux/gamepad_linux.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/linux/keyboard_linux.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/linux/mouse_linux.cpp
)

set(PLATFORM_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/linux/inc
    ${LIBEVDEV_INCLUDE_DIRS}
    ${LIBUDEV_INCLUDE_DIRS}
)

set(PLATFORM_LIBRARIES
    ${LIBEVDEV_LIBRARIES}
    ${LIBUDEV_LIBRARIES}
)

set(PLATFORM_DEFS
    CAMPHELLO_INPUT_LINUX
    ${LIBEVDEV_CFLAGS_OTHER}
    ${LIBUDEV_CFLAGS_OTHER}
)

list(APPEND INPUT_SOURCES ${PLATFORM_SOURCES})
list(APPEND INPUT_INCLUDE_DIRS ${PLATFORM_INCLUDE_DIRS})
list(APPEND INPUT_LIBRARIES ${PLATFORM_LIBRARIES})
list(APPEND INPUT_DEFS ${PLATFORM_DEFS})

message(STATUS "Linux input: Using evdev via libevdev and libudev")
