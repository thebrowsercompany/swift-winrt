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
"-output ${CMAKE_CURRENT_SOURCE_DIR}"
"-reference 10.0.20348.0"
)

string(REPLACE ";" "\n" SWIFT_WINRT_PARAMETERS "${SWIFT_WINRT_PARAMETERS}")

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/SwiftWinRT.rsp ${SWIFT_WINRT_PARAMETERS})

add_subdirectory(cpp)
add_subdirectory(Source)

# Since a lot of the bindings code isn't fully implemented, don't run the tool
# during build as this could modify the checked-in files as well as break the
# build.
#add_dependencies(WinRT_Test swiftwinrt)
#add_custom_command(
#  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/Generated/ArcCoreWinRT.swift
#  COMMAND ${CMAKE_BINARY_DIR}/bin/swiftwinrt.exe -ns-prefix -input ${ArcCore_INCLUDE_DIR}/bin/ArcCoreWin.winmd -output ${CMAKE_CURRENT_SOURCE_DIR}/Generated/ -import Ctest_component -reference 10.0.20348.0)