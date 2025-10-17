# XCTest dependency
set(XCTest_RootDir "$ENV{SDKROOT}/../../Library/XCTest-${SWIFT_VERSION}/usr")
string(REPLACE "\\" "/" XCTest_RootDir "${XCTest_RootDir}")

if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "ARM64")
    set(XCTest_BinaryDir "${XCTest_RootDir}/bin64a") 
else()
    set(XCTest_BinaryDir "${XCTest_RootDir}/bin64")
endif()

# install is used to copy the built binaries and dependencies to a known location
# where they can be run from and isn't used for distribution
install(FILES ${XCTest_BinaryDir}/XCTest.dll DESTINATION bin)

# LLDB  crashes on startup with an embedded manifest, so we're using a side-by-side
# manifest for now.
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/test_app.exe.manifest DESTINATION bin)
