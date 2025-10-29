set(CMAKE_TEST_OUTPUT ${CMAKE_BINARY_DIR})
set(CMAKE_TEST_COMPONENT_OUTPUT ${CMAKE_BINARY_DIR}/generated/test_component)

if (NOT EXISTS ${CMAKE_TEST_COMPONENT_OUTPUT})
  file(MAKE_DIRECTORY ${CMAKE_TEST_COMPONENT_OUTPUT})
endif()

# Enable Swift for the test targets.
enable_language(Swift)

add_subdirectory(test_component)
add_subdirectory(test_app)
