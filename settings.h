#pragma once

namespace swiftwinrt
{
    enum class ns_prefix
    {
        always,
        never,
        optional,
    };


    struct settings_type
    {
        std::set<std::string> input;
        std::set<std::string> reference;

        std::string output_folder;
        ns_prefix nsprefix = ns_prefix::always;
        bool license{};
        bool brackets{};
        bool verbose{};
        bool component{};
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
    };

    extern settings_type settings;
}
