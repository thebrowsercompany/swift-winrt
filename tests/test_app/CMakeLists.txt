add_executable(test_app
  main.swift)

target_link_libraries(test_app PRIVATE
  Ctest_component)

target_link_libraries(test_app PRIVATE
  test_component)

set_target_properties(test_app PROPERTIES
  Swift_LANGUAGE_VERSION 5)

target_include_directories(test_app
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE ${CMAKE_BINARY_DIR}/generated)

install(TARGETS test_app
  DESTINATION bin
  COMPONENT exe)

# LLDB  crashes on startup with an embedded manifest, so we're using a side-by-side
# manifest for now.
add_custom_command(TARGET test_app POST_BUILD
  COMMAND
  ${CMAKE_COMMAND} -E copy
  ${CMAKE_CURRENT_SOURCE_DIR}/test_app.exe.manifest
  ${CMAKE_BINARY_DIR}/bin/test_app.exe.manifest)

# Add the manifest as a resource. You need VS2019 tools in your path for mt.exe.
# add_custom_command(TARGET test_app POST_BUILD
#   COMMAND
#   mt -nologo -manifest ${CMAKE_CURRENT_SOURCE_DIR}/test_app.exe.manifest -outputresource:$<TARGET_FILE:test_app>)
