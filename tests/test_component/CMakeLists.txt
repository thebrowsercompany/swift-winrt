project(WinRT_Test)

add_subdirectory(cpp)

# Run swiftwinrt to update the generated files
set(SWIFTWINRT_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SWIFTWINRT_OUTPUT_LOG ${SWIFTWINRT_OUTPUT_DIR}/swiftwinrt.log)
set(SWIFTWINRT_RSP ${CMAKE_CURRENT_BINARY_DIR}/SwiftWinRT.rsp)
set(SWIFTWINRT_PARAMETERS
    "-ns-prefix"
    "-include test_component"
    "-input ${TEST_COMPONENT_WINMD_FILE}"
    "-support test_component"
    "-test"
    "-cmake"
    "-output ${SWIFTWINRT_OUTPUT_DIR}"
    "-log"
    "-reference ${CMAKE_SYSTEM_VERSION}")
string(REPLACE ";" "\n" SWIFTWINRT_PARAMETERS "${SWIFTWINRT_PARAMETERS}")
file(WRITE ${SWIFTWINRT_RSP}.tmp ${SWIFTWINRT_PARAMETERS})
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SWIFTWINRT_RSP}.tmp ${SWIFTWINRT_RSP})

add_custom_command(OUTPUT ${SWIFTWINRT_OUTPUT_LOG}
    COMMAND swiftwinrt @${SWIFTWINRT_RSP}
    DEPENDS ${SWIFTWINRT_RSP}
    DEPENDS ${TEST_COMPONENT_WINMD_FILE}
    COMMENT "Running swiftwinrt for test_component...")

add_custom_target(test_component_swiftwinrt DEPENDS ${SWIFTWINRT_OUTPUT_LOG})
add_dependencies(test_component_swiftwinrt test_component_cpp)

add_subdirectory(Source)

add_dependencies(test_component test_component_swiftwinrt)
add_dependencies(Ctest_component test_component_swiftwinrt)