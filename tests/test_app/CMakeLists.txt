# XCTest dependency
set(XCTest_RootDir "$ENV{SDKROOT}/../../Library/XCTest-development/usr")
string(REPLACE "\\" "/" XCTest_RootDir "${XCTest_RootDir}")
set(XCTest_BinaryDir "${XCTest_RootDir}/bin64")

install(FILES ${XCTest_BinaryDir}/XCTest.dll DESTINATION bin)

# LLDB  crashes on startup with an embedded manifest, so we're using a side-by-side
# manifest for now.
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/test_app.exe.manifest DESTINATION bin)
