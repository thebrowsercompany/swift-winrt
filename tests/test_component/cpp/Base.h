#pragma once
#include "Base.g.h"

namespace winrt::test_component::implementation
{
    struct Base : BaseT<Base>
    {
        Base() = default;

        virtual void DoTheThing();
        virtual void OnDoTheThing();
    };
}
namespace winrt::test_component::factory_implementation
{
    struct Base : BaseT<Base, implementation::Base>
    {
    };
}
