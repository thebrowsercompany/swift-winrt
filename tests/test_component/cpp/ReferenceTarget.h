#pragma once
#include "ReferenceTarget.g.h"

namespace winrt::test_component::implementation
{
    struct ReferenceTarget : ReferenceTargetT<ReferenceTarget>
    {
        ReferenceTarget() = default;

        void Method() {}
    };
}
namespace winrt::test_component::factory_implementation
{
    struct ReferenceTarget : ReferenceTargetT<ReferenceTarget, implementation::ReferenceTarget>
    {
    };
}
