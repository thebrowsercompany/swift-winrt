project(WinRT_Test)

set(WINMD_FILE
    ${CMAKE_TEST_COMPONENT_OUTPUT}/test_component.winmd
)

set(SWIFT_WINRT_PARAMETERS
"-ns-prefix"
"-include test_component"
"-input ${WINMD_FILE}"
"-support test_component"
"-test"
"-log"
"-output ${CMAKE_CURRENT_SOURCE_DIR}"
"-reference ${CMAKE_SYSTEM_VERSION}"
)

string(REPLACE ";" "\n" SWIFT_WINRT_PARAMETERS "${SWIFT_WINRT_PARAMETERS}")

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/SwiftWinRT.rsp ${SWIFT_WINRT_PARAMETERS})

add_subdirectory(cpp)

# re-run swiftwinrt to ensure the bindings get updated
set(SWIFTWINRT_PARAM_FILE ${CMAKE_CURRENT_BINARY_DIR}/SwiftWinRT.rsp)
# set the NO_AUTO_CODEGEN environment variable to disable this behavior when needing to
# work on hand-generated bindings
if (NOT DEFINED ENV{NO_AUTO_CODEGEN})
    add_custom_target(GenerateBindings
      BYPRODUCTS ${CMAKE_CURRENT_SOURCE_DIR}/Sources/test_component/test_component.swift
      DEPENDS ${SWIFTWINRT_PARAM_FILE}
      DEPENDS ${WINMD_FILE}
      COMMAND ${CMAKE_BINARY_DIR}/swiftwinrt/swiftwinrt.exe @${SWIFTWINRT_PARAM_FILE})
    add_dependencies(GenerateBindings swiftwinrt)
    add_dependencies(test_app GenerateBindings)
else()
    message(STATUS "Skipping auto codegen")
endif()