#include "../pch.h"
#include "../types.h"
#include "../type_writers.h"

#include "type_writers.h"

using namespace swiftwinrt;

const write_type_params write_type_params::swift{ projection_layer::swift };
const write_type_params write_type_params::c_abi{ projection_layer::c_abi };
const write_type_params write_type_params::swift_allow_implicit_unwrap{ projection_layer::swift, true };
const write_type_params write_type_params::swift_omit_generics{ projection_layer::swift, false, true };


void write_swift_type_identifier_ex(writer& w, metadata_type const& type, bool existential, bool omit_generic_args);

// Writes the Swift code representation of a WinRT type at the Swift projection layer
// as a 'type' syntax node.
static void write_swift_type(writer& w, metadata_type const& type, bool allow_implicit_unwrap, bool omit_generic_args)
{
    char optional_suffix = allow_implicit_unwrap ? '!' : '?';

    // Handle types with special codegen
    if (auto gen_inst = dynamic_cast<const generic_inst*>(&type))
    {
        const auto& generic_typedef = *gen_inst->generic_type();
        const auto& generic_params = gen_inst->generic_params();
        if (is_winrt_ireference(generic_typedef))
        {
            assert(!omit_generic_args); // there is no generic type to omit args from
            auto boxed_type = generic_params[0];
            w.write("%?", bind<write_swift_type>(*boxed_type, /* allow_implicit_unwrap: */ false, /* omit_generic_args */false));
            return;
        }

        if (is_delegate(gen_inst))
        {
            w.write(remove_backtick(gen_inst->swift_type_name()));

            if (!omit_generic_args)
            {
                w.write("<");
                separator sep{ w };
                for (const auto& gen_arg : generic_params)
                {
                    sep();
                    // Implicitly unwrap optionals are illegal on generic arguments
                    write_swift_type(w, *gen_arg, /* allow_implicit_unwrap: */ false, /* omit_generic_args */ false);
                }
                w.write(">%", optional_suffix);
            }
            return;
        }
    }

    if (is_reference_type(&type))
    {
        bool existential = is_interface(&type);
        w.write("%%", bind<write_swift_type_identifier_ex>(type, existential, omit_generic_args), optional_suffix);
    }
    else
    {
        write_swift_type_identifier(w, type);
    }
}

void write_swift_type_identifier_ex(writer& w, metadata_type const& type, bool existential, bool omit_generic_args)
{
    if (auto elem_type = dynamic_cast<const element_type*>(&type))
    {
        if (elem_type->type() == ElementType::Object)
        {
            w.write("Any");
        }
        else
        {
            // Primitive numeric types and String
            w.write_swift(elem_type->type());
        }
    }
    else if (auto mapped = dynamic_cast<const mapped_type*>(&type))
    {
        // mapped types are defined in headers and *not* metadata files, so these don't follow the same
        // naming conventions that other types do. We just grab the type name and will use that.
        auto swift_name = mapped->swift_type_name();
        w.write(swift_name == "HResult" ? "HRESULT" : swift_name);
    }
    else if (auto systype = dynamic_cast<const system_type*>(&type))
    {
        if (systype->category() == param_category::guid_type)
        {
            // GUID requires full type name due to collisions with WinSDK
            w.write(systype->swift_full_name());
        }
        else
        {
            w.write(systype->swift_type_name());
        }
    }
    else if (auto type_def = dynamic_cast<const typedef_base*>(&type))
    {
        // Make sure the module gets imported
        w.add_depends(type);

        // Module
        if (type_def->is_generic())
        {
            // Generic instances are always in the support module
            if (w.full_type_names || (get_swift_module(w.type_namespace) != w.support))
            {
                w.write("%.", w.support);
            }
        }
        else if (w.full_type_names || type.swift_logical_namespace() != w.type_namespace)
        {
            w.write("%.", get_swift_module(type.swift_logical_namespace()));
        }

        auto iface = dynamic_cast<const interface_type*>(type_def);
        assert(!existential || iface);
        if (existential && iface)
        {
            // Use the "AnyIFoo" typealias for any IFoo,
            // to avoid needing parens in (any IFoo)?
            w.write("Any");
        }

        w.write(remove_backtick(type.swift_type_name()));

        if (omit_generic_args == false && !type_def->generic_params.empty())
        {
            // if writing an extistential then we always want to put the generic types in the name
            w.write("<");
            separator sep{ w };
            for (const auto& gen_arg : type_def->generic_params)
            {
                sep();
                // Implicitly unwrap optionals are illegal on generic arguments
                write_swift_type(w, gen_arg, /* allow_implicit_unwrap: */ false, /* omit_generic_args */ false);
            }
            w.write(">");
        }
    }
    else if (auto gen_inst = dynamic_cast<const generic_inst*>(&type))
    {
        const auto& generic_typedef = *gen_inst->generic_type();

        // Special generic types
        if (is_winrt_ireference(generic_typedef))
        {
            throw std::exception("Special type IReference"
                " cannot be represented as a Swift type-identifier syntax node.");
        }

        write_swift_type_identifier_ex(w, generic_typedef, existential,/* omit_generic_args */ true);

        w.write("<");
        separator sep{ w };
        for (const auto& gen_arg : gen_inst->generic_params())
        {
            sep();

            // Implicitly unwrap optionals are illegal on generic arguments
            write_swift_type(w, *gen_arg, /* allow_implicit_unwrap: */ false, /* omit_generic_args */ false);
        }
        w.write(">");
    }
    else if (auto param = dynamic_cast<const generic_type_parameter*>(&type))
    {
        w.write(param->swift_type_name());
    }
    else
    {
        throw std::exception("Unexpected metadata_type");
    }
}

