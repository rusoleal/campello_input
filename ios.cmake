
add_library(${PROJECT_NAME} SHARED
    src/apple/haptics_apple.mm
    src/apple/gamepad_apple.mm
    src/apple/keyboard_apple.mm
    src/apple/mouse_apple.mm
    src/apple/touch_apple.mm
    src/apple/input_apple_system.mm
    src/input_system.cpp
)

# Set library versioning
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    FRAMEWORK FALSE
)

target_link_libraries(${PROJECT_NAME}
    "-framework GameController"
    "-framework CoreHaptics"
    "-framework UIKit"
    "-framework Foundation"
    objc
)

target_include_directories(${PROJECT_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>"
)

target_include_directories(${PROJECT_NAME} PRIVATE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/apple/inc>"
)

target_include_directories(${PROJECT_NAME} PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>"
)
