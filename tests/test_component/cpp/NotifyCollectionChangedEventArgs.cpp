#include "pch.h"
#include "NotifyCollectionChangedEventArgs.h"
#include "Microsoft/UI/Xaml/Interop/NotifyCollectionChangedEventArgs.g.cpp"

namespace winrt::Microsoft::UI::Xaml::Interop::implementation
{
    NotifyCollectionChangedEventArgs::NotifyCollectionChangedEventArgs(
        winrt::Microsoft::UI::Xaml::Interop::NotifyCollectionChangedAction const& action,
        winrt::Microsoft::UI::Xaml::Interop::IBindableVector const& newItems,
        int32_t newStartingIndex) : m_action(action), m_newItems(newItems), m_newStartingIndex(newStartingIndex)
    {
    }

    winrt::Microsoft::UI::Xaml::Interop::NotifyCollectionChangedAction NotifyCollectionChangedEventArgs::Action()
    {
        return m_action;
    }

    winrt::Microsoft::UI::Xaml::Interop::IBindableVector NotifyCollectionChangedEventArgs::NewItems()
    {
        return m_newItems;
    }
    int32_t NotifyCollectionChangedEventArgs::NewStartingIndex()
    {
        return m_newStartingIndex;
    }
}
