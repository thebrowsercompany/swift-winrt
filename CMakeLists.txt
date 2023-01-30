cmake_minimum_required(VERSION 3.15.1) # For Swift support
project(swiftwinrt LANGUAGES C CXX Swift)

option(BUILD_SHARED_LIBS "Build shared libraries by default" YES)

if(NOT EXISTS "$ENV{WindowsSdkBinPath}${CMAKE_SYSTEM_VERSION}")
    message(WARNING "Windows SDK Version ${CMAKE_SYSTEM_VERSION} appears not to be installed")
endif()

add_subdirectory(swiftwinrt)
add_subdirectory(tests)