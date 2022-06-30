
add_library(TestComponent_CWinRT SHARED
test_component.h                     # Generated
shim.c)

target_include_directories(TestComponent_CWinRT PUBLIC
${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS TestComponent_CWinRT
ARCHIVE DESTINATION lib
LIBRARY DESTINATION lib
RUNTIME DESTINATION bin)

target_include_directories(TestComponent_CWinRT
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
PRIVATE ${CMAKE_BINARY_DIR}/generated)

add_custom_command(
OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_component.h
COMMAND ${CMAKE_SOURCE_DIR}/tools/abi.exe -ns-prefix -input ${CMAKE_TEST_COMPONENT_OUTPUT}/test_component.winmd -output ${CMAKE_BINARY_DIR}/generated -reference 10.0.20348.0)
