#pragma once

#include <string_view>

#include "types/metadata_type.h"

namespace swiftwinrt
{
    struct function_return_type
    {
        winmd::reader::RetTypeSig signature;
        std::string_view name;
        metadata_type const* type{};

        bool in() const { return false; }
        bool out() const { return true; }
        bool is_array() const { return signature.Type().is_szarray() || signature.Type().is_array(); }
        bool by_ref() const { return false; }
    };
}
