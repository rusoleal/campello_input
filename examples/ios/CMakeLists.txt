cmake_minimum_required(VERSION 3.22.1)
project(campello_input_ios_example VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# iOS-specific settings
if(NOT APPLE)
    message(FATAL_ERROR "This example is only for iOS/macOS")
endif()

set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0")
set(CMAKE_OSX_SYSROOT "iphoneos")
set(CMAKE_OSX_ARCHITECTURES "arm64")

# Include campello_input
include(FetchContent)
FetchContent_Declare(
    campello_input
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..
)
FetchContent_MakeAvailable(campello_input)

# Find required frameworks
find_library(UIKIT_FRAMEWORK UIKit REQUIRED)
find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
find_library(GAMECONTROLLER_FRAMEWORK GameController REQUIRED)
find_library(COREHAPTICS_FRAMEWORK CoreHaptics REQUIRED)

# Create executable (iOS app bundle)
set(SOURCES
    src/main.m
    src/AppDelegate.m
    src/AppDelegate.h
    src/InputViewController.mm
    src/InputViewController.h
    src/InputView.mm
    src/InputView.h
)

add_executable(${PROJECT_NAME} ${SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
    MACOSX_BUNDLE_BUNDLE_NAME "Input Monitor"
    MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
    MACOSX_BUNDLE_GUI_IDENTIFIER "systems.leal.campello-input-example-ios"
    XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer"
    XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ""
    XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"  # iPhone, iPad
)

target_compile_options(${PROJECT_NAME} PRIVATE -fobjc-arc)

target_link_libraries(${PROJECT_NAME}
    campello_input
    ${UIKIT_FRAMEWORK}
    ${FOUNDATION_FRAMEWORK}
    ${GAMECONTROLLER_FRAMEWORK}
    ${COREHAPTICS_FRAMEWORK}
    objc
)

# Xcode specific
set_target_properties(${PROJECT_NAME} PROPERTIES
    XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
    XCODE_ATTRIBUTE_ENABLE_BITCODE NO
)

message(STATUS "")
message(STATUS "iOS Example Configuration:")
message(STATUS "  Build with: cmake --build . --target ${PROJECT_NAME}")
message(STATUS "  Requires: Xcode with iOS SDK")
message(STATUS "")
