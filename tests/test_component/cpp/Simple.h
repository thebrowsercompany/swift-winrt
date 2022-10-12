#pragma once
#include "Simple.g.h"
#include "winrt/test_component.Delegates.h"

namespace winrt::test_component::implementation
{
    struct Simple : SimpleT<Simple>
    {
        Simple() = default;

        void Method();
        Windows::Foundation::IReference<int32_t> Reference(Windows::Foundation::DateTime const& value);
        Windows::Foundation::IAsyncOperation<int32_t> Operation(Windows::Foundation::DateTime value);
        Windows::Foundation::IAsyncAction Action(Windows::Foundation::DateTime value);
        Windows::Foundation::IInspectable Object(Windows::Foundation::DateTime const& value);

        // All we care about static events (for now) is that they build.
        static event_token StaticEvent(Windows::Foundation::EventHandler<IInspectable> const&);
        static void StaticEvent(event_token);
        static void FireStaticEvent();
        test_component::BlittableStruct ReturnBlittableStruct()
        {
            return { 123, 456 };
        }

        void TakeBlittableStruct(test_component::BlittableStruct const& value)
        {
            if (value.First == 654 && value.Second == 321)
            {
                printf("Accepted!\n");
            }
            else
            {
                assert(false);
            }
        }

        test_component::BlittableStruct BlittableStructProperty()
        {
            return m_blittableStruct;
        }

        void BlittableStructProperty(test_component::BlittableStruct const& value)
        {
            m_blittableStruct = value;
        }

        test_component::NonBlittableStruct ReturnNonBlittableStruct()
        {
            return { hstring(L"Hello"), hstring(L"World")};
        }

        void TakeNonBlittableStruct(test_component::NonBlittableStruct const& value)
        {
            if (value.First == L"From" && value.Second == L"Swift!")
            {
                printf("Accepted!\n");
            }
            else
            {
                assert(false);
            }
        }

        test_component::NonBlittableStruct NonBlittableStructProperty()
        {
            return m_nonBlittableStruct;
        }
        void NonBlittableStructProperty(test_component::NonBlittableStruct const& value)
        {
            m_nonBlittableStruct = value;
        }

        hstring StringProperty()
        {
            return m_stringProp;
        }

        void StringProperty(hstring const& value)
        {
            m_stringProp = value;
        }

        winrt::event_token SignalEvent(test_component::Delegates::SignalDelegate const& handler);
        void SignalEvent(winrt::event_token const& token) noexcept;
        void FireEvent();
        winrt::event_token InEvent(test_component::Delegates::InDelegate const& handler);
        void InEvent(winrt::event_token const& token) noexcept;

        winrt::event_token SimpleEvent(Windows::Foundation::TypedEventHandler<test_component::Simple, test_component::SimpleEventArgs> const& handler);
        void SimpleEvent(winrt::event_token const& token) noexcept;
        private:
        hstring m_stringProp{};
        test_component::BlittableStruct m_blittableStruct{};
        test_component::NonBlittableStruct m_nonBlittableStruct{};
        winrt::event<test_component::Delegates::SignalDelegate> m_signalEvent;
        winrt::event<test_component::Delegates::InDelegate> m_inEvent;
        winrt::event<Windows::Foundation::TypedEventHandler<test_component::Simple, test_component::SimpleEventArgs>> m_simpleEvent;


        static winrt::event<Windows::Foundation::EventHandler<Windows::Foundation::IInspectable>> s_staticEvent;
    };
}
namespace winrt::test_component::factory_implementation
{
    struct Simple : SimpleT<Simple, implementation::Simple>
    {
    };
}
