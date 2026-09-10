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

        uint32_t StoredStringReferences()
        {
            // Keep m_stringProp alive while inspecting the pinned C++/WinRT HSTRING layout.
            auto header = static_cast<impl::shared_hstring_header*>(get_abi(m_stringProp));
            return header ? static_cast<uint32_t>(header->count) : 0;
        }

        Windows::Foundation::IReference<test_component::NonBlittableStruct> BoxedStruct()
        {
            return test_component::NonBlittableStruct{ m_stringProp };
        }

        void OutBoxedStruct(Windows::Foundation::IReference<test_component::NonBlittableStruct>& value)
        {
            value = BoxedStruct();
        }

        test_component::NestedNonBlittableStruct NestedStruct()
        {
            return { { m_stringProp } };
        }

        Windows::Foundation::IReference<test_component::NestedNonBlittableStruct> BoxedNestedStruct()
        {
            return NestedStruct();
        }

        void StoreStrings(array_view<hstring const> values)
        {
            m_stringProp = values[0];
        }

        void StoreStructs(array_view<test_component::NonBlittableStruct const> values)
        {
            m_stringProp = values[0].First;
        }

        void StoreNestedStruct(test_component::NestedNonBlittableStruct const& value)
        {
            m_stringProp = value.Value.First;
        }

        com_array<hstring> StoredStrings()
        {
            return { m_stringProp, m_stringProp };
        }

        com_array<test_component::NonBlittableStruct> StoredStructs()
        {
            return { { m_stringProp }, { m_stringProp } };
        }

        void OutStoredStrings(com_array<hstring>& values)
        {
            values = StoredStrings();
        }

        void FillStoredStrings(array_view<hstring> values)
        {
            for (auto& value : values) value = m_stringProp;
        }

        Windows::Foundation::Collections::IVector<hstring> StoredStringVector()
        {
            return single_threaded_vector<hstring>({ m_stringProp, m_stringProp });
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
