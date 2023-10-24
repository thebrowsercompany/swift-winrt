#pragma once
#include "UnsealedDerived.g.h"
#include "Base.h"

namespace winrt::test_component::implementation
{
    struct UnsealedDerived : UnsealedDerivedT<UnsealedDerived, test_component::implementation::Base>
    {
        UnsealedDerived() = default;
        UnsealedDerived(int32_t prop);
        UnsealedDerived(hstring const& prop, winrt::test_component::Base const& base);

        int32_t Prop();
        void Prop(int32_t value);
        void OnBeforeDoTheThing();
        void OnAfterDoTheThing();
        void DoTheThing() override;

    private:
        int32_t m_prop{};
    };
}
namespace winrt::test_component::factory_implementation
{
    struct UnsealedDerived : UnsealedDerivedT<UnsealedDerived, implementation::UnsealedDerived>
    {
    };
}
