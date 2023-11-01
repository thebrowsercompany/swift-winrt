#include "pch.h"
#include "AsyncOperationInt.h"
#include "AsyncMethods.h"
#include "AsyncMethods.g.cpp"

namespace winrt::test_component::implementation
{
    winrt::Windows::Foundation::IAsyncOperation<int32_t> AsyncMethods::GetCompletedAsync(int32_t result)
    {
        auto asyncOp = winrt::make<AsyncOperationInt>();
        asyncOp.Complete(result);
        return asyncOp;
    }

    winrt::Windows::Foundation::IAsyncOperation<int32_t> AsyncMethods::GetCompletedWithErrorAsync(winrt::hresult errorCode)
    {
        auto asyncOp = winrt::make<AsyncOperationInt>();
        asyncOp.CompleteWithError(errorCode);
        return asyncOp;
    }

    winrt::test_component::AsyncOperationInt AsyncMethods::GetPendingAsync()
    {
        return winrt::make<AsyncOperationInt>();
    }
}
