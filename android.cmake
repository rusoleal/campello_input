
add_library(${PROJECT_NAME} SHARED
    src/android/device.cpp
)

target_link_libraries(${PROJECT_NAME}
        log)

target_include_directories (${PROJECT_NAME} PUBLIC 
                            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>"
                            )

target_include_directories(${PROJECT_NAME} PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )