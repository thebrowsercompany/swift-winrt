#include "pch.h"
#include "ListView.h"
#include "ListView.g.cpp"

namespace winrt::test_component::implementation
{
    winrt::Windows::Foundation::IInspectable ListView::ItemsSource()
    {
        return m_itemsSource;
    }
    void ListView::ItemsSource(winrt::Windows::Foundation::IInspectable const& value)
    {
        m_itemsSource = value.as<Windows::UI::Xaml::Interop::IBindableVector>();
        // QI the m_itemsSource to INotifyCollectionChanged to simulate what WinUI does
        auto incc = m_itemsSource.as<Windows::UI::Xaml::Interop::INotifyCollectionChanged>();
        incc.CollectionChanged([this](auto const&, winrt::Windows::UI::Xaml::Interop::NotifyCollectionChangedEventArgs const& e)
        {
            if (e.Action() == winrt::Windows::UI::Xaml::Interop::NotifyCollectionChangedAction::Add)
            {
                AddItemsFromSource(e.NewStartingIndex(), e.NewItems().Size());
            }
        });
    }

    void ListView::AddItemsFromSource(int index, int count)
    {
        for (int i = index + count - 1; i >= index; i--)
        {
            auto item = m_itemsSource.GetAt(i);
            m_itemAddedEvent(m_itemsSource, item);
        }
    }

    winrt::event_token ListView::OnItemAdded(winrt::Windows::Foundation::EventHandler<winrt::Windows::Foundation::IInspectable> const& handler)
    {
        return m_itemAddedEvent.add(handler);
    }
    void ListView::OnItemAdded(winrt::event_token const& token) noexcept
    {
        m_itemAddedEvent.remove(token);
    }
}
