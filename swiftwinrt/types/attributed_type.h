#pragma once

#include "types/metadata_type.h"

namespace swiftwinrt
{
    struct attributed_type
    {
        metadata_type const* type{};
        bool activatable{};
        bool statics{};
        bool composable{};
        bool visible{};
        bool default_composable{};
    };
}
