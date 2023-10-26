#pragma once
#include "UnsealedDerived2.g.h"
#include "UnsealedDerived.h"

namespace winrt::test_component::implementation
{
    struct UnsealedDerived2 : UnsealedDerived2T<UnsealedDerived2, test_component::implementation::UnsealedDerived>
    {
        UnsealedDerived2() = default;
        UnsealedDerived2(int32_t prop);

        void Method();
    };
}
namespace winrt::test_component::factory_implementation
{
    struct UnsealedDerived2 : UnsealedDerived2T<UnsealedDerived2, implementation::UnsealedDerived2>
    {
    };
}
