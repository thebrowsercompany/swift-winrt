#include "pch.h"
#include "AsyncOperationInt.h"
#include "AsyncOperationInt.g.cpp"

namespace winrt::test_component::implementation
{
    uint32_t AsyncOperationInt::Id()
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::AsyncStatus AsyncOperationInt::Status()
    {
        throw hresult_not_implemented();
    }
    winrt::hresult AsyncOperationInt::ErrorCode()
    {
        throw hresult_not_implemented();
    }
    void AsyncOperationInt::Cancel()
    {
        throw hresult_not_implemented();
    }
    void AsyncOperationInt::Close()
    {
        throw hresult_not_implemented();
    }
    void AsyncOperationInt::Completed(winrt::Windows::Foundation::AsyncOperationCompletedHandler<int32_t> const& handler)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::AsyncOperationCompletedHandler<int32_t> AsyncOperationInt::Completed()
    {
        throw hresult_not_implemented();
    }
    int32_t AsyncOperationInt::GetResults()
    {
        throw hresult_not_implemented();
    }
}
