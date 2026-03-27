# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

cmake_minimum_required(VERSION 3.22.1)

project("example")

include(${PROJECT_SOURCE_DIR}/campello_input.cmake)
set(DEPENDENCIES
        ${DEPENDENCIES}
        campello_input
)

add_library(example SHARED
        main.cpp
        InputRenderer.cpp
        AndroidOut.cpp)

find_package(game-activity REQUIRED CONFIG)

target_link_libraries(example
        game-activity::game-activity
        EGL
        GLESv3
        android
        log
        ${DEPENDENCIES}
)
