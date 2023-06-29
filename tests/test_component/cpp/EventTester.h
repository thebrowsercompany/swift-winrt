#pragma once
#include "EventTester.g.h"

namespace winrt::test_component::implementation
{
    struct EventTester : EventTesterT<EventTester>
    {
        EventTester() = default;

        EventTester(winrt::test_component::IIAmImplementable const& impl);
        hstring GetResult();
        int32_t Count();
        void Subscribe();
        void Unsubscribe();

        private:
        winrt::test_component::IIAmImplementable m_impl{ nullptr };
        winrt::hstring m_result;
        winrt::event_token m_token;
        int m_count = 0;
    };
}
namespace winrt::test_component::factory_implementation
{
    struct EventTester : EventTesterT<EventTester, implementation::EventTester>
    {
    };
}
