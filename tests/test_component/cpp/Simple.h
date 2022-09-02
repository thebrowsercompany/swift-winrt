#pragma once
#include "Simple.g.h"

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
        static event_token StaticEvent(Windows::Foundation::EventHandler<IInspectable> const&) { return {}; }
        static void StaticEvent(event_token) { }

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

        private:
        hstring m_stringProp{};
        test_component::BlittableStruct m_blittableStruct{};
        test_component::NonBlittableStruct m_nonBlittableStruct{};
    };
}
namespace winrt::test_component::factory_implementation
{
    struct Simple : SimpleT<Simple, implementation::Simple>
    {
    };
}
