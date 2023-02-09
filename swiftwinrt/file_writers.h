#pragma once

#include "resources.h"

#include <regex>

namespace swiftwinrt
{
    using indent_writer = swiftwinrt::indented_writer_base<swiftwinrt::writer>;
    using push_namespace_guard = std::pair<write_scope_guard<swiftwinrt::writer>, indent_writer::indent_guard>;

    [[nodiscard]] static push_namespace_guard push_namespace(std::string_view const& ns, writer& w, bool force = false)
    {
        write_scope_guard guard{ w };

        std::string swift_ns;
        indent i{ 0 };
        auto first_ns_index = ns.find_first_of('.'); //

        // When writing the swift namespace, we want to omit the first one (which is the same as the module name).
        // This is only not true if forcing the namespace (i.e. ABI or IMPL namespaces)
        if (first_ns_index != std::string_view::npos)
        {
            auto extension = ns.substr(first_ns_index + 1);
            w.write("extension % {\n", extension);
            i.additional_indentation++;
            guard.push("%}\n", i);
        }
        else if (force)
        {
            w.write("%public enum % {\n", i, ns);
            guard.push("}\n");
            i.additional_indentation++;
        }

        auto indent_guard = w.push_indent(i);

        return std::make_pair(std::move(guard), std::move(indent_guard));
    }


    static bool push_namespace_recursive(std::string_view const& ns, writer& w, std::set<std::string_view>& namespaces)
    {
        std::string last_ns;
        auto last_ns_index = ns.find_last_of('.');
        // When writing the swift namespace, we want to omit the first one (which is the same as the module name).
        // This is only not true if forcing the namespace (i.e. ABI or IMPL namespaces)
        std::string extension;
        bool wrote = false;
        if (last_ns_index != std::string_view::npos)
        {
            auto first_ns_index = ns.find_first_of('.');
            last_ns = ns.substr(last_ns_index + 1);
            if (first_ns_index != last_ns_index)
            {
                extension = ns.substr(first_ns_index + 1, last_ns_index - first_ns_index - 1);

                // this extension namespace doesn't have any types. write the namespace
                auto parent_ns = ns.substr(0, last_ns_index);
                if (!parent_ns.empty() && namespaces.find(parent_ns) == namespaces.end())
                {
                    namespaces.emplace(parent_ns);
                    wrote = push_namespace_recursive(parent_ns, w, namespaces);
                }
            }
        }
        if (!last_ns.empty())
        {
            wrote = true;
            if (!extension.empty())
            {
                w.write("extension % { public enum % { } }\n", extension, last_ns);
            }
            else
            {
                w.write("public enum % { }\n", last_ns);
            }
        }

        return wrote;
    }

    static void write_swift_support_files(std::string_view const& module_name)
    {
        auto c_module_name = settings.get_c_module_name();

        auto dir_path = std::filesystem::path(settings.output_folder) / "Source" / module_name / "Support";
        create_directories(dir_path);

        auto support_files = get_named_resources_of_type(
            GetModuleHandle(NULL), RESOURCE_TYPE_SWIFT_FILE_STR, /* make_lowercase: */ true);
        for (const auto& support_file : support_files)
        {
            auto path = dir_path / (support_file.first + ".swift");

            // Replace the C bindings module name placeholder with regex due to no string.replace(string, string)
            std::string code{ reinterpret_cast<const char*>(support_file.second.data()), support_file.second.size() };
            code = std::regex_replace(code, std::regex("\\bC_BINDINGS_MODULE\\b"), c_module_name);
            code = std::regex_replace(code, std::regex("\\bSUPPORT_MODULE\\b"), settings.support);

            std::ofstream file;
            file.exceptions(std::ofstream::failbit | std::ofstream::badbit);
            try
            {
                file.open(path, std::ios::out | std::ios::binary);
                file.write(code.data(), code.size());
            }
            catch (std::ofstream::failure const& e)
            {
                throw std::filesystem::filesystem_error(e.what(), path, std::io_errc::stream);
            }
        }
    }

    static void write_namespace_abi(std::string_view const& ns, type_cache const& members, metadata_filter const& filter)
    {
        writer w;
        w.filter = filter;
        w.type_namespace = ns;
        w.support = settings.support;
        w.c_mod = settings.get_c_module_name();
        w.cache = members.cache;

        w.write("%", w.filter.bind_each<write_guid>(members.interfaces));
        w.write("%", w.filter.bind_each<write_guid>(members.delegates));

        for (auto& [_, inst] : members.generic_instantiations)
        {
            write_guid_generic(w, inst.get());
        }

        {
            auto abi_ns = abi_namespace(ns);
            auto [namespace_guard, indent_guard] = push_namespace(abi_ns, w, true);

            for (auto& [_, inst] : members.generic_instantiations)
            {
                write_interface_generic(w, inst.get());
            }
            w.write("%", w.filter.bind_each<write_interface_abi>(members.interfaces));
            w.write("%", w.filter.bind_each<write_delegate_abi>(members.delegates));
            w.write("%", w.filter.bind_each<write_struct_abi>(members.structs));
            w.write("%", w.filter.bind_each<write_class_abi>(members.classes));
        }

        // we have to write these in both files because they are fileprivate and required for initializing generics
        // at the ABI layer for when we have swift implemented interfaces
        for (auto& [_, inst] : members.generic_instantiations)
        {
            write_ireference_init_extension(w, inst.get());
        }

        w.write("%", w.filter.bind_each<write_struct_init_extension>(members.structs));
        w.write("%", w.filter.bind_each<write_composable_impl_extension>(members.classes));

        w.swap();
        write_preamble(w);


        w.save_file("ABI");
    }

    static void write_namespace_wrapper(std::string_view const& ns, type_cache const& members, metadata_filter const& filter)
    {
        writer w;
        w.filter = filter;
        w.support = settings.support;
        w.c_mod = settings.get_c_module_name();
        w.type_namespace = ns;
        w.cache = members.cache;

        w.write("%", w.filter.bind_each<write_enum_def>(members.enums));
        w.write("%", w.filter.bind_each<write_class>(members.classes));
        w.write("%", w.filter.bind_each<write_delegate>(members.delegates));
        w.write("%", w.filter.bind_each<write_struct>(members.structs));
        w.write("%", w.filter.bind_each<write_interface_proto>(members.interfaces));

        if (ns == "Windows.Foundation")
        {
            write_ireference(w);
        }

        w.write("%", w.filter.bind_each<write_enum_extension>(members.enums));
        for (auto& [_, inst] : members.generic_instantiations)
        {
            write_ireference_init_extension(w, inst.get());
        }

        w.swap();
        write_preamble(w);

        w.save_file();

    }

    static void write_namespace_impl(std::string_view const& ns, type_cache const& members, metadata_filter const& filter)
    {
        writer w;
        w.filter = filter;
        w.support = settings.support;
        w.c_mod = settings.get_c_module_name();
        w.type_namespace = ns;
        w.cache = members.cache;

        {
            auto impl_ns = impl_namespace(ns);
            auto impl_guard = push_namespace(impl_ns, w, true);
            auto impl_names = w.push_impl_names(true);
            w.write("%", w.filter.bind_each<write_interface_impl>(members.interfaces));
            w.write("%", w.filter.bind_each<write_delegate_implementation>(members.delegates));
            for (auto& [_, inst] : members.generic_instantiations)
            {
                write_generic_implementation(w, inst.get());
            }
        }

        for (auto& [_, inst] : members.generic_instantiations)
        {
            write_ireference_init_extension(w, inst.get());
        }


        w.swap();
        write_preamble(w);

        w.save_file("Impl");
    }
}
