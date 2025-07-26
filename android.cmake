
find_package(games-controller REQUIRED CONFIG)

add_library(${PROJECT_NAME} SHARED
    src/android/device.cpp
)

target_link_libraries(${PROJECT_NAME}
    games-controller::paddleboat_static
    log
)

target_include_directories (${PROJECT_NAME} PUBLIC 
                            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>"
                            )

target_include_directories(${PROJECT_NAME} PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )