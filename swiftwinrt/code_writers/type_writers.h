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
        swift, // AnyIVector<String>?
        c_abi // UnsafeMutablePointer<__x_IVector_HSTRING>?
    };

    struct write_type_params
    {
        projection_layer layer;
        bool allow_implicit_unwrap = false;
        bool omit_generic_args = false;

        static const write_type_params swift;
        static const write_type_params c_abi;
        static const write_type_params swift_allow_implicit_unwrap;
        static const write_type_params swift_omit_generics;
    };

    // Writes the Swift code representation of a WinRT type as a 'type' syntax node.
    // This is a type as can appear in a variable, parameter, return or property declaration,
    // and may include existentials and "?".
    // For example: AnyIVector<AnyIVector<String>?>?
    void write_type(writer& w, metadata_type const& type, write_type_params const& params);

    // Writes the Swift code representation of a WinRT type at the Swift projection layer
    // as a 'type-identifier' syntax node. This includes:
    // - (optional) The module name as a scoping prefix
    // - The name of the type
    // - (for generic instances) the generic arguments
    // Does not include decorators like existentials or "?" on the outer type,
    // but may include them on generic arguments.
    // For example: IVector<AnyIVector<String>?>
    void write_swift_type_identifier(writer& w, metadata_type const& type);

    // Writes the existential version of an interface, such as AnyIClosable for "any IClosable"
    void write_swift_interface_existential_identifier(writer& w, metadata_type const& iface);

    void write_default_value(writer& w, metadata_type const& sig, projection_layer layer);
    void write_default_init_assignment(writer& w, metadata_type const& sig, projection_layer layer);

    write_type_params swift_write_type_params_for(metadata_type const& type, bool is_array = false);
}