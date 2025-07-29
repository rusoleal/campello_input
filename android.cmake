
add_library(${PROJECT_NAME} STATIC
    src/android/manager.cpp
    src/android/device.cpp
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
                           "${PROJECT_BINARY_DIR}"
                           )