#pragma once

#include <optional>

#include "types/function_def.h"
#include "types/metadata_type.h"

namespace swiftwinrt
{
    struct property_def
    {
        winmd::reader::Property def;
        metadata_type const* type{};
        std::optional<function_def> getter;
        std::optional<function_def> setter;

        bool is_array() const
        {
            return def.Type().Type().is_szarray() || def.Type().Type().is_array();
        }
    };
}
