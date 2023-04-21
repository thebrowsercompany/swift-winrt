#pragma once
#include "types.h"
namespace swiftwinrt
{
    static bool can_write(writer& w, typedef_base const& type);
    static bool can_write(generic_inst const& type);
    static bool can_write(writer& w, const metadata_type* type)
    {
        if (auto typed = dynamic_cast<const typedef_base*>(type))
        {
            return can_write(w, *typed);
        }
        if (auto generics = dynamic_cast<const generic_inst*>(type))
        {
            return can_write(*generics);
        }
        if (auto mapped = dynamic_cast<const mapped_type*>(type))
        {
            // TODO: https://linear.app/the-browser-company/issue/WIN-103/swiftwinrt-write-iasyncinfo
            if (mapped->type().TypeName() == "IAsyncInfo") return false;
        }
        return true;
    }

    static bool can_write(writer& w, TypeDef const& type)
    {
        return can_write(w, &w.cache->find(type.TypeNamespace(), type.TypeName()));
    }

    static bool can_write(generic_inst const& type)
    {
        // TODO: WIN-275: Code generation for nested generics
        for (auto genarg : type.generic_params())
        {
            if (is_generic_inst(genarg))
            {
                return false;
            }
        }

        auto name = type.generic_type_abi_name();
        return name == "IReference" || name == "IEventHandler" || name == "ITypedEventHandler"
            || name == "IVector" || name == "IVectorView" || name == "IMap" || name == "IMapView";
    }

    static bool can_write(writer& w, function_def const& function, bool allow_special = false)
    {
        auto method = function.def;

        // Don't support writing specials (events/properties) unless told to do so (i.e. for vtable)
        if (method.SpecialName() && !allow_special) return false;

        auto method_name = get_swift_name(method);

        // TODO: WIN-30 swiftwinrt: support async/await
        if (function.is_async()) return false;
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
                // TODO: WIN-274 Code generation for IIterable/IIterator
                // TODO: WIN-124 Code generation for IObservableVector and IObservableMap
                auto name = iface.second.type->swift_full_name();
                if (name.starts_with("Windows.Foundation.Collections.IIterable")
                    || name.starts_with("Windows.Foundation.Collections.IIterator")
                    || name.starts_with("Windows.Foundation.Collections.IObservableVector")
                    || name.starts_with("Windows.Foundation.Collections.IObservableMap"))
                {
                    return false;
                }

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

        // TODO: WIN-65 swiftwinrt: support generic types
        auto generics = type.type().GenericParam();
        if (!empty(generics))
        {
            // don't write abi types bc that will put it in a UnsafeMutablePointer<>
            auto non_mangled = w.push_mangled_names(false);
            auto written = w.write_temp("%", type);
            auto can_write = !written.empty();
            if (!can_write)
            {
                return false;
            }
        }

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