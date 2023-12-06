#pragma once
#include "BaseCollection.g.h"

namespace winrt::test_component::implementation
{
    struct BaseCollection : BaseCollectionT<BaseCollection>
    {
        BaseCollection() = default;

        Windows::Foundation::Collections::IIterator<test_component::Base> First();
        test_component::Base GetAt(uint32_t index);
        uint32_t Size();
        Windows::Foundation::Collections::IVectorView<test_component::Base> GetView();
        bool IndexOf(test_component::Base const& value, uint32_t& index);
        void SetAt(uint32_t index, test_component::Base const& value);
        void InsertAt(uint32_t index, test_component::Base const& value);
        void RemoveAt(uint32_t index);
        void Append(test_component::Base const& value);
        void RemoveAtEnd();
        void Clear();
        uint32_t GetMany(uint32_t startIndex, array_view<test_component::Base> items);
        void ReplaceAll(array_view<test_component::Base const> items);
    };
}


namespace winrt::test_component::factory_implementation
{
    struct BaseCollection : BaseCollectionT<BaseCollection, implementation::BaseCollection>
    {
    };
}