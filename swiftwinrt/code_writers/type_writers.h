// Functions for writing Swift code for WinRT types.
// For references to Swift syntax nodes, see
// https://docs.swift.org/swift-book/documentation/the-swift-programming-language/types

#pragma once

#include "type_helpers.h"
#include "../helpers.h"

namespace swiftwinrt
{
    enum class projection_layer
    {
        swift, // (any IVector<String>)?
        c_abi // UnsafeMutablePointer<__x_IVector_HSTRING>?
    };

    // Writes the Swift code representation of a WinRT type as a 'type' syntax node.
    // This is a type as can appear in a variable, parameter, return or property declaration,
    // and may include decorators like "any" or "?".
    // For example: (any IVector<(any IVector<String>)?>)?
    void write_type(writer& w, metadata_type const& type, projection_layer layer);

    // Writes the Swift code representation of a WinRT type at the Swift projection layer
    // as a 'type-identifier' syntax node. This includes:
    // - (optional) The module name as a scoping prefix
    // - The name of the type
    // - (for generic instances) the generic arguments
    // Does not include decorators like "any" or "?" on the outer type,
    // but may include them on generic arguments.
    // For example: IVector<(any IVector<String>)?>
    void write_swift_type_identifier(writer& w, metadata_type const& type);

    void write_default_init_assignment(writer& w, metadata_type const& sig, projection_layer layer);
}