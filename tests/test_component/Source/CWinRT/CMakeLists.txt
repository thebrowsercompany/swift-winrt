
add_library(Ctest_component SHARED
  include/Ctest_component.h # Generatedoft
  shim.c)

target_include_directories(Ctest_component PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS Ctest_component
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

target_include_directories(Ctest_component
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE ${CMAKE_BINARY_DIR}/generated)

add_dependencies(Ctest_component test_component_cpp)
