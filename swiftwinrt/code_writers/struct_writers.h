#pragma once
#include "types.h"
#include "../type_writers.h"
namespace swiftwinrt
{
    void write_struct_initializer_params(writer& w, struct_type const& type);
    void write_struct_init_extension(writer& w, struct_type const& type);
    void write_struct(writer& w, struct_type const& type);
    void write_struct_bridgeable(writer& w, struct_type const& type);
}