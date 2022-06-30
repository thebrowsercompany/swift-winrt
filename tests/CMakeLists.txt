set(CMAKE_TEST_OUTPUT ${CMAKE_BINARY_DIR})
set(CMAKE_TEST_COMPONENT_OUTPUT ${CMAKE_BINARY_DIR}/generated/test_component)

add_subdirectory(test_component)
add_subdirectory(test_component_cwinrt)
add_subdirectory(swift_bindings)
add_subdirectory(test_app)