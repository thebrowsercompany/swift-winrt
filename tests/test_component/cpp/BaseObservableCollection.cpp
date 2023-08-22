#include "pch.h"
#include "BaseObservableCollection.h"
#include "BaseObservableCollection.g.cpp"

namespace winrt::test_component::implementation
{
    winrt::Windows::Foundation::Collections::IIterator<winrt::test_component::Base> BaseObservableCollection::First()
    {
        throw hresult_not_implemented();
    }
    winrt::test_component::Base BaseObservableCollection::GetAt(uint32_t index)
    {
        throw hresult_not_implemented();
    }
    uint32_t BaseObservableCollection::Size()
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::Collections::IVectorView<winrt::test_component::Base> BaseObservableCollection::GetView()
    {
        throw hresult_not_implemented();
    }
    bool BaseObservableCollection::IndexOf(winrt::test_component::Base const& value, uint32_t& index)
    {
        throw hresult_not_implemented();
    }
    void BaseObservableCollection::SetAt(uint32_t index, winrt::test_component::Base const& value)
    {
        throw hresult_not_implemented();
    }
    void BaseObservableCollection::InsertAt(uint32_t index, winrt::test_component::Base const& value)
    {
        throw hresult_not_implemented();
    }
    void BaseObservableCollection::RemoveAt(uint32_t index)
    {
        throw hresult_not_implemented();
    }
    void BaseObservableCollection::Append(winrt::test_component::Base const& value)
    {
        throw hresult_not_implemented();
    }
    void BaseObservableCollection::RemoveAtEnd()
    {
        throw hresult_not_implemented();
    }
    void BaseObservableCollection::Clear()
    {
        throw hresult_not_implemented();
    }
    uint32_t BaseObservableCollection::GetMany(uint32_t startIndex, array_view<winrt::test_component::Base> items)
    {
        throw hresult_not_implemented();
    }
    void BaseObservableCollection::ReplaceAll(array_view<winrt::test_component::Base const> items)
    {
        throw hresult_not_implemented();
    }
    winrt::event_token BaseObservableCollection::VectorChanged(winrt::Windows::Foundation::Collections::VectorChangedEventHandler<winrt::test_component::Base> const& vhnd)
    {
        throw hresult_not_implemented();
    }
    void BaseObservableCollection::VectorChanged(winrt::event_token const& token) noexcept
    {
        throw hresult_not_implemented();
    }
}
