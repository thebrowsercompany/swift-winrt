#pragma once
#include "UnsealedDerivedFromNoConstructor.g.h"
#include "UnsealedDerivedNoConstructor.h"

namespace winrt::test_component::implementation
{
    struct UnsealedDerivedFromNoConstructor : UnsealedDerivedFromNoConstructorT<UnsealedDerivedFromNoConstructor, test_component::implementation::UnsealedDerivedNoConstructor>
    {
        UnsealedDerivedFromNoConstructor() = default;

    };
}
namespace winrt::test_component::factory_implementation
{
    struct UnsealedDerivedFromNoConstructor : UnsealedDerivedFromNoConstructorT<UnsealedDerivedFromNoConstructor, implementation::UnsealedDerivedFromNoConstructor>
    {
    };
}
