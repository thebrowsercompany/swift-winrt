#pragma once
#include "AsyncMethods.g.h"

namespace winrt::test_component::implementation
{
    struct AsyncMethods
    {
        AsyncMethods() = default;

        static winrt::Windows::Foundation::IAsyncOperation<int32_t> GetCompletedAsync(int32_t result);
        static winrt::Windows::Foundation::IAsyncOperation<int32_t> GetCompletedWithErrorAsync(winrt::hresult errorCode);
        static winrt::Windows::Foundation::IAsyncOperation<int32_t> GetPendingAsync();
    };
}

namespace winrt::test_component::factory_implementation
{
    struct AsyncMethods : AsyncMethodsT<AsyncMethods, implementation::AsyncMethods>
    {
    };
}
