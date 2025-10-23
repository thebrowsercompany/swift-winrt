#pragma once

#include "types/metadata_type.h"

namespace swiftwinrt
{
    struct struct_member
    {
        winmd::reader::Field field;
        metadata_type const* type;
    };
}
