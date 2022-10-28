#include "pch.h"
#include "StaticClass.h"
#include "StaticClass.g.cpp"

namespace winrt::test_component::implementation
{
   test_component::Fruit StaticClass::s_fruit = test_component::Fruit::Orange;

    test_component::Fruit StaticClass::EnumProperty()
    {
        return s_fruit;
    }

    void StaticClass::EnumProperty(test_component::Fruit const& value)
    {
        s_fruit = value;
    }

    hstring StaticClass::InEnum(test_component::Signed const& value)
    {
        switch (value)
        {
            case test_component::Signed::First: return L"First";
            case test_component::Signed::Second: return L"Second";
            case test_component::Signed::Third: return L"Third";
            default: throw hresult_invalid_argument();
        }
    }

    hstring StaticClass::InNonBlittableStruct(test_component::NonBlittableStruct const& value)
    {
        hstring result{};
        for (int32_t i = 0; i < value.Third; i++)
        {
            result = result + value.First + value.Second + value.Fourth; 
        }

        return result;
    }

    void StaticClass::TakeBase(test_component::Base const& base)
    {
        base.DoTheThing();
    }
}
