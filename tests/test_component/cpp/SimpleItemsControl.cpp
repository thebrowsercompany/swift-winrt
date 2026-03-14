#include "pch.h"
#include "SimpleItemsControl.h"
#include "SimpleItemsControl.g.cpp"

namespace winrt::test_component::implementation
{
    winrt::Windows::Foundation::IInspectable SimpleItemsControl::ItemsSource()
    {
        return m_itemsSource;
    }

    void SimpleItemsControl::ItemsSource(winrt::Windows::Foundation::IInspectable const& value)
    {
        m_itemsSource = value;
        m_itemCount = 0;

        if (value)
        {
            // Replicates WinUI's InspectingDataSource (src/controls/dev/Repeater/InspectingDataSource.cpp)
            // used by all controls with ItemsSource (BreadcrumbBar, ItemsRepeater, RadioButtons,
            // NavigationView, TreeView, ItemsView, PipsPager, etc.):
            // 1. QI the Object for IVector<IInspectable>
            // 2. QI that IVector for IIterable<IInspectable> to iterate
            if (auto vector = value.try_as<Windows::Foundation::Collections::IVector<Windows::Foundation::IInspectable>>())
            {
                // WinUI stores the vector, then later iterates via IIterable<IInspectable> QI
                auto iterable = vector.as<Windows::Foundation::Collections::IIterable<Windows::Foundation::IInspectable>>();
                auto iterator = iterable.First();
                while (iterator.HasCurrent())
                {
                    m_itemCount++;
                    iterator.MoveNext();
                }
            }
            else if (auto iterable = value.try_as<Windows::Foundation::Collections::IIterable<Windows::Foundation::IInspectable>>())
            {
                auto iterator = iterable.First();
                while (iterator.HasCurrent())
                {
                    m_itemCount++;
                    iterator.MoveNext();
                }
            }
        }
    }

    int32_t SimpleItemsControl::ItemCount()
    {
        return m_itemCount;
    }
}
