project(WinRT_Test)

set(WINMD_FILE
    ${CMAKE_TEST_COMPONENT_OUTPUT}/test_component.winmd
)

execute_process(
    COMMAND swift --version
    OUTPUT_VARIABLE SWIFT_VERSION_OUTPUT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(REGEX MATCH "Swift version ([0-9]+\\.[0-9]+)" _ ${SWIFT_VERSION_OUTPUT})
set(SWIFT_TOOLS_VERSION ${CMAKE_MATCH_1})
message(STATUS "Detected Swift version: ${SWIFT_TOOLS_VERSION}")

set(SWIFT_WINRT_COMMON_PARAMETERS
"-include test_component"
"-include Windows.Foundation.Collections"
"-include Windows.Foundation.IMemoryBufferReference"
"-include Windows.Storage.Streams.Buffer"
"-include Windows.Storage.StorageFile"
"-include Windows.Storage.PathIO"
"-include Windows.Foundation.GuidHelper"
"-input ${WINMD_FILE}"
"-log"
"-output ${CMAKE_CURRENT_SOURCE_DIR}"
"-reference ${CMAKE_SYSTEM_VERSION}"
)
set(SWIFTWINRT_PARAM_FILE ${CMAKE_CURRENT_BINARY_DIR}/SwiftWinRT.rsp)

string(REPLACE ";" "\n" SWIFT_WINRT_PARAMETERS "${SWIFT_WINRT_COMMON_PARAMETERS}")

file(WRITE ${SWIFTWINRT_PARAM_FILE} ${SWIFT_WINRT_PARAMETERS})

add_subdirectory(cpp)

# re-run swiftwinrt to ensure the bindings get updated
# the language server needs to be killed before running swiftwinrt
# https://linear.app/the-browser-company/issue/WIN-161/swift-file-cannot-be-saved-locked-by-sourcekit-lsp
add_custom_target(KillLSP
    COMMAND (taskkill /im sourcekit-lsp.exe /f 2> nul || echo ...success) # need to || echo to ignore errors when the process isn't running
    COMMENT "Shutting down Language Server process"
    VERBATIM
)

# make sure build of swiftwinrt and test_component_cpp run before we kill the lsp.
# this way the LSP doesn't have time start up again before running the GenerateBindings
# target
add_dependencies(KillLSP swiftwinrt)
add_dependencies(KillLSP test_component_cpp)

add_custom_target(GenerateBindings
    BYPRODUCTS ${CMAKE_CURRENT_SOURCE_DIR}/Sources/test_component/test_component.swift
    DEPENDS ${SWIFTWINRT_PARAM_FILE}
    DEPENDS ${WINMD_FILE}
    COMMAND ${CMAKE_BINARY_DIR}/swiftwinrt/swiftwinrt.exe @${SWIFTWINRT_PARAM_FILE}
    COMMENT "Running swiftwinrt code generation...")
add_dependencies(GenerateBindings swiftwinrt)
add_dependencies(GenerateBindings test_component_cpp)
add_dependencies(GenerateBindings KillLSP)

# TODO: Fix concurrency warnings in swiftwinrt and remove this hack
set(SWIFT_TOOLS_VERSION "5.7")

set(SWIFT_WINRT_PROJECT_GENERATION_PARAMETERS
"${SWIFT_WINRT_COMMON_PARAMETERS}"
"-task GenerateProjectFiles"
"-spm"
"-swift-version ${SWIFT_TOOLS_VERSION}"
)
set(SWIFTWINRT_PROJECT_PARAM_FILE ${CMAKE_CURRENT_BINARY_DIR}/SwiftWinRTProjGen.rsp)
string(REPLACE ";" "\n" SWIFT_WINRT_PROJECT_GENERATION_PARAMETERS "${SWIFT_WINRT_PROJECT_GENERATION_PARAMETERS}")
file(WRITE ${SWIFTWINRT_PROJECT_PARAM_FILE} ${SWIFT_WINRT_PROJECT_GENERATION_PARAMETERS})

add_custom_target(GenerateProjectFiles
    BYPRODUCTS 
        ${CMAKE_CURRENT_SOURCE_DIR}/CWinRT/Package.swift
        ${CMAKE_CURRENT_SOURCE_DIR}/test_component/Package.swift
        ${CMAKE_CURRENT_SOURCE_DIR}/UWP/Package.swift
        ${CMAKE_CURRENT_SOURCE_DIR}/WindowsFoundation/Package.swift
    DEPENDS ${SWIFTWINRT_PROJECT_PARAM_FILE} swiftwinrt test_component_cpp KillLSP
    COMMAND ${CMAKE_BINARY_DIR}/swiftwinrt/swiftwinrt.exe @${SWIFTWINRT_PROJECT_PARAM_FILE}
    COMMENT "Generating project files with swiftwinrt...")