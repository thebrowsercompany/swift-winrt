file(GLOB TEST_APP_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.swift)

# XCTest dependency paths
set(XCTest_RootDir "$ENV{SDKROOT}/../../Library/XCTest-${SWIFT_VERSION}/usr")
string(REPLACE "\\" "/" XCTest_RootDir "${XCTest_RootDir}")

if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "ARM64")
    set(XCTest_BinaryDir "${XCTest_RootDir}/bin64a")
else()
    set(XCTest_BinaryDir "${XCTest_RootDir}/bin64")
endif()

add_executable(test_app ${TEST_APP_SOURCES})

target_link_libraries(test_app
    PRIVATE
        test_component
        Ctest_component)

find_library(XCTest_LIBRARY XCTest PATHS ${XCTest_BinaryDir} NO_DEFAULT_PATH)
if(NOT XCTest_LIBRARY)
    message(FATAL_ERROR "Failed to locate XCTest library in ${XCTest_BinaryDir}")
endif()

target_link_libraries(test_app PRIVATE ${XCTest_LIBRARY})

install(TARGETS test_app
    RUNTIME DESTINATION bin)

# install is used to copy the built binaries and dependencies to a known location
# where they can be run from and isn't used for distribution
install(FILES ${XCTest_BinaryDir}/XCTest.dll DESTINATION bin)

# LLDB  crashes on startup with an embedded manifest, so we're using a side-by-side
# manifest for now.
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/test_app.exe.manifest DESTINATION bin)
