#include "pch.h"
#include "UnsealedDerived2.h"
#include "UnsealedDerived2.g.cpp"

namespace winrt::test_component::implementation
{
    UnsealedDerived2::UnsealedDerived2(int32_t prop)
    {
        Prop(prop*3);
    }
    void UnsealedDerived2::Method()
    {
    }
}
