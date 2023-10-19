#include "pch.h"
#include "AsyncOperationInt.h"
#include "AsyncOperationInt.g.h"

namespace winrt::test_component::implementation
{
    void AsyncOperationInt::Cancel()
    {
        if (status == winrt::Windows::Foundation::AsyncStatus::Started)
        {
            status = winrt::Windows::Foundation::AsyncStatus::Canceled;
            if (completedHandler) completedHandler(*this, status);
        }
    }

    void AsyncOperationInt::Complete(int32_t result)
    {
        if (status != winrt::Windows::Foundation::AsyncStatus::Started) throw winrt::hresult_illegal_method_call();
        this->result = result;
        status = winrt::Windows::Foundation::AsyncStatus::Completed;
        if (completedHandler)
            completedHandler(*this, status);
    }

    void AsyncOperationInt::CompleteWithError(winrt::hresult errorCode)
    {
        if (status != winrt::Windows::Foundation::AsyncStatus::Started) throw winrt::hresult_illegal_method_call();
        this->errorCode = errorCode;
        status = winrt::Windows::Foundation::AsyncStatus::Error;
        if (completedHandler)
            completedHandler(*this, status);
    }

    void AsyncOperationInt::Completed(winrt::Windows::Foundation::AsyncOperationCompletedHandler<int32_t> const& handler)
    {
        completedHandler = handler;
        if (handler && status != winrt::Windows::Foundation::AsyncStatus::Started)
            handler(*this, status);
    }
}
