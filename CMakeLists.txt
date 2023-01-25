cmake_minimum_required(VERSION 3.10)
project(swiftwinrt)

add_subdirectory(swiftwinrt)

if(NOT SKIP_TESTS)
    add_subdirectory(tests)
endif()