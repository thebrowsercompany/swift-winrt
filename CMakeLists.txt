cmake_minimum_required(VERSION 3.15.1) # For Swift support
project(swiftwinrt LANGUAGES C CXX Swift)

option(BUILD_SHARED_LIBS "Build shared libraries by default" YES)

if(NOT EXISTS "$ENV{WindowsSdkBinPath}${CMAKE_SYSTEM_VERSION}")
    message(WARNING "Windows SDK Version ${CMAKE_SYSTEM_VERSION} appears not to be installed")
endif()

add_subdirectory(swiftwinrt)
add_subdirectory(tests)

set(NUGET_VERSION 0.0.0.1)
if (DEFINED ENV{NUGET_HASH})
    set(NUGET_VERSION_WITH_SUFFIX ${NUGET_VERSION}-$ENV{NUGET_HASH})
else()
    set(NUGET_VERSION_WITH_SUFFIX ${NUGET_VERSION}-local)
endif()

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/nuget/company.thebrowser.swiftwinrt.nupkg
    COMMAND ${CMAKE_BINARY_DIR}/nuget pack ${CMAKE_CURRENT_SOURCE_DIR}\\nuget\\swiftwinrt.nuspec -Properties swiftwinrt_exe=${CMAKE_CURRENT_SOURCE_DIR}\\out\\${CMAKE_BUILD_TYPE}\\bin\\swiftwinrt.exe -Version ${NUGET_VERSION_WITH_SUFFIX}
)
        
add_custom_target(nuget 
    DEPENDS ${CMAKE_BINARY_DIR}/nuget/company.thebrowser.swiftwinrt.nupkg)