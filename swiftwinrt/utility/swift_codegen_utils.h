#pragma once

#include <string>
#include <string_view>
#include <set>

#include "winmd_reader.h"
#include "types/metadata_type.h"
#include "utility/type_helpers.h"

namespace swiftwinrt
{
    struct writer;
    struct interface_info;
    struct function_param;
    struct function_return_type;
    struct property_def;
    struct function_def;
    struct struct_member;

    void write_preamble(writer& w, bool swift_code);

    std::string_view get_swift_module(std::string_view ns);
    std::string_view get_swift_module(winmd::reader::TypeDef const& type);

    std::string get_full_swift_type_name(writer const& w, winmd::reader::TypeDef const& type);
    std::string get_full_swift_type_name(writer const& w, metadata_type const* type);
    std::string get_full_swift_type_name(writer const& w, metadata_type const& type);

    // Swift naming methods
    std::string get_swift_name(interface_info const& iface);
    std::string get_swift_name(winmd::reader::MethodDef const& method);
    std::string get_swift_member_name(std::string_view name);
    std::string get_swift_name(winmd::reader::Property const& property);
    std::string get_swift_name(winmd::reader::Event const& event);
    std::string get_swift_name(winmd::reader::Field const& field);
    std::string get_swift_name(winmd::reader::Param const& param);
    std::string local_swift_param_name(std::string const& param_name);
    std::string local_swift_param_name(std::string_view param_name);
    std::string local_swift_param_name(function_param const& param);
    std::string get_swift_name(function_param const& param);
    std::string get_swift_name(function_return_type const& return_type);
    std::string get_swift_name(property_def const& property);
    std::string get_swift_name(function_def const& function);
    std::string get_swift_name(struct_member const& member);

    // ABI naming methods
    std::string_view get_abi_name(winmd::reader::MethodDef const& method);
    std::string_view get_abi_name(function_def const& method);
    std::string_view get_abi_name(struct_member const& member);

    // String transformation methods
    std::string to_camel_case(std::string_view name);

    // Backtick handling methods
    std::string put_in_backticks_if_needed(std::string name);
    std::string_view remove_backtick(std::string_view name);

    // Wrapper-related methods
    bool needs_wrapper(param_category category);

    // ABI/impl namespace methods
    std::string internal_namepace(std::string prefix, std::string_view ns);
    std::string abi_namespace(std::string_view ns);
    std::string impl_namespace(std::string_view ns);
    std::string abi_namespace(winmd::reader::TypeDef const& type);
    std::string abi_namespace(metadata_type const& type);
    std::string abi_namespace(metadata_type const* type);
}
