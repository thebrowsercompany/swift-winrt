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

        static void OutInt32Array(com_array<int32_t>& value);
        static void OutStringArray(com_array<hstring>& value);
        static void OutObjectArray(com_array<Windows::Foundation::IInspectable>& value);
        static void OutStringableArray(com_array<Windows::Foundation::IStringable>& value);
        static void OutStructArray(com_array<winrt::test_component::BlittableStruct>& value);
        static void OutNonBlittableStructArray(com_array<winrt::test_component::NonBlittableStruct>& value);
        static void OutEnumArray(com_array<Signed>& value);

        static void RefInt32Array(array_view<int32_t> value);
        static void RefStringArray(array_view<hstring> value);
        static void RefObjectArray(array_view<Windows::Foundation::IInspectable> value);
        static void RefStringableArray(array_view<Windows::Foundation::IStringable> value);
        static void RefStructArray(array_view<winrt::test_component::BlittableStruct> value);
        static void RefNonBlittableStructArray(array_view<winrt::test_component::NonBlittableStruct> value);
        static void RefEnumArray(array_view<Signed> value);

        static com_array<int32_t> ReturnInt32Array();
        static com_array<hstring> ReturnStringArray();
        static com_array<Windows::Foundation::IInspectable> ReturnObjectArray();
        static com_array<Windows::Foundation::IStringable> ReturnStringableArray();
        static com_array<winrt::test_component::BlittableStruct> ReturnStructArray();
        static com_array<winrt::test_component::NonBlittableStruct> ReturnNonBlittableStructArray();
        static com_array<Signed> ReturnEnumArray();

        static void TestInArrayThroughSwiftImplementation(winrt::test_component::IArrayScenarios const& scenario, array_view<int32_t const> value);
        static void TestOutArrayThroughSwiftImplementation(winrt::test_component::IArrayScenarios const& scenario, winrt::test_component::ArrayMethodCallback const& callback);
        static void TestRefArrayThroughSwiftImplementation(winrt::test_component::IArrayScenarios const& scenario, array_view<int32_t> value, winrt::test_component::ArrayMethodCallback const& callback);
        static void TestReturnArrayThroughSwiftImplementation(winrt::test_component::IArrayScenarios const& scenario, winrt::test_component::ArrayMethodCallback const& callback);

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
