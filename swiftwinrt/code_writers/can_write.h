#pragma once
#include "types.h"
namespace swiftwinrt
{
    // can_write* functions are used to determine if a type/function can be written to the output file.
    // The projection can be fully written, but we still respect type filters, so we may choose
    // not to write certain APIs based on the filter.
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

        for (auto& param : function.params)
        {
            if (!can_write(w, param.type))
            {
                return false;
            }
        }

        if (function.return_type)
        {
            auto returnType = function.return_type.value();
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

    static bool can_write(writer& w, typedef_base const& type)
    {
        return w.filter.includes(type.type());
    }
}