#pragma once
#include "StaticClass.g.h"

namespace winrt::test_component::implementation
{
    struct StaticClass
    {
        StaticClass() = default;

        static test_component::Fruit EnumProperty();
        static void EnumProperty(test_component::Fruit const& value);
        static hstring InEnum(test_component::Signed const& value);
        static hstring InNonBlittableStruct(test_component::NonBlittableStruct const& value);

        private:
        static test_component::Fruit s_fruit;
    };
}
namespace winrt::test_component::factory_implementation
{
    struct StaticClass : StaticClassT<StaticClass, implementation::StaticClass>
    {
    };
}
