#pragma once
#include "BufferTester.g.h"

namespace winrt::test_component::implementation
{
    struct BufferTester
    {
        BufferTester() = default;

        static uint8_t GetDataFrom(winrt::Windows::Storage::Streams::IBuffer const& buffer, uint32_t index);
    };
}
namespace winrt::test_component::factory_implementation
{
    struct BufferTester : BufferTesterT<BufferTester, implementation::BufferTester>
    {
    };
}
