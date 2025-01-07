#pragma once
#include "../helpers.h"
#include "../type_writers.h"
#include "type_writers.h"

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
    inline void write_generic_impl_name_base(writer& w, T const& type)
    {
        w.add_depends(type);
        std::string implName = w.write_temp("%", bind_type_mangled(type));
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

    template<typename T>
    inline void write_generic_bridge_name(writer& w, T const& type)
    {
        write_generic_impl_name_base(w, type);
        w.write("Bridge");
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
            write_generic_impl_name_base(w, type);
            w.write("Impl");
        }
    }

    template<typename T>
    inline void write_impl_name_base(writer& w, T const& type)
    {
        w.add_depends(type);
        type_name type_name{ type };
        std::string implName = w.write_temp("%", type_name.name);

        if (w.type_namespace != type_name.name_space || w.mangled_names || w.full_type_names)
        {
            w.write("%.%", impl_namespace(type_name.name_space), type_name.name);
        }
        else if (w.impl_names)
        {
            w.write(type_name.name);
        }
        else
        {
            w.write("%.%", impl_namespace(type_name.name_space), type_name.name);
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
            write_impl_name_base(w, type);
            w.write("Impl");
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
    inline void write_bridge_name(writer& w, T const& type)
    {
        if (is_generic_inst(type))
        {
            write_generic_bridge_name(w, type);
        }
        else
        {
            assert(!is_generic_def(type));

            write_impl_name_base(w, type);
            w.write("Bridge");
        }
    }

    template <typename T>
    auto bind_bridge_name(T const& type)
    {
        return [&](writer& w)
        {
            write_bridge_name(w, type);
        };
    }

    template <typename T>
    auto bind_bridge_fullname(T const& type)
    {
        return [&](writer& w)
        {
            auto full_name = w.push_full_type_names(true);
            write_bridge_name(w, type);
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

    static void write_documentation_comment(writer& w, const typedef_base& type, std::string_view member_name = {})
    {
        // Assume only public types have documentation
        if (type.type().Flags().Visibility() != TypeVisibility::Public)
        {
            return;
        }

        std::string doc_url;
        auto type_namespace = type.type().TypeNamespace();
        if (type_namespace.starts_with("Windows"))
        {
            doc_url = "https://learn.microsoft.com/uwp/api/";
        }
        else if (type_namespace.starts_with("Microsoft.UI") || type_namespace.starts_with("Microsoft.Windows") || type_namespace.starts_with("Microsoft.Graphics"))
        {
            doc_url = "https://learn.microsoft.com/windows/windows-app-sdk/api/winrt/";
        }
        else
        {
            return;
        }

        // Documentation URLs use "-" as the generic arity separator
        std::string type_name{ type.type().TypeName() };
        std::replace(begin(type_name), end(type_name), '`', '-');

        doc_url += type_namespace;
        doc_url += ".";
        doc_url += type_name;
        if (!member_name.empty())
        {
            doc_url += ".";
            doc_url += member_name;
        }

        // Documentation URLs are lower case
        std::transform(doc_url.begin(), doc_url.end(), doc_url.begin(),
            [](unsigned char c){ return std::tolower(c); });

        w.write("/// [Open Microsoft documentation](%)\n", doc_url);
    }

    static void write_convert_array_from_abi(writer& w, metadata_type const& type, std::string_view const& array_param_name)
    {
        if (is_reference_type(&type))
        {
            w.write(".from(abiBridge: %.self, abi: %)",
                bind_bridge_fullname(type),
                array_param_name);
        }
        else
        {
            w.write(".from(abi: %)",
                array_param_name);
        }
    }

    static void write_consume_type(writer& w, metadata_type const* type, std::string_view const& name, bool isOut)
    {
        TypeDef signature_type{};
        auto category = get_category(type, &signature_type);

        if (needs_wrapper(category))
        {
            auto ptrVal = isOut ? std::string(name) : w.write_temp("ComPtr(%)", name);
            if (is_class(type))
            {
                w.write("%.from(abi: %)", bind_bridge_fullname(*type), ptrVal);
            }
            else
            {
                w.write("%.unwrapFrom(abi: %)", bind_wrapper_fullname(type), ptrVal);
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
        else
        {
            auto format = ".init(from: %)";
            w.write(format, name);
        }
    }

    inline void write_generic_typealiases(writer& w, metadata_type const& type)
    {
        std::vector<named_interface_info> required_interfaces;
        bool is_public_alias = true;
        if (auto iface = dynamic_cast<const interface_type*>(&type))
        {
            required_interfaces = iface->required_interfaces;
        }
        else if (auto classType = dynamic_cast<const class_type*>(&type))
        {
            required_interfaces = classType->required_interfaces;
        }
        else if (auto genericInst = dynamic_cast<const generic_inst*>(&type))
        {
            is_public_alias = false;
            required_interfaces = genericInst->required_interfaces;
            required_interfaces.emplace_back(type.swift_type_name(), interface_info{ &type });
        }

        // required_interfaces will duplicate the generic type requirements for interfaces
        // which derive from others. For example, if we're writing IPropertySet, then required_interfaces
        // will contain IObservableMap<String, Any?>, IMap<String, Any?>, and IIterable<IKeyValuePair<String, Any?>>
        // The typealiases for IObservableMap<String, Any?> and IMap<String, Any?> will be identical, so only
        // write them once
        std::set<std::string> typealiases;

        for (const auto& [_name, info] : required_interfaces)
        {
            if (auto iface = dynamic_cast<const generic_inst*>(info.type))
            {
                auto genericType = dynamic_cast<const interface_type*>(iface->generic_type());
                auto&& generic_params = iface->generic_params();

                for (size_t i = 0; i < generic_params.size(); i++)
                {
                    auto [str, added] = typealiases.insert(w.write_temp("%typealias % = %\n",
                        is_public_alias ? "public ": "",
                        genericType->generic_params[i].swift_type_name(),
                        bind<write_type>(*generic_params[i], write_type_params::swift)));
                    if (added)
                    {
                        w.write(*str);
                    }
                }
            }
        }
    }
}