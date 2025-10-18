#pragma once

#include <string>
#include <utility>

#include "types/metadata_type.h"

namespace swiftwinrt
{
    struct interface_info
    {
        metadata_type const* type{};
        bool is_default{};
        bool defaulted{};
        bool overridable{};
        bool base{};
        bool exclusive{};
        bool fastabi{};
        bool attributed{};
        std::pair<uint32_t, uint32_t> relative_version{};
        generic_param_vector generic_params{};
    };

    using named_interface_info = std::pair<std::string, interface_info>;
}
