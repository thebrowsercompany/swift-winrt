#pragma once

namespace swiftwinrt
{
    using indent_writer = swiftwinrt::indented_writer_base<swiftwinrt::writer>;
    using push_namespace_guard = std::pair<write_scope_guard<swiftwinrt::writer>, indent_writer::indent_guard>;

    [[nodiscard]] static push_namespace_guard push_namespace(std::string_view const& ns, writer& w, bool force = false)
    {
        write_scope_guard guard{ w };

        std::string swift_ns;
        indent i{ 0 };
        auto last_ns_index = ns.find_last_of('.');
        auto first_ns_index = ns.find_first_of('.');

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

    static void write_namespace_definitions(std::map<std::string, std::vector<std::string_view>> & namespaces, settings_type const& settings)
    {
        bool has_namespace_definitions = false;
        for (auto&& [module, ns_list] : namespaces)
        {
            std::set<std::string_view> written_namespace;

            writer w;
            write_preamble(w);

            written_namespace.insert(ns_list.begin(), ns_list.end());
            std::sort(ns_list.begin(), ns_list.end());

            for (auto&& ns : ns_list)
            {
                if (push_namespace_recursive(ns, w, written_namespace))
                {
                    has_namespace_definitions = true;
                }
            }

            if (has_namespace_definitions)
            {
                auto filename{ settings.output_folder + module + "/swift/NamespaceDefinitions.swift" };
                w.flush_to_file(filename);
            }
        }
    }

    static void write_namespace_abi(std::string_view const& ns, cache::namespace_members const& members, settings_type const& settings)
    {
        writer w;
        w.filter = settings.projection_filter;
        w.type_namespace = ns;
        w.support = settings.support;

        {
            w.write("%", w.filter.bind_each<write_guid>(members.interfaces));

            auto abi_ns = abi_namespace(ns);
            auto [namespace_guard, indent_guard] = push_namespace(abi_ns, w, true);

            w.write("%", w.filter.bind_each<write_interface_abi>(members.interfaces));
            w.write("%", w.filter.bind_each<write_delegate_abi>(members.delegates));
            w.write("%", w.filter.bind_each<write_struct_abi>(members.structs));
        }
        w.swap();
        write_preamble(w);


        w.save_file("ABI");
    }

    static void write_namespace_wrapper(std::string_view const& ns, cache::namespace_members const& members, settings_type const& settings)
    {
        writer w;
        w.filter = settings.projection_filter;
        w.support = settings.support;
        w.type_namespace = ns;

        {
            auto namespace_guard = push_namespace(ns, w);

            w.write("%", w.filter.bind_each<write_enum_def>(members.enums));
            w.write("%", w.filter.bind_each<write_class>(members.classes));
            w.write("%", w.filter.bind_each<write_delegate>(members.delegates));
            w.write("%", w.filter.bind_each<write_struct>(members.structs));
            w.write("%", w.filter.bind_each<write_interface_alias>(members.interfaces));

            {
                auto impl_guard = push_namespace("Impl", w, true);
                auto impl_indent_guard = w.push_indent({ 1 });
                w.write("%", w.filter.bind_each<write_interface_impl>(members.interfaces));
            }
        }
        w.write("%", w.filter.bind_each<write_interface_proto>(members.interfaces));

        w.write("%", w.filter.bind_each<write_enum_extension>(members.enums));

        w.swap();
        write_preamble(w);

        w.save_file();
    }
}
