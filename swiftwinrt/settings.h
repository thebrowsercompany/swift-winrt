#pragma once

namespace swiftwinrt
{
    enum class project_type
    {
        none = 0x0,
        spm = 0x1,
        cmake = 0x10,
        both = spm & cmake
    };
    DEFINE_ENUM_FLAG_OPERATORS(project_type)

    struct settings_type
    {
        std::set<std::string> input;
        std::set<std::string> reference;

        std::string output_folder;
        std::string support;
        bool license{};
        bool brackets{};
        bool verbose{};
        bool log{};
        bool component{};
        bool test{};
        bool spm{};
        project_type project{ project_type::none };
        std::string component_folder;
        std::string component_name;
        bool component_prefix{};
        bool component_overwrite{};
        std::string component_lib;
        bool component_opt{};
        bool component_ignore_velocity{};

        std::set<std::string> include;
        std::set<std::string> exclude;

        winmd::reader::filter projection_filter;
        winmd::reader::filter component_filter;

        bool fastabi{};
        std::map<winmd::reader::TypeDef, winmd::reader::TypeDef> fastabi_cache;

        bool has_project_type(project_type type) const { return (project & type) != project_type::none; }

        std::string get_c_module_name() const { return test ? "C" + support : "CWinRT"; }
    };

    extern settings_type settings;
}
