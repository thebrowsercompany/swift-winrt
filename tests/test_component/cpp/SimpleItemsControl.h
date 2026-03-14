#pragma once
#include "SimpleItemsControl.g.h"

namespace winrt::test_component::implementation
{
    struct SimpleItemsControl : SimpleItemsControlT<SimpleItemsControl>
    {
        SimpleItemsControl() = default;

        winrt::Windows::Foundation::IInspectable ItemsSource();
        void ItemsSource(winrt::Windows::Foundation::IInspectable const& value);
        int32_t ItemCount();

    private:
        winrt::Windows::Foundation::IInspectable m_itemsSource{ nullptr };
        int32_t m_itemCount{ 0 };
    };
}
namespace winrt::test_component::factory_implementation
{
    struct SimpleItemsControl : SimpleItemsControlT<SimpleItemsControl, implementation::SimpleItemsControl>
    {
    };
}
