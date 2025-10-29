cmake_minimum_required(VERSION 3.15.1)
project(swiftwinrt LANGUAGES C CXX Swift)

if (DEFINED SWIFT_VERSION)
    if (SWIFT_VERSION STREQUAL "0.0.0")
        set(SWIFT_VERSION "6.3" CACHE STRING "Swift toolchain version" FORCE)
    endif()
else()
    set(SWIFT_VERSION "6.3" CACHE STRING "Swift toolchain version")
endif()

# Apply the shared Swift toolchain configuration to every Swift target in the
# project. The language version ensures the compiler front-end matches the
# desired toolchain release, while the default module name keeps the test
# executable consistent with its previous SPM identity.
set(CMAKE_Swift_LANGUAGE_VERSION 6.3)
set(CMAKE_Swift_MODULE_NAME SwiftWinRTTests)

option(BUILD_SHARED_LIBS "Build shared libraries by default" YES)

if(NOT EXISTS "$ENV{WindowsSdkBinPath}${CMAKE_SYSTEM_VERSION}")
    message(FATAL_ERROR "Windows SDK Version appears not to be installed:\n  Missing folder: $ENV{WindowsSdkBinPath}${CMAKE_SYSTEM_VERSION}")
endif()

add_subdirectory(swiftwinrt)
add_subdirectory(tests)

if (NOT DEFINED ENV{NUGET_PACKAGE_VERSION})
    set(NUGET_PACKAGE_VERSION 0.1.0)
else()
    set(NUGET_PACKAGE_VERSION $ENV{NUGET_PACKAGE_VERSION})
endif()

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/nuget/TheBrowserCompany.SwiftWinRT.nupkg
    COMMAND ${CMAKE_BINARY_DIR}/nuget pack ${CMAKE_CURRENT_SOURCE_DIR}\\nuget\\swiftwinrt.nuspec -Properties swiftwinrt_exe=${CMAKE_CURRENT_SOURCE_DIR}\\out\\${CMAKE_BUILD_TYPE}\\bin\\swiftwinrt.exe -Version ${NUGET_PACKAGE_VERSION}
)

add_custom_target(nuget
    DEPENDS ${CMAKE_BINARY_DIR}/nuget/TheBrowserCompany.SwiftWinRT.nupkg)

add_dependencies(nuget install)
