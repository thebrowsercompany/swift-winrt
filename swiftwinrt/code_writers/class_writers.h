#pragma once
#include "types.h"
#include "../type_writers.h"
namespace swiftwinrt
{
    void write_class(writer& w, class_type const& type);
    void write_class_impl(writer& w, class_type const& type);
    void write_class_bridge(writer& w, class_type const& type);
    void write_class_abi(writer& w, class_type const& type);
    void write_composable_impl_extension(writer& w, class_type const& overridable);
}
