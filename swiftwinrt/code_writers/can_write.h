#pragma once
#include "types.h"
namespace swiftwinrt
{
    static bool can_write(writer& w, typedef_base const& type);
    static bool can_write(writer& w, const metadata_type* type)
    {
        if (auto typed = dynamic_cast<const typedef_base*>(type))
        {
            return can_write(w, *typed);
        }
        return true;
    }

    static bool can_write(writer& w, TypeDef const& type)
    {
        return can_write(w, &w.cache->find(type.TypeNamespace(), type.TypeName()));
    }

    static bool can_write(writer& w, function_def const& function, bool allow_special = false)
    {
        auto method = function.def;

        // Don't support writing specials (events/properties) unless told to do so (i.e. for vtable)
        if (method.SpecialName() && !allow_special) return false;

        auto method_name = get_swift_name(method);

        for (auto& param : function.params)
        {
            auto param_name = get_swift_name(param);

            // TODO: WIN-32 swiftwinrt: support array types
            if (param.signature.Type().is_szarray() ||
                param.signature.Type().is_array())
            {
                return false;
            }

            // TODO: support reference parameters
            if (param.signature.ByRef() && is_guid(get_category(param.type)))
            {
                return false;
            }

            if (!can_write(w, param.type))
            {
                return false;
            }
        }

        if (function.return_type)
        {
            auto returnType = function.return_type.value();
            if (returnType.signature.Type().is_array() ||
                returnType.signature.Type().is_szarray())
            {
                return false;
            }
            if (!can_write(w, function.return_type.value().type))
            {
                return false;
            }
        }

        return true;
    }

    static bool can_write(writer& w, property_def const& prop)
    {
        if (prop.getter)
        {
            return can_write(w, prop.getter.value(), true);
        }
        if (prop.setter)
        {
            return can_write(w, prop.setter.value(), true);
        }
        assert(false); // property should have at least one
        return true;
    }

    static bool can_write_default(const std::vector<named_interface_info>& required_interfaces)
    {
        for (auto&& iface : required_interfaces)
        {
            // when getting the interfaces we populate them with the type name
            // if we can't write the type name then we can't write the type and
            // so we'll return false
            if (iface.second.is_default)
            {
                if (iface.first.empty()) return false;
            }
        }

        return true;
    }

    static bool can_write(writer& w, typedef_base const& type)
    {
        auto typeName = get_full_type_name(type);
        if (!w.filter.includes(type.type())) return false;

        auto category = get_category(type.type());
        if (category == category::enum_type) return true;

        if (auto iface = dynamic_cast<const interface_type*>(&type))
        {
            if (!can_write_default(iface->required_interfaces))
            {
                return false;
            }
        }
        else if (auto classType = dynamic_cast<const class_type*>(&type))
        {
            if (!can_write_default(classType->required_interfaces))
            {
                return false;
            }
        }

        return true;
    }
}