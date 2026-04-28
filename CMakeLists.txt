cmake_minimum_required(VERSION 3.16)

project(
    netronix
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

#
# Header-only library
#

add_library(netronix INTERFACE)

target_include_directories(netronix
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

#
# Compile warnings (propagowane do użytkowników biblioteki)
#

target_compile_options(netronix
    INTERFACE
        -Wall
        -Wextra
        -Wpedantic
)

#
# GoogleTest
#

find_package(GTest REQUIRED)

include(GoogleTest)

#
# Unit tests
#

add_executable(frame_ut
    test/frame_ut.cpp
)

target_link_libraries(frame_ut
    PRIVATE
        netronix
        GTest::gtest_main
)

target_compile_options(frame_ut
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
)

gtest_discover_tests(frame_ut)