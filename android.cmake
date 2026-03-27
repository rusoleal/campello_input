
add_library(${PROJECT_NAME} STATIC
    src/android/manager.cpp
    src/android/device.cpp
    src/android/input_android_system.cpp
    src/android/touch_android.cpp
    src/android/keyboard_android.cpp
    src/android/mouse_android.cpp
    src/android/haptics_android.cpp
    src/input_system.cpp
)

# Set library versioning (for consistency, though static libs don't use SOVERSION)
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
)

find_package(game-activity REQUIRED CONFIG)

target_link_libraries(${PROJECT_NAME}
        game-activity::game-activity
        android
        log
        ${DEPENDENCIES}
)

target_include_directories (${PROJECT_NAME} PUBLIC 
                            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>"
                            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/android/inc>"
                            )

target_include_directories(${PROJECT_NAME} PUBLIC
                           "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>"
                           )
