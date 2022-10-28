#pragma once
#include "UnsealedDerived2.g.h"
#include "UnsealedDerived.h"

namespace winrt::test_component::implementation
{
    struct UnsealedDerived2 : UnsealedDerived2T<UnsealedDerived2, test_component::implementation::UnsealedDerived>
    {
        UnsealedDerived2() = default;

        void Method();
    };
}
namespace winrt::test_component::factory_implementation
{
    struct UnsealedDerived2 : UnsealedDerived2T<UnsealedDerived2, implementation::UnsealedDerived2>
    {
    };
}
