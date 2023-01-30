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

    // Gets all Swift support files for the generated code, indexed by filename.
    static std::map<std::string, std::string_view> get_swift_support_files()
    {
        typedef std::map<std::string, std::string_view> support_file_map;
        support_file_map support_files;
        EnumResourceNamesA(GetModuleHandle(NULL), "SWIFT_FILE",
            [](HMODULE hModule, LPCSTR lpType, LPSTR lpName, LONG_PTR lParam) -> BOOL
            {
                // We expect lpName to be a string pointer, but it could be an integer resource ID.
                if (IS_INTRESOURCE(lpName)) return TRUE;

                auto res_handle = FindResourceA(hModule, lpName, lpType);
                if (res_handle == 0) return TRUE;

                auto data_handle = LoadResource(hModule, res_handle);
                if (data_handle == 0) return TRUE;

                // Resources always have ALL_CAPS names
                std::string file_name{ lpName };
                std::transform(file_name.begin(), file_name.end(), file_name.begin(),
                    [](unsigned char c) { return std::tolower(c); });
                file_name += ".swift";

                // From Microsoft docs:
                //   The pointer returned by LockResource is valid until the module containing the resource is unloaded.
                //   It is not necessary to unlock resources because the system automatically deletes them when the process that created them terminates.
                auto text_ptr = static_cast<const char*>(LockResource(data_handle));
                auto text_size = SizeofResource(hModule, res_handle);

                ((support_file_map*)lParam)->emplace(std::move(file_name), std::string_view(text_ptr, text_size));
                return TRUE;
            },
            (LONG_PTR)&support_files);

        return support_files;
    }

    static void write_swift_support_files(std::string_view const& module_name)
    {
        auto support_files = get_swift_support_files();

        std::string dir_path = { settings.output_folder + "Source\\" };
        dir_path += module_name;
        dir_path += "\\Support\\";
        create_directories(dir_path);

        for (auto& support_file : support_files)
        {
            std::string path = dir_path + support_file.first;

            std::ofstream file;
            file.exceptions(std::ofstream::failbit | std::ofstream::badbit);
            try
            {
                file.open(path, std::ios::out | std::ios::binary);
                file.write(support_file.second.data(), support_file.second.size());
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
        w.c_mod = settings.test ? "C" + settings.support : "CWinRT";
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
        w.c_mod = settings.test ? "C" + settings.support : "CWinRT";
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
        w.c_mod = settings.test ? "C" + settings.support : "CWinRT";
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
