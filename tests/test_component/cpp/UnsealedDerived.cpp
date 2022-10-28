#include "pch.h"
#include "UnsealedDerived.h"
#include "UnsealedDerived.g.cpp"

namespace winrt::test_component::implementation
{
    int32_t UnsealedDerived::Prop()
    {
        return m_prop;
    }
    void UnsealedDerived::Prop(int32_t value)
    {
        m_prop = value;
    }

    void UnsealedDerived::DoTheThing()
    {
        auto overrides = overridable();
        overrides.OnBeforeDoTheThing();
        overrides.OnDoTheThing();
        overrides.OnAfterDoTheThing();
    }

    void UnsealedDerived::OnBeforeDoTheThing()
    {
        printf("UnsealedDerived OnBeforeDoTheThing\n");
    }

    void UnsealedDerived::OnAfterDoTheThing()
    {
        printf("UnsealedDerived OnAfterDoTheThing\n");
    }
}
