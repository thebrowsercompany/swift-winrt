#include "pch.h"
#include "ArrayMethods.h"
#include "ArrayMethods.g.cpp"

namespace winrt::test_component::implementation
{
    using namespace Windows::Foundation;

    hstring ArrayMethods::InInt32Array(array_view<int32_t const> value)
    {
        simulate_rpc_behavior(value);

        hstring result;

        for (auto&& v : value)
        {
            result = result + std::to_wstring(v);
        }

        return result;
    }
    hstring ArrayMethods::InStringArray(array_view<hstring const> value)
    {
        simulate_rpc_behavior(value);

        hstring result;

        for (auto&& v : value)
        {
            result = result + v;
        }

        return result;
    }
    hstring ArrayMethods::InStructArray(array_view<BlittableStruct const> value)
    {
        simulate_rpc_behavior(value);

        hstring result;

        for (auto&& v : value)
        {
            result = result + std::to_wstring(v.First) + std::to_wstring(v.Second);
        }

        return result;
    }
    hstring ArrayMethods::InNonBlittableStructArray(array_view<NonBlittableStruct const> value)
    {
        simulate_rpc_behavior(value);

        hstring result;

        for (auto&& v : value)
        {
            result = result + v.First + v.Second + std::to_wstring(v.Third) + v.Fourth;
        }

        return result;
    }
    hstring ArrayMethods::InObjectArray(array_view<Windows::Foundation::IInspectable const> value)
    {
        simulate_rpc_behavior(value);

        hstring result;

        for (auto&& v : value)
        {
            result = result + v.as<IStringable>().ToString();
        }

        return result;
    }
    hstring ArrayMethods::InStringableArray(array_view<Windows::Foundation::IStringable const> value)
    {
        simulate_rpc_behavior(value);

        hstring result;

        for (auto&& v : value)
        {
            result = result + v.ToString();
        }

        return result;
    }

    hstring ArrayMethods::InEnumArray(array_view<Signed const> value)
    {
        simulate_rpc_behavior(value);

        hstring result;

        for (auto&& v : value)
        {
            result = result + InEnum(v);
        }

        return result;
    }

    hstring ArrayMethods::InEnum(Signed const& value)
    {
        switch (value)
        {
        case Signed::First: return L"First";
        case Signed::Second: return L"Second";
        case Signed::Third: return L"Third";
        }

        throw hresult_invalid_argument();
    }
}
