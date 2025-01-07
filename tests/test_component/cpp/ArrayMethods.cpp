#include "pch.h"
#include "ArrayMethods.h"
#include "ArrayMethods.g.cpp"

namespace winrt::test_component::implementation
{
    using namespace Windows::Foundation;

    struct Value : implements<Value, IStringable>
    {
        Value(int32_t value) :
            m_value(value)
        {
        }

        hstring ToString()
        {
            return hstring{ std::to_wstring(m_value) };
        }

    private:

        int32_t m_value{};
    };

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

    void ArrayMethods::OutInt32Array(com_array<int32_t>& value)
    {
        value = { 1,2,3 };
    }

    void ArrayMethods::OutStringArray(com_array<hstring>& value)
    {
        value = { L"1", L"2", L"3" };
    }

    void ArrayMethods::OutObjectArray(com_array<Windows::Foundation::IInspectable>& value)
    {
        value = { make<Value>(1), make<Value>(2), make<Value>(3) };
    }

    void ArrayMethods::OutStringableArray(com_array<Windows::Foundation::IStringable>& value)
    {
        value = { make<Value>(1), make<Value>(2), make<Value>(3) };
    }

    void ArrayMethods::OutStructArray(com_array<BlittableStruct>& value)
    {
        value = { { 1, 2 }, { 3, 4 }, { 5, 6 } };
    }

    void ArrayMethods::OutNonBlittableStructArray(com_array<NonBlittableStruct>& value)
    {
        value = { { L"1", L"2", 3, L"4" }, { L"5", L"6", 7, L"8" }, { L"9", L"10", 11, L"12" } };
    }

    void ArrayMethods::OutEnumArray(com_array<Signed>& value)
    {
        value = { Signed::First, Signed::Second };
    }

    void ArrayMethods::RefInt32Array(array_view<int32_t> value)
    {
        simulate_rpc_behavior(value);

        if (value.size())
        {
            int32_t counter{};

            std::generate(value.begin(), value.end() - 1, [&]
                {
                    return ++counter;
                });
        }
    }

    void ArrayMethods::RefStringArray(array_view<hstring> value)
    {
        simulate_rpc_behavior(value);

        if (value.size())
        {
            int32_t counter{};

            std::generate(value.begin(), value.end() - 1, [&]
                {
                    return hstring{ std::to_wstring(++counter) };
                });
        }
    }

    void ArrayMethods::RefObjectArray(array_view<Windows::Foundation::IInspectable> value)
    {
        simulate_rpc_behavior(value);

        if (value.size())
        {
            int32_t counter{};

            std::generate(value.begin(), value.end() - 1, [&]
                {
                    return make<Value>(++counter);
                });
        }
    }

    void ArrayMethods::RefStringableArray(array_view<Windows::Foundation::IStringable> value)
    {
        simulate_rpc_behavior(value);

        if (value.size())
        {
            int32_t counter{};

            std::generate(value.begin(), value.end() - 1, [&]
                {
                    return make<Value>(++counter);
                });
        }
    }

    void ArrayMethods::RefStructArray(array_view<BlittableStruct> value)
    {
        simulate_rpc_behavior(value);

        if (value.size())
        {
            int32_t counter{};

            std::generate(value.begin(), value.end() - 1, [&]
                {
                    return BlittableStruct{ ++counter, ++counter };
                });
        }
    }

    void ArrayMethods::RefNonBlittableStructArray(array_view<NonBlittableStruct> value)
    {
        simulate_rpc_behavior(value);

        if (value.size())
        {
            int32_t counter{};

            std::generate(value.begin(), value.end() - 1, [&]
                {
                    return NonBlittableStruct{ hstring(std::to_wstring(++counter)), hstring(std::to_wstring(++counter)), ++counter, hstring(std::to_wstring(++counter)) };
                });
        }
    }

    void ArrayMethods::RefEnumArray(array_view<Signed> value)
    {
        simulate_rpc_behavior(value);

        if (value.size())
        {
            Signed counter{ Signed::First };

            std::generate(value.begin(), value.end() - 1, [&]
                {
                    auto result = counter;
                    counter = static_cast<Signed>(static_cast<int32_t>(counter) + 1);
                    return result;
                });
        }
    }

    com_array<int32_t> ArrayMethods::ReturnInt32Array()
    {
        return { 1,2,3 };
    }

    com_array<hstring> ArrayMethods::ReturnStringArray()
    {
        return { L"1", L"2", L"3" };
    }

    com_array<Windows::Foundation::IInspectable> ArrayMethods::ReturnObjectArray()
    {
        return { make<Value>(1), make<Value>(2), make<Value>(3) };
    }

    com_array<Windows::Foundation::IStringable> ArrayMethods::ReturnStringableArray()
    {
        return { make<Value>(1), make<Value>(2), make<Value>(3) };
    }

    com_array<BlittableStruct> ArrayMethods::ReturnStructArray()
    {
        return { { 1, 2 }, { 3, 4 }, { 5, 6 } };
    }
    com_array<NonBlittableStruct> ArrayMethods::ReturnNonBlittableStructArray()
    {
        return { { L"1", L"2", 3, L"4" }, { L"5", L"6", 7, L"8" }, { L"9", L"10", 11, L"12" } };
    }

    com_array<Signed> ArrayMethods::ReturnEnumArray()
    {
        return { Signed::First, Signed::Second };
    }

    void ArrayMethods::TestInArrayThroughSwiftImplementation(winrt::test_component::IArrayScenarios const& scenario, array_view<int32_t const> value)
    {
        scenario.InArray(value);
    }
    void ArrayMethods::TestOutArrayThroughSwiftImplementation(winrt::test_component::IArrayScenarios const& scenario, winrt::test_component::ArrayMethodCallback const& callback)
    {
        com_array<int32_t> value;
        scenario.OutArray(value);
        callback(value);
    }
    void ArrayMethods::TestRefArrayThroughSwiftImplementation(winrt::test_component::IArrayScenarios const& scenario, array_view<int32_t> value, winrt::test_component::ArrayMethodCallback const& callback)
    {
        scenario.RefArray(value);
        callback(value);
    }
    void ArrayMethods::TestReturnArrayThroughSwiftImplementation(winrt::test_component::IArrayScenarios const& scenario, winrt::test_component::ArrayMethodCallback const& callback)
    {
        callback(scenario.ReturnArray());
    }
}
