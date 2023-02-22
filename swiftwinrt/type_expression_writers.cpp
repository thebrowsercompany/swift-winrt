#include "pch.h"
#include "types.h"
#include "type_writers.h"

#include "type_expression_writers.h"

using namespace swiftwinrt;

static void write_swift_type_expression(writer& w, metadata_type const& type, bool omit_outer_optional);

static void write_swift_typedef_expression(writer& w, typedef_base const& type,
    const generic_inst* opt_generic_inst, bool omit_outer_optional)
{
    // Make sure the module gets imported
    w.add_depends(type);

    if (is_interface(&type))
    {
        // Project as existential and optional
        if (!omit_outer_optional)
        {
            w.write("(");
        }

        w.write("any ");
    }

    // Namespace
    if (opt_generic_inst)
    {
        // Generic instances are always in the support module
        w.write("%.", w.support);
    }
    else if (w.full_type_names || type.swift_logical_namespace() != w.type_namespace)
    {
        w.write("%.", get_swift_module(type.swift_logical_namespace()));
    }

    w.write(remove_backtick(type.swift_type_name()));

    // Generic type arguments
    if (opt_generic_inst)
    {
        w.write("<");
        separator sep{ w };
        for (auto&& gen_arg : opt_generic_inst->generic_params())
        {
            sep();
            write_swift_type_expression(w, *gen_arg, /* omit_outer_optional: */ false);
        }
        w.write(">");
    }

    if (!omit_outer_optional)
    {
        if (is_interface(&type))
        {
            w.write(")"); // Close existential parenthesis
        }

        if (is_reference_type(&type))
        {
            w.write("?"); // Project as optional
        }
    }
}

static void write_swift_type_expression(writer& w, metadata_type const& type, bool omit_outer_optional)
{
    if (auto elem_type = dynamic_cast<const element_type*>(&type))
    {
        assert(!omit_outer_optional);
        if (elem_type->type() == ElementType::Object)
        {
            w.write("%.IInspectable", w.support);
        }
        else
        {
            // Primitive numeric types and String
            w.write_swift(elem_type->type());
        }
    }
    else if (auto mapped = dynamic_cast<const mapped_type*>(&type))
    {
        assert(!omit_outer_optional);
        // mapped types are defined in headers and *not* metadata files, so these don't follow the same
        // naming conventions that other types do. We just grab the type name and will use that.
        auto swift_name = mapped->swift_type_name();
        w.write(swift_name == "HResult" ? "HRESULT" : swift_name);
    }
    else if (auto systype = dynamic_cast<const system_type*>(&type))
    {
        assert(!omit_outer_optional);
        w.write(systype->swift_type_name());
    }
    else if (auto type_def = dynamic_cast<const typedef_base*>(&type))
    {
        assert(!is_generic(type));
        write_swift_typedef_expression(w, *type_def, /* opt_generic_params: */ nullptr, omit_outer_optional);
    }
    else if (auto geninst = dynamic_cast<const generic_inst*>(&type))
    {
        auto&& gentype = *geninst->generic_type();
        auto&& genparams = geninst->generic_params();

        // Special generic types
        bool typed_event_handler = false;
        if (is_ireference(gentype))
        {
            assert(!omit_outer_optional);
            auto boxed_type = genparams[0];
            w.write("%?", bind<write_swift_type_expression>(*boxed_type, /* omit_outer_optional: */ false));
        }
        else if (is_eventhandler(gentype))
        {
            assert(!omit_outer_optional);
            auto args_type = genparams[0];
            w.write("^@escaping (%.IInspectable,%) -> ()",
                w.support, bind<write_swift_type_expression>(*args_type, /* omit_outer_optional: */ false));
        }
        else if (is_typedeventhandler(gentype))
        {
            assert(!omit_outer_optional);
            auto sender_type = genparams[0];
            auto args_type = genparams[1];
            w.write("^@escaping (%,%) -> ()",
                bind<write_swift_type_expression>(*sender_type, /* omit_outer_optional: */ false),
                bind<write_swift_type_expression>(*args_type, /* omit_outer_optional: */ false));
        }
        else
        {
            // Collections and other generic types
            write_swift_typedef_expression(w, gentype, geninst, omit_outer_optional);
        }
    }
}

static void write_c_abi_type_expression(writer& w, metadata_type const& type, bool omit_outer_optional)
{
    if (auto elem_type = dynamic_cast<const element_type*>(&type))
    {
        if (elem_type->type() == ElementType::Object)
        {
            w.write("UnsafeMutablePointer<%.IInspectable>?", w.c_mod);
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
        // mapped types are defined in headers and *not* metadata files, so these don't follow the same
        // naming conventions that other types do. We just grab the type name and will use that.
        auto swift_name = mapped->swift_type_name();
        w.write(swift_name == "HResult" ? "HRESULT" : swift_name);
    }
    else if (auto systype = dynamic_cast<const system_type*>(&type))
    {
        w.write(systype->cpp_abi_name());
    }
    else
    {
        auto geninst = dynamic_cast<const generic_inst*>(&type);
        auto type_def = geninst == nullptr
            ? dynamic_cast<const typedef_base*>(&type)
            : geninst->generic_type();
        if (type_def == nullptr)
        {
            assert(!"Unexpected metadata_type");
            return;
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
            w.write("UnsafeMutablePointer<%>", type);
            if (!omit_outer_optional)
            {
                w.write("?");
            }
        }
        else
        {
            w.write(type);
        }
    }
}

void swiftwinrt::write_type_expression_ex(writer& w, metadata_type const& type,
    projection_layer layer, bool omit_outer_optional)
{
    if (layer == projection_layer::swift)
    {
        write_swift_type_expression(w, type, omit_outer_optional);
    }
    else
    {
        write_c_abi_type_expression(w, type, omit_outer_optional);
    }
}


void swiftwinrt::write_default_init_assignment(writer& w, metadata_type const& sig, projection_layer layer)
{
    auto category = get_category(&sig);

    if (category == param_category::object_type || category == param_category::generic_type)
    {
        // Projected to Optional and default-initialized to nil
    }
    else if (category == param_category::string_type)
    {
        // abi representation is HRESULT? which defaults to nil
        // swift representation is String, which must be initialized to ""
        if (layer == projection_layer::swift) w.write(" = \"\"");
    }
    else if (category == param_category::struct_type || is_guid(category))
    {
        w.write(" = .init()");
    }
    else if (category == param_category::enum_type)
    {
        w.write(" = .init(0)");
    }
    else if (is_boolean(&sig))
    {
        w.write(" = %", layer == projection_layer::c_abi ? "0" : "false");
    }
    else
    {
        w.write(" = %", is_floating_point(&sig) ? "0.0" : "0");
    }
}