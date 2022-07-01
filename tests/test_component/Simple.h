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
                OutputDebugStringW(L"Accepted");
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

        private:
        test_component::BlittableStruct m_blittableStruct{};
    };
}
namespace winrt::test_component::factory_implementation
{
    struct Simple : SimpleT<Simple, implementation::Simple>
    {
    };
}
