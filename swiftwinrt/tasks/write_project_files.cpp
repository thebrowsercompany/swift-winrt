#include "write_project_files.h"
#include "file_writers/spm_project_file_writer.h"
#include "file_writers/cmake_project_file_writer.h"
#include "file_writers/file_writers.h"
#include "utility/type_writers.h"
#include "utility/metadata_helpers.h"
#include "resources.h"

namespace swiftwinrt
{
    void write_spm_project_file(
        task_group& group,
        std::string const& swiftToolsVersion,
        std::string_view const& module,
        std::vector<std::string_view> const& namespaces,
        metadata_cache const& cache,
        include_only_used_filter const& mf)
    {
        group.add([swiftToolsVersion, module, namespaces, &cache, &mf]
        {
            auto dependencies = get_module_dependencies(module, namespaces, cache, mf);
            write_spm_project_file(swiftToolsVersion, module, dependencies);
        });

        if (module == settings.support)
        {
            group.add([]
            {
                auto dir_path = writer::root_directory() / "CWinRT";
                auto spm_template = find_resource(RESOURCE_TYPE_OTHER_FILE_STR, RESOURCE_NAME_CWINRT_PACKAGE_SWIFT_STR);
                fill_template_placeholders_to_file(spm_template, dir_path / "Package.swift");
            });
        }
    }

    void write_cmake_project_file(
        task_group& group,
        std::string_view const& module,
        std::vector<std::string_view> const& namespaces,
        metadata_cache const& cache,
        include_only_used_filter const& mf)
    {
        group.add([module, namespaces, &cache, &mf]
        {
            auto dependencies = get_module_dependencies(module, namespaces, cache, mf);
            write_cmake_project_file(module, dependencies);
        });
    }
}