#pragma once
#include "NoopClosable.g.h"

namespace winrt::test_component::implementation
{
    struct NoopClosable : NoopClosableT<NoopClosable>
    {
        NoopClosable() = default;

        void Close() {}
    };
}
namespace winrt::test_component::factory_implementation
{
    struct NoopClosable : NoopClosableT<NoopClosable, implementation::NoopClosable>
    {
    };
}
