#pragma once
#include "DerivedFromExternalBase.g.h"
#include "ExternalBase.h"

namespace winrt::test_component::implementation
{
    struct DerivedFromExternalBase : DerivedFromExternalBaseT<DerivedFromExternalBase, external_base::implementation::ExternalBase>
    {
        DerivedFromExternalBase() = default;

        int32_t Prop();
        void Prop(int32_t value);

    private:
        int32_t m_prop{};
    };
}
namespace winrt::test_component::factory_implementation
{
    struct DerivedFromExternalBase : DerivedFromExternalBaseT<DerivedFromExternalBase, implementation::DerivedFromExternalBase>
    {
    };
}
