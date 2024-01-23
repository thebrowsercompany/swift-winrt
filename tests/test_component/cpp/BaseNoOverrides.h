#pragma once
#include "BaseNoOverrides.g.h"

namespace winrt::test_component::implementation
{
    struct BaseNoOverrides : BaseNoOverridesT<BaseNoOverrides>
    {
        BaseNoOverrides() = default;
        static winrt::test_component::BaseNoOverrides CreateFromString(hstring const& value) { return winrt::make<BaseNoOverrides>(); }
    };
}
namespace winrt::test_component::factory_implementation
{
    struct BaseNoOverrides : BaseNoOverridesT<BaseNoOverrides, implementation::BaseNoOverrides>
    {
    };
}
