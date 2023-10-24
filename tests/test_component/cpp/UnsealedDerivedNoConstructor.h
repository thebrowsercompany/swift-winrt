#pragma once
#include "UnsealedDerivedNoConstructor.g.h"
#include "Base.h"

namespace winrt::test_component::implementation
{
    struct UnsealedDerivedNoConstructor : UnsealedDerivedNoConstructorT<UnsealedDerivedNoConstructor, test_component::implementation::Base>
    {
        UnsealedDerivedNoConstructor() = default;

    };
}
