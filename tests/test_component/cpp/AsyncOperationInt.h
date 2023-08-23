#pragma once
#include "AsyncOperationInt.g.h"

namespace winrt::test_component::implementation
{
    struct AsyncOperationInt : AsyncOperationIntT<AsyncOperationInt>
    {
        AsyncOperationInt() = default;

        uint32_t Id();
        winrt::Windows::Foundation::AsyncStatus Status();
        winrt::hresult ErrorCode();
        void Cancel();
        void Close();
        void Completed(winrt::Windows::Foundation::AsyncOperationCompletedHandler<int32_t> const& handler);
        winrt::Windows::Foundation::AsyncOperationCompletedHandler<int32_t> Completed();
        int32_t GetResults();
    };
}
