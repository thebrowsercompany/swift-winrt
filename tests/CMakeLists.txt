set(CMAKE_TEST_OUTPUT ${CMAKE_BINARY_DIR})
set(CMAKE_TEST_COMPONENT_OUTPUT ${CMAKE_BINARY_DIR}/generated/test_component)

add_subdirectory(test_component)
if(NOT NO_SWIFT)
    add_subdirectory(test_app)
endif()