#pragma once
#include "types.h"
#include "../type_writers.h"
namespace swiftwinrt
{
    void write_enum_def(writer& w, enum_type const& type);
    void write_enum_extension(writer& w, enum_type const& type);
}
