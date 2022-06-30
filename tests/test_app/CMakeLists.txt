add_executable(test_app
  main.swift)

target_link_libraries(test_app PRIVATE
  TestComponent_CWinRT)

target_link_libraries(test_app PRIVATE
  WinRT_Test)

set_target_properties(test_app PROPERTIES
  Swift_LANGUAGE_VERSION 5)

target_include_directories(test_app
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE ${CMAKE_BINARY_DIR}/generated)

install(TARGETS test_app
  DESTINATION bin
  COMPONENT exe)

# TONY: Is this 'file' mapping needed? If so, do we want it elsewhere?
#file(TO_NATIVE_PATH ${CMAKE_BINARY_DIR}/bin CMAKE_BINARY_DIR_WIN)
#add_custom_command(TARGET test_app POST_BUILD
#    COMMAND cmd.exe /c ${ArcCore_POST_BUILD_SCRIPT} ${CMAKE_BINARY_DIR_WIN})

# Add the manifest as a resource to ArcCoreDemoWin.exe
#
# Needed for ComCtl32 v6 support (e.g. modern app theming), and the WinRT
# activation manifest.
#
# Future: add minimal OS version number (for DComp / WinUI) and high DPI 
# support.
#
# Note: You need VS2019 tools in your path for mt.exe.
add_custom_command(TARGET test_app POST_BUILD
  COMMAND
  mt -nologo -manifest ${CMAKE_CURRENT_SOURCE_DIR}/test_app.exe.manifest -outputresource:$<TARGET_FILE:test_app>)
