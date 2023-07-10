#pragma once
#include "Microsoft/UI/Xaml/Interop/NotifyCollectionChangedEventArgs.g.h"

namespace winrt::Microsoft::UI::Xaml::Interop::implementation
{
    struct NotifyCollectionChangedEventArgs : NotifyCollectionChangedEventArgsT<NotifyCollectionChangedEventArgs>
    {
        NotifyCollectionChangedEventArgs() = default;

        NotifyCollectionChangedEventArgs(winrt::Microsoft::UI::Xaml::Interop::NotifyCollectionChangedAction const& action, winrt::Microsoft::UI::Xaml::Interop::IBindableVector const& newItems, int32_t newStartingIndex);
        winrt::Microsoft::UI::Xaml::Interop::NotifyCollectionChangedAction Action();
        winrt::Microsoft::UI::Xaml::Interop::IBindableVector NewItems();
        int32_t NewStartingIndex();

    private:
        winrt::Microsoft::UI::Xaml::Interop::NotifyCollectionChangedAction m_action{ winrt::Microsoft::UI::Xaml::Interop::NotifyCollectionChangedAction::Add };
        winrt::Microsoft::UI::Xaml::Interop::IBindableVector m_newItems{ nullptr };
        int32_t m_newStartingIndex{ 0 };
    };
}
namespace winrt::Microsoft::UI::Xaml::Interop::factory_implementation
{
    struct NotifyCollectionChangedEventArgs : NotifyCollectionChangedEventArgsT<NotifyCollectionChangedEventArgs, implementation::NotifyCollectionChangedEventArgs>
    {
    };
}
