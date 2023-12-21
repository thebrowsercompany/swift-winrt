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

        test_component::StructWithIReference ReturnStructWithReference()
        {
            winrt::Windows::Foundation::IReference<int32_t> value1 = 4;
            winrt::Windows::Foundation::IReference<int32_t> value2 = 2;

            return { value1, value2};
        }

        void TakeStructWithReference(test_component::StructWithIReference const& value)
        {
            if (winrt::unbox_value<int32_t>(value.Value1) != 4 && winrt::unbox_value<int32_t>(value.Value2) != 2)
            {
                throw hresult_not_implemented();
            }
        }

        test_component::StructWithIReference StructWithReferenceProperty()
        {
            return m_structWithIReferenceStruct;
        }
        void StructWithReferenceProperty(test_component::StructWithIReference const& value)
        {
            m_structWithIReferenceStruct = value;
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
        void CantActuallyOverrideBecauseNotComposable(){}
        private:
        hstring m_stringProp{};
        test_component::BlittableStruct m_blittableStruct{};
        test_component::NonBlittableStruct m_nonBlittableStruct{};
        test_component::StructWithIReference m_structWithIReferenceStruct{};
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
