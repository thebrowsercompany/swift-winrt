#pragma once
#include "pch.h"
#include "utility/task_group.h"
#include "utility/metadata_cache.h"
#include "utility/metadata_filter.h"

namespace swiftwinrt
{
    void write_spm_project_file(
        task_group& group,
        std::string const& swiftToolsVersion,
        std::string_view const& module,
        std::vector<std::string_view> const& namespaces,
        metadata_cache const& cache,
        include_only_used_filter const& mf);
    void write_cmake_project_file(
        task_group& group,
        std::string_view const& module,
        std::vector<std::string_view> const& namespaces,
        metadata_cache const& cache,
        include_only_used_filter const& mf);
}