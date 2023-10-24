#pragma once
#include "DerivedFromNoConstructor.g.h"
#include "UnsealedDerivedNoConstructor.h"

namespace winrt::test_component::implementation
{
    struct DerivedFromNoConstructor
    {
        DerivedFromNoConstructor() = default;
        void Method();
    };
}
