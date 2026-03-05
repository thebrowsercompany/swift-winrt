#include "pch.h"
#include "DerivedFromExternalBase.h"
#include "DerivedFromExternalBase.g.cpp"

namespace winrt::test_component::implementation
{
    int32_t DerivedFromExternalBase::Prop()
    {
        return m_prop;
    }
    void DerivedFromExternalBase::Prop(int32_t value)
    {
        m_prop = value;
    }
}
