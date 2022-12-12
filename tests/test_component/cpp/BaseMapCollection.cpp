#include "pch.h"
#include "BaseMapCollection.h"
#include "BaseMapCollection.g.cpp"

namespace winrt::test_component::implementation
{
    Windows::Foundation::Collections::IIterator<Windows::Foundation::Collections::IKeyValuePair<hstring, test_component::Base>> BaseMapCollection::First()
    {
        throw hresult_not_implemented();
    }
    test_component::Base BaseMapCollection::Lookup(hstring const& key)
    {
        throw hresult_not_implemented();
    }
    uint32_t BaseMapCollection::Size()
    {
        throw hresult_not_implemented();
    }
    bool BaseMapCollection::HasKey(hstring const& key)
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::Collections::IMapView<hstring, test_component::Base> BaseMapCollection::GetView()
    {
        throw hresult_not_implemented();
    }
    bool BaseMapCollection::Insert(hstring const& key, test_component::Base const& value)
    {
        throw hresult_not_implemented();
    }
    void BaseMapCollection::Remove(hstring const& key)
    {
        throw hresult_not_implemented();
    }
    void BaseMapCollection::Clear()
    {
        throw hresult_not_implemented();
    }
}
