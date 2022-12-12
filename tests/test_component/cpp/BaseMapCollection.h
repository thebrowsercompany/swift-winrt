#pragma once
#include "BaseMapCollection.g.h"

namespace winrt::test_component::implementation
{
    struct BaseMapCollection : BaseMapCollectionT<BaseMapCollection>
    {
        BaseMapCollection() = default;

        Windows::Foundation::Collections::IIterator<Windows::Foundation::Collections::IKeyValuePair<hstring, test_component::Base>> First();
        test_component::Base Lookup(hstring const& key);
        uint32_t Size();
        bool HasKey(hstring const& key);
        Windows::Foundation::Collections::IMapView<hstring, test_component::Base> GetView();
        bool Insert(hstring const& key, test_component::Base const& value);
        void Remove(hstring const& key);
        void Clear();
    };
}
