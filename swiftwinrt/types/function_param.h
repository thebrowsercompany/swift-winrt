#pragma once

#include <string_view>

#include "types/metadata_type.h"

namespace swiftwinrt
{
    struct function_param
    {
        winmd::reader::Param def;
        winmd::reader::ParamSig signature;
        std::string_view name;
        metadata_type const* type{};

        bool in() const { return def.Flags().In(); }
        bool out() const { return def.Flags().Out(); }
        bool is_array() const { return signature.Type().is_szarray() || signature.Type().is_array(); }
        bool by_ref() const { return signature.ByRef(); }
    };
}
