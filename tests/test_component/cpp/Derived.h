#pragma once
#include "Derived.g.h"
#include "Base.h"

namespace winrt::test_component::implementation
{
    struct Derived : DerivedT<Derived, test_component::implementation::Base>
    {
        Derived() = default;

        int32_t Prop();
        void Prop(int32_t value);
       
        void OnDoTheThing() override;
        private:
        int32_t m_prop{};
    };
}
namespace winrt::test_component::factory_implementation
{
    struct Derived : DerivedT<Derived, implementation::Derived>
    {
    };
}
