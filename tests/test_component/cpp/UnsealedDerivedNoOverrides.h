#pragma once
#include "UnsealedDerivedNoOverrides.g.h"
#include "BaseNoOverrides.h"

namespace winrt::test_component::implementation
{
    struct UnsealedDerivedNoOverrides : UnsealedDerivedNoOverridesT<UnsealedDerivedNoOverrides, test_component::implementation::BaseNoOverrides>
    {
        UnsealedDerivedNoOverrides() = default;

    };
}
namespace winrt::test_component::factory_implementation
{
    struct UnsealedDerivedNoOverrides : UnsealedDerivedNoOverridesT<UnsealedDerivedNoOverrides, implementation::UnsealedDerivedNoOverrides>
    {
    };
}
