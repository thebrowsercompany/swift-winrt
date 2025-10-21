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
}
