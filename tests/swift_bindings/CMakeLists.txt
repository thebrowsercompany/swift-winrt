add_library(WinRT_Test SHARED
  Generated/winrt/test_component.swift
  Generated/winrt/test_component+ABI.swift
  Support/IUnknown.swift
  Support/IUnknown+Swift.Swift
  Support/IUnknownRef.swift
  Support/COM+Extensions.swift
  Support/Error.swift
  Support/ErrorHandling.swift
  Support/GeneratedCodeDefinitions.swift
  Support/HString.swift
  Support/HStringStorage.swift
  Support/IInspectable.swift
  Support/IInspectable+Swift.swift
  Support/RawTyped.swift
  Support/Runtime+Swift.swift
  Support/StringHelpers.swift
  Support/Swift+Extensions.swift
  Support/TrustLevel+Swift.swift
  Support/WinSDK+Extensions.swift
  )

install(TARGETS WinRT_Test
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  COMPONENT lib)

target_link_libraries(WinRT_Test PRIVATE
  TestComponent_CWinRT)

target_link_libraries(WinRT_Test PRIVATE
  ole32)

target_include_directories(WinRT_Test
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(WinRT_Test
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE ${CMAKE_BINARY_DIR}/generated)

  
# Since a lot of the bindings code isn't fully implemented, don't run the tool
# during build as this could modify the checked-in files as well as break the
# build.
#add_dependencies(WinRT swiftwinrt)
#add_custom_command(
#  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/Generated/ArcCoreWinRT.swift
#  COMMAND ${CMAKE_BINARY_DIR}/bin/swiftwinrt.exe -ns-prefix -input ${ArcCore_INCLUDE_DIR}/bin/ArcCoreWin.winmd -output ${CMAKE_CURRENT_SOURCE_DIR}/Generated/ -import TestComponent_CWinRT -reference 10.0.20348.0)

