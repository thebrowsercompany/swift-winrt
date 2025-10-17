#pragma once

#include "types/metadata_type.h"

namespace swiftwinrt
{
    struct event_def
    {
        winmd::reader::Event def;
        metadata_type const* type{};
    };
}
