project(test_component_cpp)

set(H_FILE
    ${CMAKE_TEST_COMPONENT_OUTPUT}/test_component.h
)

set(MIDL_FILE
    ${CMAKE_CURRENT_SOURCE_DIR}/test_component.idl
)

set(CPP_WINRT_OUTPUT_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/generated
)

# Fix the paths passed to midlrt
string(REPLACE "/" "\\" MIDL_FILE "${MIDL_FILE}")
string(REPLACE "/" "\\" WINMD_FILE "${WINMD_FILE}")
string(REPLACE "/" "\\" H_FILE "${H_FILE}")
string(REPLACE "/" "\\" CPP_WINRT_OUTPUT_DIR "${CPP_WINRT_OUTPUT_DIR}")

set(MIDL_OUTPUT
    ${H_FILE}
    ${WINMD_FILE}
)

set(CMAKE_CXX_STANDARD 17)
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
    add_compile_options(-Wno-delete-non-virtual-dtor -mcx16 -fno-delayed-template-parsing -Xclang -fcoroutines-ts)
else()
    add_compile_options(/permissive- /await)
endif()

# Explicitly configure _DEBUG preprocessor macro
string(APPEND CMAKE_CXX_FLAGS_DEBUG " /D_DEBUG")

add_definitions(-DNOMINMAX)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-Wno-missing-field-initializers)
endif()

set(CPPWINRT_OUTPUT
    ${CPP_WINRT_OUTPUT_DIR}/module.g.cpp
    ${CPP_WINRT_OUTPUT_DIR}/winrt/test_component.h
    ${CPP_WINRT_OUTPUT_DIR}/winrt/fast_forward.h
    ${CPP_WINRT_OUTPUT_DIR}/winrt/impl/test_component.0.h
    ${CPP_WINRT_OUTPUT_DIR}/winrt/impl/test_component.1.h
    ${CPP_WINRT_OUTPUT_DIR}/winrt/impl/test_component.2.h
)

add_custom_command(
  OUTPUT ${MIDL_OUTPUT} ${CPPWINRT_OUTPUT} 
  COMMAND midlrt /metadata_dir $ENV{SystemRoot}/system32/winmetadata /winrt /winmd ${WINMD_FILE} /h ${H_FILE} /ns_prefix /nomidl ${MIDL_FILE}
  COMMAND ${CMAKE_SOURCE_DIR}/tools/cppwinrt.exe -input ${WINMD_FILE} -comp ${CPP_WINRT_OUTPUT_DIR} -output ${CPP_WINRT_OUTPUT_DIR} -include test_component -verbose -opt -lib test -overwrite -reference 10.0.20348.0 -name test_component
  COMMAND ${CMAKE_SOURCE_DIR}/tools/cppwinrt.exe -output ${CPP_WINRT_OUTPUT_DIR} -include Windows.Foundation -opt -verbose -reference 10.0.20348.0
  DEPENDS ${MIDL_FILE}
  MAIN_DEPENDENCY ${MIDL_FILE}
  VERBATIM
)

add_library(test_component_cpp "")

target_include_directories(test_component_cpp PUBLIC ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR} ${CPP_WINRT_OUTPUT_DIR} ${CMAKE_TEST_COMPONENT_OUTPUT})
target_precompile_headers(test_component_cpp PRIVATE pch.h)

install(TARGETS test_component_cpp
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  COMPONENT lib)

target_sources(test_component_cpp PRIVATE
    StaticClass.cpp
    StaticClass.h
    Class.cpp
    Class.h
    module.cpp
    ${CPPWINRT_OUTPUT}
    #Optional.cpp
    #Optional.h
    pch.cpp
    pch.h
    Simple.cpp
    Simple.h
    exports.def
    #Windows.Class.cpp
    #Windows.Class.h
    ${MIDL_FILE}
    ${MIDL_OUTPUT})