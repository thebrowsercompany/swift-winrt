#pragma once
#include "pch.h"
namespace swiftwinrt
{
    void write_cmake_project_file(std::string_view const& module, std::set<std::string_view> const& dependencies);
}