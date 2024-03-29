#pragma once
#include "WeakReferencer.g.h"

namespace winrt::test_component::implementation
{
    struct WeakReferencer : WeakReferencerT<WeakReferencer>
    {
        WeakReferencer() = default;

        WeakReferencer(test_component::IReferenceTarget const& object);
        test_component::IReferenceTarget Resolve();

    private:
        winrt::weak_ref<test_component::IReferenceTarget> target;
    };
}
namespace winrt::test_component::factory_implementation
{
    struct WeakReferencer : WeakReferencerT<WeakReferencer, implementation::WeakReferencer>
    {
    };
}
