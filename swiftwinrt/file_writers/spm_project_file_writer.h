#pragma once
#include "pch.h"
namespace swiftwinrt
{
    void write_spm_project_file(
        std::string const& swiftToolsVersion,
        std::string_view const& module,
        std::set<std::string_view> const& dependencies);
}