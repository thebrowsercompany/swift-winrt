#pragma once

namespace swiftwinrt
{
    struct settings_type
    {
        std::set<std::string> input;
        std::set<std::string> reference;

        std::filesystem::path output_folder;
        std::string support;
        bool license{};
        bool brackets{};
        bool verbose{};
        bool log{};
        bool test{};
        std::filesystem::path component_folder;
        std::string component_name;
        bool component_prefix{};
        bool component_overwrite{};
        bool file_per_category{};
        std::string component_lib;
        bool component_opt{};
        bool component_ignore_velocity{};

        std::set<std::string> include;
        std::set<std::string> exclude;

        winmd::reader::filter projection_filter;
        winmd::reader::filter component_filter;

        bool fastabi{};
        std::map<winmd::reader::TypeDef, winmd::reader::TypeDef> fastabi_cache;

        std::string get_c_module_name() const { return test ? "C" + support : "CWinRT"; }
    };

    extern settings_type settings;
}
