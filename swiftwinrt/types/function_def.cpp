#include "pch.h"

#include "types/function_def.h"

#include "types/type_constants.h"

namespace swiftwinrt
{
    bool function_def::is_async() const
    {
        if (!return_type)
        {
            return false;
        }

        if (return_type->type->swift_logical_namespace() == winrt_foundation_namespace)
        {
            auto return_type_name = return_type->type->swift_type_name();
            return return_type_name == "IAsyncAction" ||
                return_type_name == "IAsyncOperation`1" ||
                return_type_name == "IAsyncActionWithProgress`1" ||
                return_type_name == "IAsyncOperationWithProgress`2";
        }

        return false;
    }
}
