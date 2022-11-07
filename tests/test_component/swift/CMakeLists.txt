
set(GENERATED_FILES_DIR ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB SWIFTWINRT_GENERATED_FILES ${GENERATED_FILES_DIR}/*)
file(GLOB SUPPORT_FILES ${GENERATED_FILES_DIR}/Support/*)
add_library(test_component SHARED
  ${SWIFTWINRT_GENERATED_FILES}
  ${SUPPORT_FILES}
  )

install(TARGETS test_component
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  COMPONENT lib)

target_link_libraries(test_component PRIVATE
  Ctest_component
  )

target_include_directories(test_component
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_include_directories(test_component
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE ${GENERATED_FILES_DIR})

