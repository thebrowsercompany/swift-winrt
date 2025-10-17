#pragma once

#include "types/metadata_type.h"

namespace swiftwinrt
{
    template <typename T>
    inline auto begin_type_definition(writer& w, T const& type)
    {
        if (type.is_experimental())
        {
            w.write("#if defined(ENABLE_WINRT_EXPERIMENTAL_TYPES)\n");
        }
    }

    template <typename T>
    inline void end_type_definition(writer& w, T const& type)
    {
        if (type.is_experimental())
        {
            w.write("#endif // defined(ENABLE_WINRT_EXPERIMENTAL_TYPES)\n");
        }

        w.write('\n');
    }
}
