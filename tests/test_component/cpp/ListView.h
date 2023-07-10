#pragma once
#include "ListView.g.h"

namespace winrt::test_component::implementation
{
    struct ListView : ListViewT<ListView>
    {
        ListView() = default;

        winrt::Windows::Foundation::IInspectable ItemsSource();
        void ItemsSource(winrt::Windows::Foundation::IInspectable const& value);
        winrt::event_token OnItemAdded(winrt::Windows::Foundation::EventHandler<winrt::Windows::Foundation::IInspectable> const& handler);
        void OnItemAdded(winrt::event_token const& token) noexcept;

        private:
        // meant to mimic api here: https://github.com/microsoft/microsoft-ui-xaml/blob/a7183df20367bc0e2b8c825430597a5c1e6871b6/dev/TreeView/TreeViewNode.cpp#L210C32-L210C32
        void AddItemsFromSource(int index, int count);
        winrt::event<Windows::Foundation::EventHandler<winrt::Windows::Foundation::IInspectable>> m_itemAddedEvent;
        Microsoft::UI::Xaml::Interop::IBindableVector m_itemsSource;

    };
}
namespace winrt::test_component::factory_implementation
{
    struct ListView : ListViewT<ListView, implementation::ListView>
    {
    };
}
