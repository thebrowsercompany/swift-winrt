#include "pch.h"
#include "EventTester.h"
#include "EventTester.g.cpp"
#include <winrt/test_component.Delegates.h>

namespace winrt::test_component::implementation
{
    EventTester::EventTester(winrt::test_component::IIAmImplementable const& impl)
        : m_impl(impl)
    {
    }
    hstring EventTester::GetResult()
    {
        return m_result;
    }
    int32_t EventTester::Count()
    {
        return m_count;
    }

    void EventTester::Subscribe()
    {
        m_token = m_impl.ImplementableEvent([this](winrt::hstring const& result)
        {
            m_count++;
            m_result = result;
        });
    }
    void EventTester::Unsubscribe()
    {
        m_impl.ImplementableEvent(m_token);
    }
}
