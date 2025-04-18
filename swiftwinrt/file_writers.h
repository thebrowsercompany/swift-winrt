#pragma once

#include "resources.h"
#include "code_writers.h"
#include <regex>
#include <span>

#include "utility/swift_codegen_utils.h"

namespace swiftwinrt
{
    using indent_writer = swiftwinrt::indented_writer_base<swiftwinrt::writer>;
    using push_namespace_guard = std::pair<write_scope_guard<swiftwinrt::writer>, indent_writer::indent_guard>;

    [[nodiscard]] static push_namespace_guard push_internal_namespace(std::string_view const& ns, writer& w)
    {
        write_scope_guard guard{ w };

        w.write("@_spi(WinRTInternal)\n");
        w.write("public enum % {\n", ns);
        guard.push("}\n");

        auto indent_guard = w.push_indent({1});
        return std::make_pair(std::move(guard), std::move(indent_guard));
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

    static void write_preamble_and_save(writer& w, typedef_base const& type)
    {
        // Empty writer may mean no types in the namespace or they were filtered out. 
        if (w.empty()) return;

        w.swap();
        write_preamble(w, /* swift_code: */ true);

        w.save_file(remove_backtick(type.swift_type_name()));
    }


    static void write_preamble_and_save(writer& w, std::string_view const& extension)
    {
        // Empty writer may mean no types in the namespace or they were filtered out. Either way,
        // don't write an empty file
        if (w.empty()) return;

        w.swap();
        write_preamble(w, /* swift_code: */ true);

        w.save_file(extension);
    }

    static void write_namespace_class(writer& w, std::string_view const& ns, class_type const& type)
    {
        write_class(w, type);

        w.write("// MARK: - % Internals\n\n", type.swift_type_name());

        if (type.default_interface)
        {
            auto impl_ns = impl_namespace(ns);
            auto impl_guard = push_namespace(impl_ns, w, true);
            auto impl_names = w.push_impl_names(true);
            write_class_bridge(w, type);
        }
        write_make_from_abi(w, type);

        {
            auto abi_ns = abi_namespace(ns);
            auto [namespace_guard, indent_guard] = push_namespace(abi_ns, w, true);
            write_class_abi(w, type);

            for (const auto& [interface_name, info] : type.required_interfaces)
            {
                auto iface = dynamic_cast<interface_type const*>(info.type);
                if (iface && is_exclusive(*iface) && !info.base)
                {
                    write_guid(w, *iface);
                    write_interface_abi(w, *iface);
                }
            }

            for (const auto& [interface_name, info] : type.factories)
            {
                auto iface = dynamic_cast<interface_type const*>(info.type);
                if (iface && is_exclusive(*iface))
                {
                    write_guid(w, *iface);
                    write_interface_abi(w, *iface);
                }
            }
        }

        write_composable_impl_extension(w, type);
    }

    static void write_namespace_classes(std::string_view const& ns, type_cache const& members, include_only_used_filter const& filter)
    {
        writer w;
        w.filter = filter;
        w.support = settings.support;
        w.c_mod = settings.get_c_module_name();
        w.type_namespace = ns;
        w.swift_module = get_swift_module(ns);
        w.cache = members.cache;

        for (auto&& member : members.classes)
        {
            if (!filter.includes(member.get().type())) continue;

            if (!settings.file_per_type)
            {
                w.write("// MARK: - %\n\n", member.get().swift_type_name());
            }
            write_namespace_class(w, ns, member.get());

            if (settings.file_per_type)
            {
                write_preamble_and_save(w, member.get());
            }
        }

        if (!settings.file_per_type)
        {
            write_preamble_and_save(w, "Classes");
        }
    }

    static void write_namespace_interface(writer& w, std::string_view const& ns, interface_type const& type)
    {
        write_interface_proto(w, type);

        w.write("// MARK: - % Internals\n\n", remove_backtick(type.swift_type_name()));

        {
            auto impl_ns = impl_namespace(ns);
            auto impl_guard = push_namespace(impl_ns, w, true);
            auto impl_names = w.push_impl_names(true);
            write_interface_impl(w, type);
        }

        {
            auto abi_ns = abi_namespace(ns);
            auto [namespace_guard, indent_guard] = push_namespace(abi_ns, w, true);
            write_guid(w, type);
            write_interface_abi(w, type);
        }

        write_make_from_abi(w, type);
    }

    static void write_namespace_interfaces(std::string_view const& ns, type_cache const& members, include_only_used_filter const& filter)
    {
        writer w;
        w.filter = filter;
        w.support = settings.support;
        w.c_mod = settings.get_c_module_name();
        w.type_namespace = ns;
        w.swift_module = get_swift_module(ns);
        w.cache = members.cache;

        for (auto&& member : members.interfaces)
        {
            // Don't write exclusive interfaces here, those are handled by the class
            if (!filter.includes(member.get().type()) || is_exclusive(member)) continue;

            if (!settings.file_per_type)
            {
                w.write("// MARK: - %\n\n", member.get().swift_type_name());
            }
            write_namespace_interface(w, ns, member.get());

            if (settings.file_per_type)
            {
                write_preamble_and_save(w, member.get());
            }
        }   

        if (!settings.file_per_type)
        {
            write_preamble_and_save(w, "Interfaces");
        }
    }

    static void write_namespace_struct(writer& w, std::string_view const& ns, struct_type const& type)
    {
        write_struct(w, type);

        w.write("// MARK: - % Internals\n\n", type.swift_type_name());

        write_struct_bridgeable(w, type);

        if (!is_struct_blittable(type))
        {
            auto abi_ns = abi_namespace(ns);
            auto [namespace_guard, indent_guard] = push_namespace(abi_ns, w, true);
            write_struct_abi(w, type);
        }

        write_struct_init_extension(w, type);
    }

    static void write_namespace_structs(std::string_view const& ns, type_cache const& members, include_only_used_filter const& filter)
    {
        writer w;
        w.filter = filter;
        w.support = settings.support;
        w.c_mod = settings.get_c_module_name();
        w.type_namespace = ns;
        w.swift_module = get_swift_module(ns);
        w.cache = members.cache; 
        for (auto&& member : members.structs)
        {
            if (!filter.includes(member.get().type())) continue;
        
            if (!settings.file_per_type)
            {
                w.write("// MARK: - %\n\n", member.get().swift_type_name());
            }
            write_namespace_struct(w, ns, member.get());
            if (settings.file_per_type)
            {
                write_preamble_and_save(w, member.get());
            }
        }
        if (!settings.file_per_type)
        {
            write_preamble_and_save(w, "Structs");
        }
    }

    static void write_namespace_delegate(writer& w, std::string_view const& ns, delegate_type const& type)
    {
        write_delegate(w, type);
        w.write("\n// MARK: - % Internals\n\n", type.swift_type_name());

        if (!type.is_generic())
        {
            auto impl_ns = impl_namespace(ns);
            auto impl_guard = push_namespace(impl_ns, w, true);
            auto impl_names = w.push_impl_names(true);
            write_delegate_implementation(w, type);
        }

        write_delegate_abi(w, type);
    }

    static void write_namespace_delegates(std::string_view const& ns, type_cache const& members, include_only_used_filter const& filter)
    {
        writer w;
        w.filter = filter;
        w.support = settings.support;
        w.c_mod = settings.get_c_module_name();
        w.type_namespace = ns;
        w.swift_module = get_swift_module(ns);
        w.cache = members.cache;

        for (auto&& member : members.delegates)
        {
            if (!filter.includes(member.get().type())) continue;
            if (!settings.file_per_type)
            {
                w.write("// MARK: - %\n\n", member.get().swift_type_name());
            }
            write_namespace_delegate(w, ns, member.get());

            if (settings.file_per_type)
            {
                write_preamble_and_save(w, member.get());
            }
        }   
        if (!settings.file_per_type)
        {
            write_preamble_and_save(w, "Delegates");
        }
    }

    static void write_namespace_enum(writer& w, std::string_view const& ns, enum_type const& type)
    {
        write_enum_def(w, type);
        w.write("\n");
        write_enum_extension(w, type);
    }

    static void write_namespace_enums(std::string_view const& ns, type_cache const& members, include_only_used_filter const& filter)
    {
        writer w;
        w.filter = filter;
        w.support = settings.support;
        w.c_mod = settings.get_c_module_name();
        w.type_namespace = ns;
        w.swift_module = get_swift_module(ns);
        w.cache = members.cache;

        for (auto&& member : members.enums)
        {
            if (!filter.includes(member.get().type())) continue;
            if (!settings.file_per_type)
            {
                w.write("// MARK: - %\n\n", member.get().swift_type_name());
            }
            write_namespace_enum(w, ns, member.get());

            if (settings.file_per_type)
            {
                write_preamble_and_save(w, member.get());
            }
        }

        if (!settings.file_per_type)
        {
            write_preamble_and_save(w, "Enums");
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
