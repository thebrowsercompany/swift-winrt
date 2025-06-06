set(CMAKE_TEST_OUTPUT ${CMAKE_BINARY_DIR})
set(CMAKE_TEST_COMPONENT_OUTPUT ${CMAKE_BINARY_DIR}/generated/test_component)

if (NOT EXISTS ${CMAKE_TEST_COMPONENT_OUTPUT})
  file(MAKE_DIRECTORY ${CMAKE_TEST_COMPONENT_OUTPUT})
endif()

# workaround for swiftc hang: https://linear.app/the-browser-company/issue/WIN-96/fix-swiftc-hanging-on-ci-machines
if ($ENV{NUMBER_OF_PROCESSORS} EQUAL 64)
  set(SWIFT_BUILD_ARGS ${SWIFT_BUILD_ARGS} -j 8)
else()
  set(SWIFT_BUILD_ARGS ${SWIFT_BUILD_ARGS} -j $ENV{NUMBER_OF_PROCESSORS})
endif()

# workaround for https://linear.app/the-browser-company/issue/WIN-648/swift-compiler-crashes-with-imapstring-any
set(SWIFT_BUILD_ARGS ${SWIFT_BUILD_ARGS} -Xswiftc -disable-round-trip-debug-types)

# SPM requires build config to be in lower case
set(SWIFT_BUILD_ARGS ${SWIFT_BUILD_ARGS} -c $<LOWER_CASE:${CMAKE_BUILD_TYPE}>)

# Output to the cmake binary dir so build outputs are consolidated
set(SWIFT_BUILD_ARGS ${SWIFT_BUILD_ARGS} --scratch-path ${CMAKE_CURRENT_BINARY_DIR})

set(SWIFT_COMMAND swift build ${SWIFT_BUILD_ARGS})
if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "ARM64")
  set(SPM_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/aarch64-unknown-windows-msvc/${CMAKE_BUILD_TYPE})
else()
  set(SPM_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/x86_64-unknown-windows-msvc/${CMAKE_BUILD_TYPE})
endif()

add_custom_target(test_app ALL
  COMMAND ${SWIFT_COMMAND}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Running Swift build..."
  BYPRODUCTS ${SPM_BIN_DIR}/test_app.exe
  DEPENDS GenerateBindings
  VERBATIM
)

# install is used to copy the built binaries and dependencies to a known location
# where they can be run from and isn't used for distribution
install(FILES ${SPM_BIN_DIR}/test_app.exe DESTINATION bin)
install(FILES ${SPM_BIN_DIR}/test_component.dll DESTINATION bin)

add_subdirectory(test_component)
add_subdirectory(test_app)