set(CMAKE_TEST_OUTPUT ${CMAKE_BINARY_DIR})
set(CMAKE_TEST_COMPONENT_OUTPUT ${CMAKE_BINARY_DIR}/generated/test_component)

include(ExternalProject)

# swift package resolving happens in the IDE, there's no need to run it as part of the build and add seconds to
# what should otherwise be very fast builds
set(SWIFT_BUILD_ARGS --disable-automatic-resolution --skip-update -Xswiftc -g)

# workaround for swiftc hang: https://linear.app/the-browser-company/issue/WIN-96/fix-swiftc-hanging-on-ci-machines
if ($ENV{NUMBER_OF_PROCESSORS} EQUAL 64)
  set(SWIFT_BUILD_ARGS ${SWIFT_BUILD_ARGS} -j 8)
else()
  set(SWIFT_BUILD_ARGS ${SWIFT_BUILD_ARGS} -j $ENV{NUMBER_OF_PROCESSORS})
endif()

# currently only use codeview since we can't actually build the c++ code with dwarf debug info
set(SWIFT_BUILD_ARGS ${SWIFT_BUILD_ARGS} -Xswiftc -debug-info-format=codeview -Xlinker -debug)

# SPM requires build config to be in lower case
string(TOLOWER ${CMAKE_BUILD_TYPE} SPM_BUILD_TYPE)
set(SWIFT_BUILD_ARGS ${SWIFT_BUILD_ARGS} -c ${SPM_BUILD_TYPE} --scratch-path ${CMAKE_CURRENT_BINARY_DIR})

set(SWIFT_COMMAND swift build ${SWIFT_BUILD_ARGS})
set(SPM_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/x86_64-unknown-windows-msvc/${SPM_BUILD_TYPE})

ExternalProject_Add(test_app
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
    INSTALL_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ${SWIFT_COMMAND}
    BUILD_IN_SOURCE true
    BUILD_ALWAYS ON
    BUILD_BYPRODUCTS ${SPM_BIN_DIR}/test_app.exe
    COMMENT "Running Swift build..."
)

ExternalProject_Get_Property(test_app BINARY_DIR)


install(FILES ${SPM_BIN_DIR}/test_app.exe DESTINATION bin)
install(FILES ${SPM_BIN_DIR}/test_component.dll DESTINATION bin)

add_subdirectory(test_component)
add_subdirectory(test_app)