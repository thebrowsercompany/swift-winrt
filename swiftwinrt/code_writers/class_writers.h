#pragma once
#include "types.h"
#include "utility/type_writers.h"
namespace swiftwinrt
{
    void write_class(writer& w, class_type const& type);
    void write_class_impl(writer& w, class_type const& type);
    void write_class_bridge(writer& w, class_type const& type);
    void write_class_abi(writer& w, class_type const& type);
    void write_composable_impl_extension(writer& w, class_type const& overridable);

    // Helpers
    void write_overrides_vtable(writer& w, class_type const& type, interface_info const& overrides, std::vector<named_interface_info> const& other_interfaces);
    void write_factory_constructors(writer& w, attributed_type const& factory, class_type const& type, metadata_type const& default_interface);
    void write_composable_impl(writer& w, class_type const& parent, metadata_type const& overrides, bool compose);
    void write_static_members(writer& w, attributed_type const& statics, class_type const& type);
    void write_statics_body(writer& w, function_def const& method, metadata_type const& statics);
    void write_default_constructor_declarations(writer& w, class_type const& type, metadata_type const& default_interface);
    void write_composable_constructor(writer& w, attributed_type const& factory, class_type const& type);
}
