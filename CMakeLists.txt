cmake_minimum_required(VERSION 3.15.1) # For Swift support
if(NO_SWIFT)
    project(swiftwinrt LANGUAGES C CXX)
else()
    project(swiftwinrt LANGUAGES C CXX Swift)
endif()

if(NOT EXISTS "$ENV{WindowsSdkBinPath}${CMAKE_SYSTEM_VERSION}")
    message(WARNING "Windows SDK Version ${CMAKE_SYSTEM_VERSION} appears not to be installed")
endif()

add_subdirectory(swiftwinrt)
add_subdirectory(tests)