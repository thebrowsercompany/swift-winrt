#pragma once
#include "types.h"
#include "../type_writers.h"
namespace swiftwinrt
{
    void write_guid(writer& w, typedef_base const& type);
    void write_interface_proto(writer& w, interface_type const& type);
    void write_interface_abi(writer& w, interface_type const& type);
    void write_interface_impl(writer& w, interface_type const& type);
    void write_make_from_abi(writer& w, metadata_type const& type);
    void write_eventsource_invoke_extension(writer& w, metadata_type const* event_type);

    // Helpers
    void write_interface_bridge(writer& w, metadata_type const& type);
    void write_interface_abi_body(writer& w, typedef_base const& type, std::vector<function_def> const& methods);
    void write_vtable(writer& w, interface_type const& type);
    void write_implementable_interface(writer& w, interface_type const& type);
    void write_interface_impl_members(writer& w, interface_info const& info, typedef_base const& type_definition);

    void write_class_impl_func(writer& w, function_def const& method, interface_info const& iface, typedef_base const& type_definition);
    void write_class_impl_property(writer& w, property_def const& prop, interface_info const& iface, typedef_base const& type_definition);
    void write_class_impl_event(writer& w, event_def const& event, interface_info const& iface, typedef_base const& type_definition);
    void write_class_func_body(writer& w, function_def const& function, interface_info const& iface, bool is_noexcept);
}
