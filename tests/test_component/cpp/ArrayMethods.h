#pragma once
#include "ArrayMethods.g.h"

namespace winrt::test_component::implementation
{
    struct ArrayMethods
    {
        ArrayMethods() = default;

        static hstring InInt32Array(array_view<int32_t const> value);
        static hstring InStringArray(array_view<hstring const> value);
        static hstring InObjectArray(array_view<winrt::Windows::Foundation::IInspectable const> value);
        static hstring InStringableArray(array_view<winrt::Windows::Foundation::IStringable const> value);
        static hstring InStructArray(array_view<winrt::test_component::BlittableStruct const> value);
        static hstring InNonBlittableStructArray(array_view<winrt::test_component::NonBlittableStruct const> value);
        static hstring InEnumArray(array_view<winrt::test_component::Signed const> value);

    private:

        static hstring InEnum(winrt::test_component::Signed const& value);

        template<typename T>
        static void simulate_rpc_behavior(array_view<T> const& value)
        {
            // RPC requires array pointers to be non-null.
            if (value.begin() == nullptr)
            {
                throw hresult_error(static_cast<hresult>(0x800706f4)); // HRESULT_FROM_WIN32(RPC_X_NULL_REF_POINTER)
            }
        }
    };
}
namespace winrt::test_component::factory_implementation
{
    struct ArrayMethods : ArrayMethodsT<ArrayMethods, implementation::ArrayMethods>
    {
    };
}
