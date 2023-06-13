
set(GENERATED_FILES_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SWIFTWINRT_GENERATED_FILES
  Windows.Foundation.swift
  Windows.Foundation+ABI.swift
  Windows.Foundation+Impl.swift
  test_component.swift
  test_component+ABI.swift
  test_component+Impl.swift
  test_component.Delegates.swift
  test_component.Delegates+ABI.swift
  test_component.Delegates+Impl.swift
  test_component+Generics.swift
)
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

