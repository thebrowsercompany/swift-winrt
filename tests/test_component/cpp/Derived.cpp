#include "pch.h"
#include "Derived.h"
#include "Derived.g.cpp"

namespace winrt::test_component::implementation
{
    int32_t Derived::Prop()
    {
        return m_prop;
    }
    void Derived::Prop(int32_t value)
    {
        m_prop = value;
    }

    void Derived::OnDoTheThing()
    {
        printf("Derived OnDoTheThing\n");
    }
}
