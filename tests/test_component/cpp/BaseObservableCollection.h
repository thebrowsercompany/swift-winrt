#pragma once
#include "BaseObservableCollection.g.h"

namespace winrt::test_component::implementation
{
    struct BaseObservableCollection : BaseObservableCollectionT<BaseObservableCollection>
    {
        BaseObservableCollection() = default;

        winrt::Windows::Foundation::Collections::IIterator<winrt::test_component::Base> First();
        winrt::test_component::Base GetAt(uint32_t index);
        uint32_t Size();
        winrt::Windows::Foundation::Collections::IVectorView<winrt::test_component::Base> GetView();
        bool IndexOf(winrt::test_component::Base const& value, uint32_t& index);
        void SetAt(uint32_t index, winrt::test_component::Base const& value);
        void InsertAt(uint32_t index, winrt::test_component::Base const& value);
        void RemoveAt(uint32_t index);
        void Append(winrt::test_component::Base const& value);
        void RemoveAtEnd();
        void Clear();
        uint32_t GetMany(uint32_t startIndex, array_view<winrt::test_component::Base> items);
        void ReplaceAll(array_view<winrt::test_component::Base const> items);
        winrt::event_token VectorChanged(winrt::Windows::Foundation::Collections::VectorChangedEventHandler<winrt::test_component::Base> const& vhnd);
        void VectorChanged(winrt::event_token const& token) noexcept;
    };
}
