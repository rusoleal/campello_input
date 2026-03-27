cmake_minimum_required(VERSION 3.22.1)
project(campello_input_macos_example)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD 17)

include(campello_input.cmake)

set(SOURCES
    src/main.mm
    src/AppDelegate.mm
    src/InputViewController.mm
    src/InputView.mm
)

add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST          ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
    MACOSX_BUNDLE_BUNDLE_NAME         "Input Monitor"
    MACOSX_BUNDLE_BUNDLE_VERSION      "1.0"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
    MACOSX_BUNDLE_GUI_IDENTIFIER      "systems.leal.campello-input-example-macos"
)

target_compile_options(${PROJECT_NAME} PRIVATE -fobjc-arc)

target_link_libraries(${PROJECT_NAME}
    campello_input
    "-framework AppKit"
    "-framework Foundation"
    objc
)
