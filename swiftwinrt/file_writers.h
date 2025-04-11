#pragma once

#include "resources.h"
#include "code_writers/struct_writers.h"
#include <regex>
#include <span>

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
            if (ns.starts_with("__"))
            {
                w.write("@_spi(WinRTInternal)\n");
            }
            w.write("%extension % {\n", i, ns);
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

    static void write_file(const std::filesystem::path& path, std::span<const std::byte> data)
    {
        std::ofstream file;
        file.exceptions(std::ofstream::failbit | std::ofstream::badbit);
        try
        {
            file.open(path, std::ios::out | std::ios::binary);
            file.write((const char*)data.data(), data.size());
        }
        catch (std::ofstream::failure const& e)
        {
            throw std::filesystem::filesystem_error(e.what(), path, std::io_errc::stream);
        }
    }

    static void fill_template_placeholders_to_file(std::span<const std::byte> data, const std::filesystem::path& path)
    {
        // Replace the module name placeholders with regex due to no string.replace(string, string)
        std::string text{ reinterpret_cast<const char*>(data.data()), data.size() };
        text = std::regex_replace(text, std::regex("C_BINDINGS_MODULE"), settings.get_c_module_name());
        text = std::regex_replace(text, std::regex("SUPPORT_MODULE"), settings.support);
        write_file(path, std::span(reinterpret_cast<const std::byte*>(text.data()), text.size()));
    }

    static void write_swift_support_files(std::string_view const& module_name)
    {
        auto dir_path = writer::root_directory() / module_name / "Support";
        create_directories(dir_path);

        auto support_files = get_named_resources_of_type(
            GetModuleHandle(NULL), RESOURCE_TYPE_SWIFT_SUPPORT_FILE_STR, /* make_lowercase: */ true);
        for (const auto& support_file : support_files)
        {
            auto path = dir_path / (support_file.first + ".swift");
            fill_template_placeholders_to_file(support_file.second, path);
        }
    }

    static void write_cwinrt_build_files()
    {
        auto dir_path = writer::root_directory() / "CWinRT";

        auto shim_template = find_resource(RESOURCE_TYPE_OTHER_FILE_STR, RESOURCE_NAME_CWINRT_SHIM_C_STR);
        fill_template_placeholders_to_file(shim_template, dir_path / "shim.c");
        auto guidworkaround_h_template = find_resource(RESOURCE_TYPE_OTHER_FILE_STR, RESOURCE_NAME_CWINRT_CPPINTEROPWORKAROUND_H_STR);
        fill_template_placeholders_to_file(guidworkaround_h_template, dir_path / "include" / "CppInteropWorkaround.h");
        auto ivectorchangedeventargs_h_template = find_resource(RESOURCE_TYPE_OTHER_FILE_STR, RESOURCE_NAME_CWINRT_IVECTORCHANGEDEVENTARGS_H_STR);
        fill_template_placeholders_to_file(ivectorchangedeventargs_h_template, dir_path / "include" / "IVectorChangedEventArgs.h");
        auto restrictederrorinfo_h_template = find_resource(RESOURCE_TYPE_OTHER_FILE_STR, RESOURCE_NAME_CWINRT_RESTRICTEDERRORINFO_H_STR);
        fill_template_placeholders_to_file(restrictederrorinfo_h_template, dir_path / "include" / "RestrictedErrorInfo.h");

        auto robuffer_h_template = find_resource(RESOURCE_TYPE_OTHER_FILE_STR, RESOURCE_NAME_CWINRT_ROBUFFER_H_STR);
        fill_template_placeholders_to_file(robuffer_h_template, dir_path / "include" / "robuffer.h");

        auto memorybuffer_h_template = find_resource(RESOURCE_TYPE_OTHER_FILE_STR, RESOURCE_NAME_CWINRT_MEMORYBUFFER_H_STR);
        fill_template_placeholders_to_file(memorybuffer_h_template, dir_path / "include" / "MemoryBuffer.h");
        auto weakreference_h_template = find_resource(RESOURCE_TYPE_OTHER_FILE_STR, RESOURCE_NAME_CWINRT_WEAKREFERENCE_H_STR);
        fill_template_placeholders_to_file(weakreference_h_template, dir_path / "include" / "WeakReference.h");

        if (settings.has_project_type(project_type::spm))
        {
            auto package_template = find_resource(RESOURCE_TYPE_OTHER_FILE_STR, RESOURCE_NAME_CWINRT_PACKAGE_SWIFT_STR);
            fill_template_placeholders_to_file(package_template, dir_path / "Package.swift");
        }

        auto support_files = get_named_resources_of_type(
            GetModuleHandle(nullptr), RESOURCE_TYPE_C_INCLUDE_FILE_STR, /* make_lowercase: */ true);
        for (const auto& support_file : support_files)
        {
            auto path = dir_path / "include" / (support_file.first + ".h");
            fill_template_placeholders_to_file(support_file.second, path);
        }
    }

    // All write_namespace_abi does is define the abi enum so that all other usages can be extensions to that type. This is
    // a temporary measure until we remove this naming scheme entirely.
    static void write_namespace_abi(std::string_view const& ns, type_cache const& members, include_only_used_filter const& filter)
    {
        writer w;
        w.filter = filter;
        w.type_namespace = ns;
        w.swift_module = get_swift_module(ns);
        w.support = settings.support;
        w.c_mod = settings.get_c_module_name();
        w.cache = members.cache;

        w.write("@_spi(WinRTInternal)\n");
        w.write("public enum % {}\n", abi_namespace(ns));
        w.swap();
        write_preamble(w, /* swift_code: */ true);

        w.save_file("ABI");
    }

    static void write_namespace_classes(std::string_view const& ns, type_cache const& members, include_only_used_filter const& filter)
    {
        for (auto&& member : members.classes)
        {
            writer w;
            w.filter = filter;
            w.support = settings.support;
            w.c_mod = settings.get_c_module_name();
            w.type_namespace = ns;
            w.swift_module = get_swift_module(ns);
            w.cache = members.cache;
            write_class(w, member);

            w.write("\n// MARK: - Internals\n\n");

            {
                auto impl_ns = impl_namespace(ns);
                auto impl_guard = push_namespace(impl_ns, w, true);
                auto impl_names = w.push_impl_names(true);
                write_class_bridge(w, member);
            }
            write_make_from_abi(w, member);

            {
                auto abi_ns = abi_namespace(ns);
                auto [namespace_guard, indent_guard] = push_namespace(abi_ns, w, true);
                write_class_abi(w, member);

                for (const auto& [interface_name, info] : member.get().required_interfaces)
                {
                    auto iface = dynamic_cast<interface_type const*>(info.type);
                    if (iface && is_exclusive(*iface) && !info.base)
                    {
                        write_guid(w, *iface);
                        write_interface_abi(w, *iface);
                    }
                }

                for (const auto& [interface_name, info] : member.get().factories)
                {
                    auto iface = dynamic_cast<interface_type const*>(info.type);
                    if (iface && is_exclusive(*iface))
                    {
                        write_guid(w, *iface);
                        write_interface_abi(w, *iface);
                    }
                }
            }

            write_composable_impl_extension(w, member);
            w.swap();
            write_preamble(w, /* swift_code: */ true);

            w.save_file(member.get().swift_type_name());
        }
    }

    static void write_namespace_interfaces(std::string_view const& ns, type_cache const& members, include_only_used_filter const& filter)
    {
        for (auto&& member : members.interfaces)
        {
            // Don't write exclusive interfaces here, those are handled by the class
            if (is_exclusive(member)) continue;

            writer w;
            w.filter = filter;
            w.support = settings.support;
            w.c_mod = settings.get_c_module_name();
            w.type_namespace = ns;
            w.swift_module = get_swift_module(ns);
            w.cache = members.cache;

            write_interface_proto(w, member);

            w.write("\n// MARK: - Internals\n\n");

            {
                auto impl_ns = impl_namespace(ns);
                auto impl_guard = push_namespace(impl_ns, w, true);
                auto impl_names = w.push_impl_names(true);
                write_interface_impl(w, member);
            }

            {
                auto abi_ns = abi_namespace(ns);
                auto [namespace_guard, indent_guard] = push_namespace(abi_ns, w, true);
                write_guid(w, member);
                write_interface_abi(w, member);
            }

            write_make_from_abi(w, member);

            w.swap();
            write_preamble(w, /* swift_code: */ true);

            w.save_file(remove_backtick(member.get().swift_type_name())); // remove backtick for generic interfaces
        }   
    }

    static void write_namespace_structs(std::string_view const& ns, type_cache const& members, include_only_used_filter const& filter)
    {
        for (auto&& member : members.structs)
        {
            writer w;
            w.filter = filter;
            w.support = settings.support;
            w.c_mod = settings.get_c_module_name();
            w.type_namespace = ns;
            w.swift_module = get_swift_module(ns);
            w.cache = members.cache;
            write_struct(w, member);

            w.write("\n// MARK: - Internals\n\n");

            write_struct_bridgeable(w, member);

            if (!is_struct_blittable(member))
            {
                auto abi_ns = abi_namespace(ns);
                auto [namespace_guard, indent_guard] = push_namespace(abi_ns, w, true);
                write_struct_abi(w, member);
            }

            write_struct_init_extension(w, member);

            w.swap();
            write_preamble(w, /* swift_code: */ true);

            w.save_file(member.get().swift_type_name());
        }
    }

    static void write_namespace_delegates(std::string_view const& ns, type_cache const& members, include_only_used_filter const& filter)
    {
        for (auto&& member : members.delegates)
        {
            writer w;
            w.filter = filter;
            w.support = settings.support;
            w.c_mod = settings.get_c_module_name();
            w.type_namespace = ns;
            w.swift_module = get_swift_module(ns);
            w.cache = members.cache;
            write_delegate(w, member);
            w.write("\n// MARK: - Internals\n\n");

            {
                auto impl_ns = impl_namespace(ns);
                auto impl_guard = push_namespace(impl_ns, w, true);
                auto impl_names = w.push_impl_names(true);
                write_delegate_implementation(w, member);
            }

            write_delegate_abi(w, member);

            w.swap();
            write_preamble(w, /* swift_code: */ true);

            w.save_file(remove_backtick(member.get().swift_type_name())); // remove backtick for generic delegates
        }   
    }

    static void write_namespace_enums(std::string_view const& ns, type_cache const& members, include_only_used_filter const& filter)
    {
        for (auto&& member : members.enums)
        {
            writer w;
            w.filter = filter;
            w.support = settings.support;
            w.c_mod = settings.get_c_module_name();
            w.type_namespace = ns;
            w.swift_module = get_swift_module(ns);
            w.cache = members.cache;
            write_enum_def(w, member);
            w.write("\n");
            write_enum_extension(w, member);
          
            w.swap();
            write_preamble(w, /* swift_code: */ true);

            w.save_file(member.get().swift_type_name());
        }
    }

    // All write_namespace_impl does is define the impl enum so that all other usages can be extensions to that type. This is
    // a temporary measure until we remove this naming scheme entirely.
    static void write_namespace_impl(std::string_view const& ns, type_cache const& members, include_only_used_filter const& filter)
    {
        writer w;
        w.filter = filter;
        w.support = settings.support;
        w.c_mod = settings.get_c_module_name();
        w.type_namespace = ns;
        w.swift_module = get_swift_module(ns);
        w.cache = members.cache;

        w.write("@_spi(WinRTInternal)\n");
        w.write("public enum % {}\n", impl_namespace(ns));
        w.swap();
        write_preamble(w, /* swift_code: */ true);

        w.save_file("Impl");
    }

    static void write_module_generics(std::string_view const& module, type_cache const& members, include_only_used_filter const& filter)
    {
        writer w;
        w.filter = filter;
        w.support = settings.support;
        w.c_mod = settings.get_c_module_name();
        w.type_namespace = module;
        w.swift_module = module;
        w.cache = members.cache;

        for (auto& [_, inst] : members.generic_instantiations)
        {
            write_guid_generic(w, inst.get());
            write_generic_extension(w, inst.get());
            write_interface_generic(w, inst.get());
            auto impl_names = w.push_impl_names(true);
            write_generic_implementation(w, inst.get());
        }

        for (auto& [_, eventType] : members.implementable_event_types)
        {
            if (is_generic_inst(eventType))
            {
                write_eventsource_invoke_extension(w, eventType);
            }
        }

        w.swap();
        write_preamble(w, /* swift_code: */ true);
        w.save_file("Generics");
    }
}
