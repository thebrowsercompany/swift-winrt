#pragma once
#include "pch.h"
#include "utility/task_group.h"
#include "utility/metadata_cache.h"
#include "utility/metadata_filter.h"

namespace swiftwinrt
{
    void write_swift_code_files(
        task_group& group,
        std::string_view const& module,
        std::vector<std::string_view> const& namespaces,
        metadata_cache const& mdCache,
        include_only_used_filter const& mf);

    void write_c_code_files(
        task_group& group,
        std::string_view const& module,
        std::vector<std::string_view> const& namespaces,
        metadata_cache const& mdCache,
        include_all_filter const& mf);
}