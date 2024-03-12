#include "pch.h"
#include "Failure.h"
#include "Failure.g.cpp"

namespace winrt::test_component::implementation
{
    void Failure::FailedStaticMethod()
    {
        throw hresult_not_implemented();
    }
    bool Failure::FailedStaticProperty()
    {
        throw hresult_not_implemented();
    }
    hstring Failure::FailedProperty()
    {
        throw hresult_not_implemented();
    }
    void Failure::FailedProperty(hstring const& value)
    {
        throw hresult_not_implemented();
    }
}
