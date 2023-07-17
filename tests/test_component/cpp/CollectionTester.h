#pragma once
#include "CollectionTester.g.h"

namespace winrt::test_component::implementation
{
    struct CollectionTester : CollectionTesterT<CollectionTester>
    {
        CollectionTester() = default;

        static hstring InIterable(Windows::Foundation::Collections::IIterable<hstring> const& value);
        static hstring InIterablePair(Windows::Foundation::Collections::IIterable<Windows::Foundation::Collections::IKeyValuePair<hstring, hstring>> const& value);
        static hstring InMap(winrt::Windows::Foundation::Collections::IMap<hstring, hstring> const& value);
        static hstring InMapView(winrt::Windows::Foundation::Collections::IMapView<hstring, hstring> const& value);
        static hstring InVector(winrt::Windows::Foundation::Collections::IVector<hstring> const& value);
        static hstring InVectorView(winrt::Windows::Foundation::Collections::IVectorView<hstring> const& value);
        static void GetObjectAt(winrt::Windows::Foundation::Collections::IVector<winrt::Windows::Foundation::IInspectable> const& value, uint32_t index, winrt::test_component::ObjectHandler const& callback);

        winrt::Windows::Foundation::Collections::IVector<hstring> ReturnStoredStringVector();
        winrt::Windows::Foundation::Collections::IMap<hstring, hstring> ReturnMapFromStringToString();

        private:
        Windows::Foundation::Collections::IVector<hstring> m_vector { winrt::single_threaded_vector<hstring>() };
    };
}
namespace winrt::test_component::factory_implementation
{
    struct CollectionTester : CollectionTesterT<CollectionTester, implementation::CollectionTester>
    {
    };
}
