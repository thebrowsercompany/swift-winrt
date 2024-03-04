#pragma once
#include "Failure.g.h"

namespace winrt::test_component::implementation
{
    struct Failure : FailureT<Failure>
    {
        Failure() = default;

        static void FailedStaticMethod();
        static bool FailedStaticProperty();
        hstring FailedProperty();
        void FailedProperty(hstring const& value);
    };
}
namespace winrt::test_component::factory_implementation
{
    struct Failure : FailureT<Failure, implementation::Failure>
    {
    };
}
