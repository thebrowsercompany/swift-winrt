set(CMAKE_TEST_OUTPUT ${CMAKE_BINARY_DIR})
set(CMAKE_TEST_COMPONENT_OUTPUT ${CMAKE_BINARY_DIR}/generated/test_component)
set(TEST_COMPONENT_WINMD_FILE ${CMAKE_TEST_COMPONENT_OUTPUT}/test_component.winmd)

add_subdirectory(test_component)
add_subdirectory(test_app)