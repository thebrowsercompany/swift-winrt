#include "pch.h"
#include "WeakReferencer.h"
#include "WeakReferencer.g.cpp"

namespace winrt::test_component::implementation
{
    WeakReferencer::WeakReferencer(test_component::IReferenceTarget const& object)
    {
        this->target = winrt::make_weak(object);
    }
    test_component::IReferenceTarget WeakReferencer::Resolve()
    {
        return target.get();
    }
}
