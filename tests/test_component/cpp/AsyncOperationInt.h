#pragma once

#include "AsyncOperationInt.g.h"

namespace winrt::test_component::implementation
{
    class AsyncOperationInt : public AsyncOperationIntT<AsyncOperationInt>
    {
        winrt::Windows::Foundation::AsyncStatus status = winrt::Windows::Foundation::AsyncStatus::Started;
        winrt::hresult errorCode = S_OK;
        int32_t result = 0;
        winrt::Windows::Foundation::AsyncOperationCompletedHandler<int32_t> completedHandler;
    
    public:
        AsyncOperationInt() = default;

        uint32_t Id() { return 42; }
        winrt::Windows::Foundation::AsyncStatus Status() { return status; }
        winrt::hresult ErrorCode() { return errorCode; }
        void Cancel();
        void Close() {}
        void Complete(int32_t result);
        void CompleteWithError(winrt::hresult errorCode);
        void Completed(winrt::Windows::Foundation::AsyncOperationCompletedHandler<int32_t> const& handler);
        winrt::Windows::Foundation::AsyncOperationCompletedHandler<int32_t> Completed() { return completedHandler; }
        int32_t GetResults();
    };
}
