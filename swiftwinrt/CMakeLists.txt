project(swiftwinrt)

set(SWIFTWINRT_VERSION_STRING "0.0.1")
set(MicrosoftWindowsWinMD_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/winmd/src)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# change the warning level to 4
string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# change from dynamic to static CRT
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE INTERNAL "")

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
    utility/metadata_cache.cpp
    utility/metadata_filter.cpp
    utility/metadata_helpers.cpp
    utility/type_helpers.cpp
    utility/swift_codegen_utils.cpp
    types/class_type.cpp
    types/delegate_type.cpp
    types/element_type.cpp
    types/enum_type.cpp
    types/function_def.cpp
    types/generic_inst.cpp
    types/interface_type.cpp
    types/mapped_type.cpp
    types/struct_type.cpp
    types/system_type.cpp
    types/typedef_base.cpp
    code_writers/enum_writers.cpp
    code_writers/interface_writers.cpp
    code_writers/generic_writers.cpp
    code_writers/delegate_writers.cpp
    code_writers/class_writers.cpp
    code_writers/type_writers.cpp
    code_writers/struct_writers.cpp
    file_writers/spm_project_file_writer.cpp
    file_writers/cmake_project_file_writer.cpp
    tasks/write_project_files.cpp
    tasks/write_code_files.cpp
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
