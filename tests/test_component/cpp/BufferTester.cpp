#include "pch.h"
#include "BufferTester.h"
#include "BufferTester.g.cpp"

namespace winrt::test_component::implementation
{
    uint8_t BufferTester::GetDataFrom(winrt::Windows::Storage::Streams::IBuffer const& buffer, uint32_t index)
    {
        if (index >= buffer.Length())
        {
            throw winrt::hresult_out_of_bounds();
        }
        auto data = buffer.data();
        return data[index];
    }
}
