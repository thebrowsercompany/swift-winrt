#include "cmake_project_file_writer.h"
#include "utility/type_writers.h"
namespace swiftwinrt
{
    void write_cmake_project_file(std::string_view const& module, std::set<std::string_view> const& dependencies)
    {
        writer w;
        w.swift_module = module;
        w.c_mod = settings.get_c_module_name();
        auto path = w.project_directory() / "CMakeLists.txt";

        auto content = R"(
set(GENERATED_FILES_DIR ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB SWIFTWINRT_GENERATED_FILES CONFIGURE_DEPENDS *.swift)
add_library(% SHARED
  ${SWIFTWINRT_GENERATED_FILES}
  )

target_link_libraries(% PRIVATE
  CWinRT
  %)
)";
        w.write(content,
            module,
            module,
            bind_list("\n  ", dependencies));
        w.save_cmake();
    }
}