void swiftwinrt::write_swift_type_identifier(writer& w, metadata_type const& type) {
    write_swift_type_identifier_ex(w, type, /* existential: */ false, /* omit_generic_args */ true);
}

void swiftwinrt::write_swift_interface_existential_identifier(writer& w, metadata_type const& iface) {
    assert(is_interface(iface));
    write_swift_type_identifier_ex(w, iface, /* existential: */ true, /* omit_generic_args */ false);
}

// Writes the Swift code representation of a WinRT type at the C ABI projection layer
// as a 'type' syntax node.
static void write_c_abi_type(writer& w, metadata_type const& type)
{
    if (auto elem_type = dynamic_cast<const element_type*>(&type))
    {
        if (elem_type->type() == ElementType::Object)
        {
            w.write("UnsafeMutablePointer<C_IInspectable>?");
        }
        else if (elem_type->type() == ElementType::String)
        {
            // Projecting as optional makes it easier to interop
            // with UnsafePointers
            w.write("HSTRING?");
        }
        else
        {
            // Primitive numeric types
            auto abi_guard = w.push_abi_types(true);
            w.write_abi(elem_type->type());
        }
    }
    else if (auto mapped = dynamic_cast<const mapped_type*>(&type))
    {
        w.write(mapped->cpp_abi_name());
    }
    else if (auto systype = dynamic_cast<const system_type*>(&type))
    {
        if (systype->category() == param_category::guid_type)
        {
            w.write("%.GUID", w.support);
        }
        else
        {
            w.write(systype->cpp_abi_name());
        }
    }
    else
    {
        auto type_def = dynamic_cast<const typedef_base*>(&type);
        const generic_inst* geninst = nullptr;
        if (type_def == nullptr)
        {
            geninst = dynamic_cast<const generic_inst*>(&type);
            if (geninst == nullptr)
            {
                throw std::exception("Unexpected metadata_type");
            }

            type_def = geninst->generic_type();
        }
        else
        {
            if (type_def->is_generic())
            {
                throw std::exception("Cannot write a type expression of a generic type definition.");
            }
        }

        auto abi_guard = w.push_abi_types(true);
        auto mangling_guard = w.push_mangled_names_if_needed(get_category(&type));

        std::optional<writer::generic_param_guard> generic_params_guard;
        if (geninst)
        {
            generic_params_guard = w.push_generic_params(*geninst);
        }

        if (is_reference_type(&type))
        {
            w.write("UnsafeMutablePointer<%>?", type);
        }
        else
        {
            w.write(type);
        }
    }
}

void swiftwinrt::write_type(writer& w, metadata_type const& type, write_type_params const& params)
{
    if (params.layer == projection_layer::swift)
    {
        write_swift_type(w, type, params.allow_implicit_unwrap, params.omit_generic_args);
    }
    else
    {
        write_c_abi_type(w, type);
    }
}

void swiftwinrt::write_default_value(writer& w, metadata_type const& sig, projection_layer layer)
{
    auto category = get_category(&sig);

    if (category == param_category::object_type || category == param_category::generic_type)
    {
        w.write("nil");
    }
    else if (category == param_category::string_type)
    {
        // abi representation is HRESULT? which defaults to nil
        // swift representation is String, which must be initialized to ""
        if (layer == projection_layer::swift) w.write("\"\"");
    }
    else if (category == param_category::struct_type || is_guid(category))
    {
        w.write(".init()");
    }
    else if (category == param_category::enum_type)
    {
        w.write(".init(0)");
    }
    else if (is_boolean(&sig))
    {
        w.write("%", layer == projection_layer::c_abi ? "0" : "false");
    }
    else
    {
        w.write("%", is_floating_point(&sig) ? "0.0" : "0");
    }
}

void swiftwinrt::write_default_init_assignment(writer& w, metadata_type const& sig, projection_layer layer)
{
    auto category = get_category(&sig);

    if (category == param_category::object_type ||
        category == param_category::generic_type ||
        (category == param_category::string_type && layer == projection_layer::c_abi))
    {
        // Projected to Optional and default-initialized to nil
    }
    else
    {
        w.write(" = %", bind<write_default_value>(sig, layer));
    }
}

write_type_params swiftwinrt::swift_write_type_params_for(metadata_type const& type, bool is_array)
{
    // When implementing a generic interface,
    // we cannot use implicit unwrapping, because of Swift limitations:
    // typename Element must be Base? and not Base!,
    // and declaring GetAt(_: UInt32) -> Base! would not bind to GetAt(_: UInt32) -> Element.
    auto is_generic = is_generic_def(type) || is_generic_inst(type) ||  is_array;
    return is_generic ? write_type_params::swift : write_type_params::swift_allow_implicit_unwrap;
}
