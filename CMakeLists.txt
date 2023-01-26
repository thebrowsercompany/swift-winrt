cmake_minimum_required(VERSION 3.10)
project(swiftwinrt)

if(NOT EXISTS "$ENV{WindowsSdkBinPath}${CMAKE_SYSTEM_VERSION}")
    message(WARNING "Windows SDK Version ${CMAKE_SYSTEM_VERSION} appears not to be installed")
endif()

add_subdirectory(swiftwinrt)
add_subdirectory(tests)