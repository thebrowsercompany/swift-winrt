project(swiftwinrt)

set(CMAKE_C_COMPILER cl)
set(CMAKE_CXX_COMPILER cl)

set(SWIFTWINRT_VERSION_STRING "0.0.1")
set(MicrosoftWindowsWinMD_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/winmd/src)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# change the warning level to 4
string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# change from dynamic to static CRT
string(REPLACE "/MDd" "/MTd" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
foreach(build_type RELEASE MINSIZEREL RELWITHDEBINFO)
    string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_${build_type} "${CMAKE_CXX_FLAGS_${build_type}}")
    string(APPEND CMAKE_CXX_FLAGS_${build_type} " /GL")

    # /GL requires /LTCG
    string(APPEND CMAKE_EXE_LINKER_FLAGS_${build_type} " /LTCG")

    # TODO: replacing /INCREMENTAL leaves ":NO" on the command line, which screws up the link
    #       Figure out the best way to make these changes to the build and linker flags
    # string(REPLACE "/INCREMENTAL" "" CMAKE_EXE_LINKER_FLAGS_${build_type} "${CMAKE_EXE_LINKER_FLAGS_${build_type}}")
    # string(APPEND CMAKE_EXE_LINKER_FLAGS_${build_type} " /LTCG:INCREMENTAL /OPT:REF")
endforeach()

# Always generate symbols for release builds
string(APPEND CMAKE_CXX_FLAGS_RELEASE " /Zi")
string(APPEND CMAKE_SHARED_LINKER_FLAGS_RELEASE " /DEBUG /OPT:REF /OPT:ICF /MAP")
string(APPEND CMAKE_EXE_LINKER_FLAGS_RELEASE " /DEBUG /OPT:REF /OPT:ICF /MAP")

if (CMAKE_CXX_COMPILER MATCHES "clang-cl")
    add_compile_options(-Wno-delete-non-virtual-dtor -mcx16 -fno-delayed-template-parsing)
else()
    add_compile_options(/permissive- /await)
endif()

# Explicitly configure _DEBUG preprocessor macro
string(APPEND CMAKE_CXX_FLAGS_DEBUG " /D_DEBUG")

add_definitions(-DNOMINMAX)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-Wno-missing-field-initializers)
endif()

add_executable(swiftwinrt "")
target_sources(swiftwinrt PUBLIC
    main.cpp
    pch.cpp
    metadata_cache.cpp
    types.cpp
    metadata_filter.cpp
    code_writers/type_writers.cpp
    code_writers/struct_writers.cpp
    resources.rc
)

# Make resources.rc depend on the files it embeds
file(GLOB_RECURSE SUPPORT_FILES CONFIGURE_DEPENDS Resources/*)
set_property(SOURCE resources.rc APPEND PROPERTY OBJECT_DEPENDS ${SUPPORT_FILES})

target_include_directories(swiftwinrt PUBLIC ${MicrosoftWindowsWinMD_INCLUDE_DIR} ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR})
target_compile_definitions(swiftwinrt PUBLIC "SWIFTWINRT_VERSION_STRING=\"${SWIFTWINRT_VERSION_STRING}\"")

target_precompile_headers(swiftwinrt PRIVATE pch.h)
target_link_libraries(swiftwinrt windowsapp ole32 shlwapi)

file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/swiftwinrt.exe" swiftwinrtwinrt_exe)
set_target_properties(swiftwinrt PROPERTIES "swiftwinrtwinrt_exe" ${swiftwinrtwinrt_exe})

install(TARGETS swiftwinrt DESTINATION bin COMPONENT exe)
