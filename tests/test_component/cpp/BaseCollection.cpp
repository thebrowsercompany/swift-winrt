#include "pch.h"
#include "BaseCollection.h"
#include "BaseCollection.g.cpp"

namespace winrt::test_component::implementation
{
    Windows::Foundation::Collections::IIterator<test_component::Base> BaseCollection::First()
    {
        throw hresult_not_implemented();
    }
    test_component::Base BaseCollection::GetAt(uint32_t index)
    {
        throw hresult_not_implemented();
    }
    uint32_t BaseCollection::Size()
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::Collections::IVectorView<test_component::Base> BaseCollection::GetView()
    {
        throw hresult_not_implemented();
    }
    bool BaseCollection::IndexOf(test_component::Base const& value, uint32_t& index)
    {
        throw hresult_not_implemented();
    }
    void BaseCollection::SetAt(uint32_t index, test_component::Base const& value)
    {
        throw hresult_not_implemented();
    }
    void BaseCollection::InsertAt(uint32_t index, test_component::Base const& value)
    {
        throw hresult_not_implemented();
    }
    void BaseCollection::RemoveAt(uint32_t index)
    {
        throw hresult_not_implemented();
    }
    void BaseCollection::Append(test_component::Base const& value)
    {
        throw hresult_not_implemented();
    }
    void BaseCollection::RemoveAtEnd()
    {
        throw hresult_not_implemented();
    }
    void BaseCollection::Clear()
    {
        throw hresult_not_implemented();
    }
    uint32_t BaseCollection::GetMany(uint32_t startIndex, array_view<test_component::Base> items)
    {
        throw hresult_not_implemented();
    }
    void BaseCollection::ReplaceAll(array_view<test_component::Base const> items)
    {
        throw hresult_not_implemented();
    }
}
