#pragma once
#include "../helpers.h"
#include "../type_writers.h"
namespace swiftwinrt
{
    template <typename T>
    inline void write_type_mangled(writer& w, T const& type)
    {
        auto push_mangled = w.push_mangled_names(true);
        w.write(type);
    }

    template <typename T>
    auto bind_type_mangled(T const& type)
    {
        return [&](writer& w)
        {
            write_type_mangled(w, type);
        };
    }

    template <typename T>
    inline void write_type_abi(writer& w, T const& type)
    {
        auto push_abi = w.push_abi_types(true);
        w.write(type);
    }

    template <typename T>
    auto bind_type_abi(T const& type)
    {
        return [&](writer& w)
        {
            write_type_abi(w, type);
        };
    }

    template<typename T>
    inline void write_generic_impl_name(writer& w, T const& type)
    {
        // for IReference<> types we use the same IPropertyValueImpl class that is
        // specially generated. this type can hold any value type and implements
        // the appropriate interface
        if (is_winrt_ireference(type))
        {
            w.write("%.%", impl_namespace("Windows.Foundation"), "IPropertyValueImpl");
        }
        else
        {
            std::string implName;
            {
                auto use_mangled = w.push_mangled_names(true);
                implName = w.write_temp("%Impl", type);
            }

            if (w.impl_names)
            {
                w.write(implName);
            }
            else
            {
                // generics are written once per module and aren't namespaced
                w.write("%.%", w.swift_module, implName);
            }
        }
    }

    template<typename T>
    inline void write_impl_name(writer& w, T const& type)
    {
        if (is_generic_inst(type))
        {
            write_generic_impl_name(w, type);
        }
        else
        {
            assert(!is_generic_def(type));

            type_name type_name{ type };
            std::string implName = w.write_temp("%Impl", type_name.name);

            if (w.type_namespace != type_name.name_space || w.mangled_names || w.full_type_names)
            {
                w.write("%.%", impl_namespace(type_name.name_space), implName);
            }
            else if (w.impl_names)
            {
                w.write(implName);
            }
            else
            {
                w.write("%.%", impl_namespace(type_name.name_space), implName);
            }
        }
    }

    template <typename T>
    auto bind_impl_name(T const& type)
    {
        return [&](writer& w)
        {
            write_impl_name(w, type);
        };
    }

    template <typename T>
    auto bind_impl_fullname(T const& type)
    {
        return [&](writer& w)
        {
            auto full_name = w.push_full_type_names(true);
            write_impl_name(w, type);
        };
    }

    template<typename T>
    inline void write_wrapper_name(writer& w, T const& type)
    {
        type_name type_name(type);

        if (is_generic_inst(type))
        {
            auto mangled_name = w.push_mangled_names(true);
            auto handlerWrapperTypeName = w.write_temp("%Wrapper", type);
            if (w.full_type_names)
            {
                // generics are written once per module and aren't namespaced
                w.write("%.%", w.swift_module, handlerWrapperTypeName);
            }
            else
            {
                w.write(handlerWrapperTypeName);
            }
        }
        else
        {
            assert(!is_generic_def(type));

            auto handlerWrapperTypeName = w.write_temp("%Wrapper", type_name.name);
            if (w.full_type_names)
            {
                w.write("%.%", abi_namespace(type_name.name_space), handlerWrapperTypeName);
            }
            else
            {
                w.write(handlerWrapperTypeName);
            }
        }

    }

    template <typename T>
    auto bind_wrapper_name(T const& type)
    {
        return [&](writer& w)
        {
            write_wrapper_name(w, type);
        };
    }

    template <typename T>
    auto bind_wrapper_fullname(T const& type)
    {
        return [&](writer& w)
        {
            auto full_type_names = w.push_full_type_names(true);
            write_wrapper_name(w, type);
        };
    }

    static void write_consume_type(writer& w, metadata_type const* type, std::string_view const& name)
    {
        TypeDef signature_type{};
        auto category = get_category(type, &signature_type);

        if (category == param_category::array_type)
        {
            // TODO: WIN-32 swiftwinrt: add support for array types
            XLANG_ASSERT("**TODO: implement array type in write_consume_return_type");
        }
        else if (category == param_category::object_type)
        {
            if (is_class(type))
            {
                w.write(".from(abi: %)", name);
            }
            else
            {
                w.write("%.unwrapFrom(abi: %)", bind_wrapper_fullname(type), name);
            }

        }
        else if (category == param_category::struct_type)
        {
            if (type->swift_type_name() == "EventRegistrationToken")
            {
                w.write(name);
            }
            else if (w.abi_types)
            {
                w.write(".from(swift: %)", name);
            }
            else
            {
                w.write(".from(abi: %)", name);
            }
        }
        else if (is_type_blittable(category))
        {
            // fundamental types can just be simply copied to since the types match
            w.write(name);
        }
        else if (w.abi_types && category == param_category::string_type)
        {
            auto format = "try! HString(%).detach()";
            w.write(format, name);
        }
        else if (category == param_category::generic_type)
        {
            if (is_winrt_ireference(type))
            {
                w.write(".init(ref: %)", name);
            }
            else
            {
                w.write("%.unwrapFrom(abi: %)",
                    bind_wrapper_fullname(type),
                    name);
            }
        }
        else
        {
            auto format = ".init(from: %)";
            w.write(format, name);
        }
    }
}