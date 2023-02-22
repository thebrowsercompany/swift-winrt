// Functions for writing "type expressions", which are types as they
// appear in variable, field, params and return type declarations,
// and may include decorators such as "any", "?" and generic params.

#pragma once

#include "type_helpers.h"
#include "helpers.h"

namespace swiftwinrt
{
    enum class projection_layer
    {
        swift, // (any IVector<String>)?
        c_abi // UnsafeMutablePointer<__x_IVector_HSTRING>?
    };

    // "ex" to avoid overloading, which breaks bind<>
    // omit_outer_optional will prevent optionals to be added to the outer type,
    // while preserving any optionals in generic type arguments,
    // for example: any IVector<(any IFoo)?>
    // This is useful in typealias contexts where we don't want to capture the optionality
    void write_type_expression_ex(writer& w, metadata_type const& type,
        projection_layer layer, bool omit_outer_optional);

    inline void write_type_expression(writer& w, metadata_type const& type,
        projection_layer layer)
    {
        write_type_expression_ex(w, type, layer, /* omit_outer_optional: */ false);
    }

    void write_default_init_assignment(writer& w, metadata_type const& sig, projection_layer layer);
}