#pragma once
#include "Base.g.h"

namespace winrt::test_component::implementation
{
    struct Base : BaseT<Base>
    {
        Base() = default;
        ~Base();
        virtual void DoTheThing();
        virtual void OnDoTheThing();
        static winrt::test_component::Base CreateFromString(hstring const& value) { return winrt::make<Base>(); }
    };
}
namespace winrt::test_component::factory_implementation
{
    struct Base : BaseT<Base, implementation::Base>
    {
    };
}
