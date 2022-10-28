#pragma once
#include "BaseNoOverrides.g.h"

namespace winrt::test_component::implementation
{
    struct BaseNoOverrides : BaseNoOverridesT<BaseNoOverrides>
    {
        BaseNoOverrides() = default;

    };
}
namespace winrt::test_component::factory_implementation
{
    struct BaseNoOverrides : BaseNoOverridesT<BaseNoOverrides, implementation::BaseNoOverrides>
    {
    };
}
