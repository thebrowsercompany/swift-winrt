#pragma once
#include "NullValues.g.h"

namespace winrt::test_component::implementation
{
    struct NullValues
    {
        NullValues() = default;

        static bool IsInterfaceNull(winrt::Windows::Foundation::IClosable const& value) { return value == nullptr; }
        static bool IsGenericInterfaceNull(winrt::Windows::Foundation::Collections::IVector<hstring> const& value) { return value == nullptr; }
        static bool IsClassNull(winrt::test_component::NoopClosable const& value) { return value == nullptr; }
        static bool IsDelegateNull(winrt::test_component::VoidToVoidDelegate const& value) { return value == nullptr; }
        static winrt::Windows::Foundation::IClosable GetNullInterface() { return nullptr; }
        static winrt::Windows::Foundation::Collections::IVector<hstring> GetNullGenericInterface() { return nullptr; }
        static winrt::test_component::NoopClosable GetNullClass() { return nullptr; }
        static winrt::test_component::VoidToVoidDelegate GetNullDelegate() { return nullptr; }
    };
}
namespace winrt::test_component::factory_implementation
{
    struct NullValues : NullValuesT<NullValues, implementation::NullValues>
    {
    };
}
