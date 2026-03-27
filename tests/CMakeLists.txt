# MIT License
#
# Copyright (c) 2025 Ruben Leal Mirete
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

cmake_minimum_required(VERSION 3.22.1)

# Enable testing for this directory
enable_testing()

# -----------------------------------------------------------------------------
# Fetch Google Test
# -----------------------------------------------------------------------------
include(FetchContent)

# Try to find system-installed Google Test first
find_package(GTest QUIET)

if(NOT GTest_FOUND)
    # Download and build Google Test
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )

    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(googletest)
endif()

# -----------------------------------------------------------------------------
# Test Executable
# -----------------------------------------------------------------------------
add_executable(campello_input_tests
    unit/test_main.cpp
    unit/test_enums.cpp
    unit/test_state_structs.cpp
    unit/test_mock_devices.cpp
    unit/test_observer.cpp
)

# Link against the main library and Google Test
if(GTest_FOUND)
    target_link_libraries(campello_input_tests
        campello_input
        GTest::gtest_main
        GTest::gmock
    )
else()
    target_link_libraries(campello_input_tests
        campello_input
        gtest_main
        gmock
    )
endif()

# Include directories
target_include_directories(campello_input_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
    ${CMAKE_SOURCE_DIR}/inc
)

# -----------------------------------------------------------------------------
# CTest Integration
# -----------------------------------------------------------------------------
add_test(NAME campello_input_unit_tests COMMAND campello_input_tests)

# Optional: Auto-discover individual tests (requires test to be built first)
include(GoogleTest OPTIONAL)
if(COMMAND gtest_discover_tests)
    gtest_discover_tests(campello_input_tests)
endif()

# -----------------------------------------------------------------------------
# Code Coverage (optional)
# -----------------------------------------------------------------------------
option(ENABLE_COVERAGE "Enable code coverage reporting" OFF)

if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(campello_input_tests PRIVATE --coverage -O0)
    target_link_options(campello_input_tests PRIVATE --coverage)
    
    # Find lcov for coverage reporting
    find_program(LCOV_PATH lcov)
    find_program(GENHTML_PATH genhtml)
    
    if(LCOV_PATH AND GENHTML_PATH)
        add_custom_target(coverage
            COMMAND ${LCOV_PATH} --capture --directory . --output-file coverage.info
            COMMAND ${LCOV_PATH} --remove coverage.info '/usr/*' '*/tests/*' '*/googletest/*' --output-file coverage.info.cleaned
            COMMAND ${GENHTML_PATH} coverage.info.cleaned --output-directory coverage_report
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating code coverage report"
        )
    endif()
endif()


