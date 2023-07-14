#include "pch.h"
#include "CollectionTester.h"
#include "CollectionTester.g.cpp"

namespace winrt::test_component::implementation
{

    hstring CollectionTester::InIterable(Windows::Foundation::Collections::IIterable<hstring> const& value)
    {
        return value.First().Current();
    }
    hstring CollectionTester::InIterablePair(Windows::Foundation::Collections::IIterable<Windows::Foundation::Collections::IKeyValuePair<hstring, hstring>> const& value)
    {
        return value.First().Current().Key();
    }

    hstring CollectionTester::InMap(Windows::Foundation::Collections::IMap<hstring, hstring> const& value)
    {
        return value.Lookup(L"A");
    }
    hstring CollectionTester::InMapView(Windows::Foundation::Collections::IMapView<hstring, hstring> const& value)
    {
        return value.Lookup(L"A");
    }

    hstring CollectionTester::InVector(Windows::Foundation::Collections::IVector<hstring> const& value)
    {
        if (value.Size() == 0) return L"empty";
        
        return value.GetAt(0);
    }
    hstring CollectionTester::InVectorView(Windows::Foundation::Collections::IVectorView<hstring> const& value)
    {
        return value.GetAt(0);
    }
    
    Windows::Foundation::Collections::IVector<hstring> CollectionTester::ReturnStoredStringVector()
    {
        if (m_vector.Size() == 0)
        {
            m_vector.Append(L"Hello");
        }
        return m_vector;
    }
    
    Windows::Foundation::Collections::IMap<hstring, hstring> CollectionTester::ReturnMapFromStringToString()
    {
        auto map = winrt::single_threaded_map<hstring, hstring>();
        map.Insert(L"A", L"Alpha");
        return map;
    }

}
