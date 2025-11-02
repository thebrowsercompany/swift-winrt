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

    enum class task
    {
        code_gen,
        proj_gen
    };

    struct settings_type
    {
        std::set<std::string> input;
        std::set<std::string> reference;

        std::filesystem::path output_folder;
        std::string support;
        std::string swift_version;
        bool license{};
        bool brackets{};
        bool verbose{};
        bool log{};
        std::filesystem::path component_folder;
        std::string component_name;
        bool component_prefix{};
        bool component_overwrite{};
        bool file_per_category{};
        std::string component_lib;
        bool component_opt{};
        bool component_ignore_velocity{};
        project_type project{ project_type::none };
        task task{ task::code_gen };
        std::set<std::string> include;
        std::set<std::string> exclude;

        winmd::reader::filter projection_filter;
        winmd::reader::filter component_filter;

        bool fastabi{};
        std::map<winmd::reader::TypeDef, winmd::reader::TypeDef> fastabi_cache;

        std::string get_c_module_name() const { return "CWinRT"; }
        bool has_project_type(project_type type) const { return (project & type) != project_type::none; }
    };

    extern settings_type settings;
}
