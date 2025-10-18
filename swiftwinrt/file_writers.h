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

    static void write_namespace_abi(std::string_view const& ns, type_cache const& members, include_only_used_filter const& filter)
    {
        writer w;
        w.filter = filter;
        w.type_namespace = ns;
        w.swift_module = get_swift_module(ns);
        w.support = settings.support;
        w.c_mod = settings.get_c_module_name();
        w.cache = members.cache;

        w.write("%", w.filter.bind_each<write_guid>(members.interfaces));
        w.write("%", w.filter.bind_each<write_guid>(members.delegates));

        {
            auto abi_ns = abi_namespace(ns);
            auto [namespace_guard, indent_guard] = push_internal_namespace(abi_ns, w);
            w.write("%", w.filter.bind_each<write_interface_abi>(members.interfaces));
            w.write("%", w.filter.bind_each<write_struct_abi>(members.structs));
            w.write("%", w.filter.bind_each<write_class_abi>(members.classes));
        }

        w.write("%", w.filter.bind_each<write_struct_init_extension>(members.structs));
        w.write("%", w.filter.bind_each<write_composable_impl_extension>(members.classes));
        w.write("%", w.filter.bind_each<write_delegate_abi>(members.delegates));

        w.swap();
        write_preamble(w, /* swift_code: */ true);


        w.save_file("ABI");
    }

    static void write_namespace_wrapper(std::string_view const& ns, type_cache const& members, include_only_used_filter const& filter)
    {
        writer w;
        w.filter = filter;
        w.support = settings.support;
        w.c_mod = settings.get_c_module_name();
        w.type_namespace = ns;
        w.swift_module = get_swift_module(ns);
        w.cache = members.cache;

        w.write("%", w.filter.bind_each<write_enum_def>(members.enums));
        w.write("%", w.filter.bind_each<write_class>(members.classes));
        w.write("%", w.filter.bind_each<write_delegate>(members.delegates));
        w.write("%", w.filter.bind_each<write_struct>(members.structs));
        w.write("%", w.filter.bind_each<write_interface_proto>(members.interfaces));

        w.write("%", w.filter.bind_each<write_enum_extension>(members.enums));

        w.swap();
        write_preamble(w, /* swift_code: */ true);

        w.save_file();

    }

    static void write_namespace_impl(std::string_view const& ns, type_cache const& members, include_only_used_filter const& filter)
    {
        writer w;
        w.filter = filter;
        w.support = settings.support;
        w.c_mod = settings.get_c_module_name();
        w.type_namespace = ns;
        w.swift_module = get_swift_module(ns);
        w.cache = members.cache;

        {
            auto impl_ns = impl_namespace(ns);
            auto impl_guard = push_internal_namespace(impl_ns, w);
            auto impl_names = w.push_impl_names(true);
            w.write("%", w.filter.bind_each<write_interface_impl>(members.interfaces));
            w.write("%", w.filter.bind_each<write_delegate_implementation>(members.delegates));
            w.write("%", w.filter.bind_each<write_class_bridge>(members.classes));
        }

        w.write("%", w.filter.bind_each<write_struct_bridgeable>(members.structs));

        // MakeFromAbi has to be in impl file (or main file) otherwise they get stripped away
        w.write("%", w.filter.bind_each<write_make_from_abi>(members.interfaces));
        w.write("%", w.filter.bind_each<write_make_from_abi>(members.classes));

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
