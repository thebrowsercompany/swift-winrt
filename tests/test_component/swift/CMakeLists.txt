add_library(test_component SHARED
  test_component.swift
  test_component+ABI.swift
  test_component.Delegates.Swift
  test_component.Delegates+ABI.swift
  Windows.Foundation.swift
  Windows.Foundation+Abi.swift
  NamespaceDefinitions.swift
  Support/Aggregation.swift
  Support/IUnknown.swift
  Support/IUnknown+Swift.Swift
  Support/IUnknownRef.swift
  Support/COM+Extensions.swift
  Support/Error.swift
  Support/ErrorHandling.swift
  Support/Event.swift
  Support/HString.swift
  Support/HStringStorage.swift
  Support/IInspectable.swift
  Support/IInspectable+Swift.swift
  Support/RawTyped.swift
  Support/Runtime+Swift.swift
  Support/StringHelpers.swift
  Support/Swift+Extensions.swift
  Support/TrustLevel+Swift.swift
  Support/WinRTWrapperBase.swift
  Support/WinSDK+Extensions.swift
  )

install(TARGETS test_component
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  COMPONENT lib)

target_link_libraries(test_component PRIVATE
  Ctest_component)

target_link_libraries(test_component PRIVATE
  ole32)

target_include_directories(test_component
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(test_component
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE ${CMAKE_BINARY_DIR}/generated)

