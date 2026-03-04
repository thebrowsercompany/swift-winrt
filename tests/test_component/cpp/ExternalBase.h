#pragma once
#include "external_base/ExternalBase.g.h"

namespace winrt::external_base::implementation
{
    struct ExternalBase : ExternalBaseT<ExternalBase>
    {
        ExternalBase() = default;
    };
}
namespace winrt::external_base::factory_implementation
{
    struct ExternalBase : ExternalBaseT<ExternalBase, implementation::ExternalBase>
    {
    };
}
