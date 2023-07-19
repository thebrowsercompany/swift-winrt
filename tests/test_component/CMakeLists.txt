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
# the language server needs to be killed before running swiftwinrt
# https://linear.app/the-browser-company/issue/WIN-161/swift-file-cannot-be-saved-locked-by-sourcekit-lsp
add_custom_target(KillLSP
    COMMAND (taskkill /im sourcekit-lsp.exe /f 2> nul || echo ...success) # need to || echo to ignore errors when the process isn't running
    COMMENT "Shutting down Language Server process"
    VERBATIM
)
add_custom_target(GenerateBindings
    BYPRODUCTS ${CMAKE_CURRENT_SOURCE_DIR}/Sources/test_component/test_component.swift
    DEPENDS ${SWIFTWINRT_PARAM_FILE}
    DEPENDS ${WINMD_FILE}
    COMMAND ${CMAKE_BINARY_DIR}/swiftwinrt/swiftwinrt.exe @${SWIFTWINRT_PARAM_FILE}
    COMMENT "Running swiftwinrt...")
add_dependencies(GenerateBindings swiftwinrt)
add_dependencies(GenerateBindings test_component_cpp)
add_dependencies(GenerateBindings KillLSP)
