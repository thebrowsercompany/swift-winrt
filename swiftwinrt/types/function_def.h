#pragma once

#include <optional>
#include <vector>

#include "types/function_param.h"
#include "types/function_return_type.h"

namespace swiftwinrt
{
    struct function_def
    {
        winmd::reader::MethodDef def;
        std::optional<function_return_type> return_type;
        std::vector<function_param> params;

        bool is_async() const;
    };
}
