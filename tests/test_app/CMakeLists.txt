add_executable(test_app main.swift)
install(TARGETS test_app DESTINATION bin COMPONENT exe)
  
set_target_properties(test_app PROPERTIES Swift_LANGUAGE_VERSION 5)

target_include_directories(test_app
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE ${CMAKE_BINARY_DIR}/generated)
target_link_libraries(test_app PRIVATE Ctest_component)
target_link_libraries(test_app PRIVATE test_component)

# XCTest dependency
set(XCTest_DIR "C:/Library/Developer/Platforms/Windows.platform/Developer/Library/XCTest-development")
set(XCTest_LIBRARY "${XCTest_DIR}/usr/lib/swift/windows/x86_64")
target_include_directories(test_app PRIVATE "${XCTest_LIBRARY}")
target_link_directories(test_app PRIVATE "${XCTest_LIBRARY}")
add_custom_command(TARGET test_app POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "${XCTest_DIR}/usr/bin64/XCTest.dll" ${CMAKE_CURRENT_BINARY_DIR}/XCTest.dll)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/XCTest.dll DESTINATION bin)

# LLDB  crashes on startup with an embedded manifest, so we're using a side-by-side
# manifest for now.
add_custom_command(TARGET test_app POST_BUILD
  COMMAND
  ${CMAKE_COMMAND} -E copy
  ${CMAKE_CURRENT_SOURCE_DIR}/test_app.exe.manifest
  ${CMAKE_CURRENT_BINARY_DIR}/test_app.exe.manifest)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/test_app.exe.manifest DESTINATION bin)

# Add the manifest as a resource. You need VS2019 tools in your path for mt.exe.
# add_custom_command(TARGET test_app POST_BUILD
#   COMMAND
#   mt -nologo -manifest ${CMAKE_CURRENT_SOURCE_DIR}/test_app.exe.manifest -outputresource:$<TARGET_FILE:test_app>)
