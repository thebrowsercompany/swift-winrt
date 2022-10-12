#include "pch.h"
#include "Simple.h"
#include "Simple.g.cpp"

namespace winrt::test_component::implementation
{
    winrt::event<Windows::Foundation::EventHandler<Windows::Foundation::IInspectable>> Simple::s_staticEvent = {};
    void Simple::Method()
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IReference<int32_t> Simple::Reference(Windows::Foundation::DateTime const&)
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IAsyncOperation<int32_t> Simple::Operation(Windows::Foundation::DateTime)
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IAsyncAction Simple::Action(Windows::Foundation::DateTime)
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IInspectable Simple::Object(Windows::Foundation::DateTime const&)
    {
        throw hresult_not_implemented();
    }

    winrt::event_token Simple::SignalEvent(test_component::Delegates::SignalDelegate const& handler)
    {
        return m_signalEvent.add(handler);
    }
    
    void Simple::SignalEvent(winrt::event_token const& token) noexcept
    {
        m_signalEvent.remove(token);
    }

    void Simple::FireEvent()
    {
        m_signalEvent();
        m_inEvent(L"You're Fired");
        auto args = test_component::SimpleEventArgs{};
        m_simpleEvent(*this, args);
    }

    winrt::event_token Simple::InEvent(test_component::Delegates::InDelegate const& handler)
    {
        return m_inEvent.add(handler);
    }

    void Simple::InEvent(winrt::event_token const& token) noexcept
    {
        m_inEvent.remove(token);
    }

    event_token Simple::StaticEvent(Windows::Foundation::EventHandler<IInspectable> const& handler)
    { 
        return s_staticEvent.add(handler);
    }
    void Simple::StaticEvent(event_token token)
    {
        s_staticEvent.remove(token);
    }

    void Simple::FireStaticEvent()
    {
        s_staticEvent(nullptr,nullptr);
    }

    winrt::event_token Simple::SimpleEvent(Windows::Foundation::TypedEventHandler<test_component::Simple, test_component::SimpleEventArgs> const& handler)
    {
        return m_simpleEvent.add(handler);  
    }

    void Simple::SimpleEvent(winrt::event_token const& token) noexcept
    {
        m_simpleEvent.remove(token);
    }
}
