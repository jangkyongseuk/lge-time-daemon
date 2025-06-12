cmake_minimum_required(VERSION 3.16)
project(lge-time VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 --coverage")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Find required packages
find_package(Threads REQUIRED)

# Main executable
add_executable(lge-time lge-time.cpp)
target_link_libraries(lge-time Threads::Threads)

# Create a library for testing
add_library(lge-time-lib STATIC lge-time-lib.cpp lge-time-lib.h)
target_link_libraries(lge-time-lib Threads::Threads)

# Enable testing
enable_testing()

# Find Google Test
find_package(GTest QUIET)
if(NOT GTest_FOUND)
    # Download and build Google Test
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/v1.14.0.zip
    )
    FetchContent_MakeAvailable(googletest)
endif()

# Test executable
add_executable(lge-time-test test/test_lge_time.cpp)
target_link_libraries(lge-time-test 
    lge-time-lib 
    gtest_main 
    gmock_main
    Threads::Threads
)

# Add test
add_test(NAME lge-time-unit-tests COMMAND lge-time-test)

# Code coverage setup
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    find_program(GCOV_PATH gcov)
    find_program(LCOV_PATH lcov)
    find_program(GENHTML_PATH genhtml)
    
    if(GCOV_PATH AND LCOV_PATH AND GENHTML_PATH)
        # Add coverage target
        add_custom_target(coverage
            COMMAND ${LCOV_PATH} --directory . --zerocounters
            COMMAND ${CMAKE_CTEST_COMMAND} --verbose
            COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage.info
            COMMAND ${LCOV_PATH} --remove coverage.info '/usr/*' '*/test/*' '*/googletest/*' --output-file coverage.info.cleaned
            COMMAND ${GENHTML_PATH} -o coverage_html coverage.info.cleaned
            COMMAND ${CMAKE_COMMAND} -E remove coverage.info coverage.info.cleaned
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating code coverage report"
        )
        
        message(STATUS "Code coverage enabled. Use 'make coverage' to generate report.")
    else()
        message(WARNING "Code coverage tools not found. Install lcov and gcov for coverage reports.")
    endif()
endif()

# Install targets
install(TARGETS lge-time DESTINATION bin)
install(FILES lge-time.service DESTINATION /etc/systemd/system OPTIONAL)
