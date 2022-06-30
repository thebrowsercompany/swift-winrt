#include "pch.h"

bool __stdcall test_can_unload_now() noexcept;
void* __stdcall test_get_activation_factory(std::wstring_view const& name);

HRESULT __stdcall DllCanUnloadNow()
{
    return test_can_unload_now() ? S_OK : E_NOT_VALID_STATE;
}

HRESULT __stdcall DllGetActivationFactory(void* classId, void** factory)
{
    try
    {
        std::wstring_view const name{ *reinterpret_cast<winrt::hstring*>(&classId) };
        *factory = test_get_activation_factory(name);

        if (*factory)
        {
            return S_OK;
        }

        return winrt::hresult_class_not_available(name).to_abi();
    }
    catch (...)
    {
        return winrt::to_hresult();
    }
}
