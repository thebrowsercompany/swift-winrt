#pragma once
#include "metadata_cache.h"
namespace swiftwinrt
{
    struct finish_with
    {
        writer& w;
        void (*finisher)(writer&);

        finish_with(writer& w, void (*finisher)(writer&)) : w(w), finisher(finisher) {}
        finish_with(finish_with const&) = delete;
        void operator=(finish_with const&) = delete;

        ~finish_with() { finisher(w); }
    };

    static void write_nothing(writer&)
    {
    }

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
        // for reference types we use the same IPropertyValueImpl class that is
        // specially generated. this type can hold any value type and implements
        // the appropriate interface
        auto swift_name = w.write_temp("%", type);
        if (swift_name.find("?") != swift_name.npos)
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
                // generics aren't public and so we use the namespace of the writer
                w.write("%.%", impl_namespace(w.type_namespace), implName);
            }
        }
    }

    template<typename T>
    inline void write_impl_name(writer& w, T const& type)
    {
        if (is_generic(type))
        {
            write_generic_impl_name(w, type);
        }
        else
        {
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

        if (is_generic(type))
        {
            auto mangled_name = w.push_mangled_names(true);
            auto handlerWrapperTypeName = w.write_temp("%Wrapper", type);
            if (w.full_type_names)
            {
                w.write("%.%", abi_namespace(w.type_namespace), handlerWrapperTypeName);
            }
            else
            {
                w.write(handlerWrapperTypeName);
            }
        }
        else
        {
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

    static bool can_write(writer& w, typedef_base const& type);

    static void write_enum_def(writer& w, enum_type const& type)
    {
        // Async status is defined in it's own header with a nice name of AsyncStatus.
        // Metadata attributes don't have backing code
        if (get_full_type_name(type) == "Windows.Foundation.AsyncStatus" ||
            type.swift_logical_namespace() == "Windows.Foundation.Metadata" || !can_write(w, type))
        {
            return;
        }
        w.write("public typealias % = %\n", type, bind_type_mangled(type));
    }

    static void write_enum_extension(writer& w, enum_type const& type)
    {
        if (get_full_type_name(type) == "Windows.Foundation.AsyncStatus" ||
            type.swift_logical_namespace() == "Windows.Foundation.Metadata" || !can_write(w, type))
        {
            return;
        }
        w.write("extension % {\n", get_full_swift_type_name(w, type));
        {
            auto format = R"(    public static var % : % {
        %_%
    }
)";
            for (auto&& field : type.type().FieldList())
            {
                if (field.Constant())
                {
                    // this enum is not written by our ABI tool, so it doesn't use a mangled name
                    if (get_full_type_name(type) == "Windows.Foundation.Collections.CollectionChange")
                    {
                        w.write(format, get_swift_name(field), get_full_swift_type_name(w, type), type, field.Name());
                    }
                    else
                    {
                        // we use mangled names for enums because otherwise the WinAppSDK enums collide with the Windows ones
                        w.write(format, get_swift_name(field), get_full_swift_type_name(w, type), bind_type_mangled(type), field.Name());
                    }
                }
            }
        }
        w.write("}\n\n");
    }

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
        for (auto& param: function.params)
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

    template <typename T>
    static bool can_write_default(const T* type)
    {
        bool defaultWritten = true;
        for (auto&& iface : type->required_interfaces)
        {
            // when getting the interfaces we populate them with the type name
            // if we can't write the type name then we can't write the type and
            // so we'll return false
            if (iface.second.is_default && defaultWritten)
            {
                defaultWritten = !iface.first.empty();
            }
        }

        return defaultWritten;
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
            if (!can_write_default(iface))
            {
                return false;
            }
        }
        else if (auto classType = dynamic_cast<const class_type*>(&type))
        {
            if (!can_write_default(classType))
            {
                return false;
            }
        }

        return true;
    }

    static void write_guid_value(writer& w, std::vector<FixedArgSig> const& args)
    {
        using std::get;

        w.write_printf("Data1: 0x%08X, Data2: 0x%04X, Data3: 0x%04X, Data4: ( 0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X )",
            get<uint32_t>(get<ElemSig>(args[0].value).value),
            get<uint16_t>(get<ElemSig>(args[1].value).value),
            get<uint16_t>(get<ElemSig>(args[2].value).value),
            get<uint8_t>(get<ElemSig>(args[3].value).value),
            get<uint8_t>(get<ElemSig>(args[4].value).value),
            get<uint8_t>(get<ElemSig>(args[5].value).value),
            get<uint8_t>(get<ElemSig>(args[6].value).value),
            get<uint8_t>(get<ElemSig>(args[7].value).value),
            get<uint8_t>(get<ElemSig>(args[8].value).value),
            get<uint8_t>(get<ElemSig>(args[9].value).value),
            get<uint8_t>(get<ElemSig>(args[10].value).value));
    }

    static void write_guid_comment(writer& w, std::vector<FixedArgSig> const& args)
    {
        using std::get;

        w.write_printf("%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
            get<uint32_t>(get<ElemSig>(args[0].value).value),
            get<uint16_t>(get<ElemSig>(args[1].value).value),
            get<uint16_t>(get<ElemSig>(args[2].value).value),
            get<uint8_t>(get<ElemSig>(args[3].value).value),
            get<uint8_t>(get<ElemSig>(args[4].value).value),
            get<uint8_t>(get<ElemSig>(args[5].value).value),
            get<uint8_t>(get<ElemSig>(args[6].value).value),
            get<uint8_t>(get<ElemSig>(args[7].value).value),
            get<uint8_t>(get<ElemSig>(args[8].value).value),
            get<uint8_t>(get<ElemSig>(args[9].value).value),
            get<uint8_t>(get<ElemSig>(args[10].value).value));
    }

    static void write_guid_value_hash(writer& w, std::array<uint8_t, 20ui64> const& iidHash)
    {
        w.write_printf("Data1: 0x%02x%02x%02x%02x, Data2: 0x%02x%02x, Data3: 0x%02x%02x, Data4: ( 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x )",
            iidHash[0], iidHash[1], iidHash[2], iidHash[3],
            iidHash[4], iidHash[5],
            iidHash[6], iidHash[7],
            iidHash[8], iidHash[9],
            iidHash[10], iidHash[11], iidHash[12], iidHash[13], iidHash[14], iidHash[15]);
    }

    static void write_guid_comment_hash(writer& w, std::array<uint8_t, 20ui64> const& iidHash)
    {
        w.write_printf("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            iidHash[0], iidHash[1], iidHash[2], iidHash[3],
            iidHash[4], iidHash[5],
            iidHash[6], iidHash[7],
            iidHash[8], iidHash[9],
            iidHash[10], iidHash[11], iidHash[12], iidHash[13], iidHash[14], iidHash[15]);
    }

    static void write_guid(writer& w, typedef_base const& type)
    {
        auto attribute = get_attribute(type.type(), "Windows.Foundation.Metadata", "GuidAttribute");

        if (!attribute)
        {
            throw_invalid("'Windows.Foundation.Metadata.GuidAttribute' attribute for type '", type.swift_full_name(), "' not found");
        }

        auto abi_guard = w.push_mangled_names(true);

        auto guid = attribute.Value().FixedArgs();
        auto format = R"(private var IID_%: IID {
    IID(%)// %
}

)";

        w.write(format,
            type,
            bind<write_guid_value>(guid),
            bind<write_guid_comment>(guid));
    }


    static void write_guid_generic(writer& w, generic_inst const& type)
    {
        auto abi_guard = w.push_mangled_names(true);

        static constexpr std::uint8_t namespaceGuidBytes[] =
        {
            0x11, 0xf4, 0x7a, 0xd5,
            0x7b, 0x73,
            0x42, 0xc0,
            0xab, 0xae, 0x87, 0x8b, 0x1e, 0x16, 0xad, 0xee
        };
        sha1 signatureHash;
        signatureHash.append(namespaceGuidBytes, std::size(namespaceGuidBytes));
        type.append_signature(signatureHash);
        auto iidHash = signatureHash.finalize();
        iidHash[6] = (iidHash[6] & 0x0F) | 0x50;
        iidHash[8] = (iidHash[8] & 0x3F) | 0x80;
        auto format = R"(private var IID_%: IID {
    IID(%)// %
}

)";

        w.write(format,
            type.mangled_name(),
            bind<write_guid_value_hash>(iidHash),
            bind<write_guid_comment_hash>(iidHash));
    }

    static void write_array_size_name(writer& w, Param const& param)
    {
        w.write(" __%Size", get_swift_name(param));
    }

    static void write_type_expression(writer& w, metadata_type const& type, bool abi);
    
    static void write_type_expression(writer& w, element_type const& type, bool abi)
    {
        if (abi)
        {
            if (elem_type->type() == ElementType::Object)
            {
                w.write("%.IInspectable", w.c_mod);
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
                w.write_abi(elem_type->type());
            }
        }
        else
        {
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
    }

    static void write_type_expression(writer& w, typedef_base const& type,
        const generic_inst* opt_generic_inst, bool abi)
    {
        bool is_interface = dynamic_cast<const interface_type*>(type) != nullptr;
        bool is_reference_type = is_interface || dynamic_cast<const class_type*>(type) != nullptr;

        if (abi)
        {
            if (is_reference_type)
            {
                auto abi_guard = w.push_abi();
                auto guard{ push_mangled_names_if_needed(get_category(type)) };
                if (is_reference_type) w.write("UnsafeMutablePointer<%>?", type);
                else w.write(type);
            }
            else
            {
                assert(!"Not implemented");
            }
        }
        else
        {
            if (is_reference_type) w.write("("); // Project as optional
            if (is_interface) w.write("any "); // Project as existential
            
            // Namespace
            if (opt_generic_inst)
            {
                writer.write("%.", writer.support);
            }
            else if (type->swift_logical_namespace() != writer.type_namespace)
            {
                writer.write("%.", type->swift_logical_namespace());
            }

            w.write(type);
            
            // Generic type arguments
            if (opt_generic_inst)
            {
                w.write("<");
                separator sep{ w };
                for (const auto& gen_arg : opt_generic_inst->generic_params())
                {
                    sep();
                    write_type_expression(w, gen_arg, abi);
                }
                w.write(">");
            }

            if (is_reference_type) w.write(")?"); // Project as optional
        }
    }

    static void write_type_expression(writer& w, metadata_type const& type, bool abi)
    {
        if (auto elem_type = dynamic_cast<const element_type*>(&type))
        {
            write_type_expression(w, *elem_type, abi);
        }
        else if (auto mapped = dynamic_cast<const mapped_type*>(&type))
        {
            // mapped types are defined in headers and *not* metadata files, so these don't follow the same
            // naming conventions that other types do. We just grab the type name and will use that.
            auto swift_name = mapped->swift_type_name();
            w.write(swift_name == "HResult" ? "HRESULT" : swift_name);
        }
        else if (auto type_def = dynamic_cast<const typedef_base*>(&type))
        {
            assert(!is_generic(type));
            write_type_expression(w, type_def, /* opt_generic_params: */ nullptr, layer);
        }
        else if (auto geninst = dynamic_cast<const generic_inst*>(&type))
        {
            auto gentype = *geninst->generic_type();
            const auto& genparams = geninst->generic_params();
            
            // Special generic types
            if (gentype.swift_full_name() == "Windows.Foundation.IReference`1")
            {
                auto boxed_type = genparams[0];
                write("%?", bind<write_type_expression>(w, boxed_type, layer));
            }
            else if (gentype.swift_full_name() == "Windows.Foundation.TypedEventHandler`2")
            {
                auto sender_type = genparams[0];
                auto args_type = genparams[1];
                write("^@escaping (%,%) -> ()",
                    bind<write_type_expression>(w, sender_type, layer),
                    bind<write_type_expression>(w, args_type, layer));
            }
            else if (gentype.swift_full_name() == "Windows.Foundation.EventHandler`1")
            {
                auto args_type = genparams[0];
                write("^@escaping (%.IInspectable,%) -> ()",
                    support, bind<write_type_expression>(w, args_type, layer));
            }
            else
            {
                // Collections and other generic types
                write_type_expression(w, gentype, &genparams, layer);
            }
        }
    }

    static void write_function_param(writer& w, function_param const& param, ProjectionLayer layer)
    {
        w.write("_ %: ", get_swift_name(param));
        if (param.out) w.write("inout ");
        write_type_expression(w, *param.type, layer);
    }

    static void write_function_params(writer& w, function_def const& function)
    {
        separator s{ w };

        for (auto&& param : function.params)
        {
            s();
            write_function_param(param);
        }
    }

    static void write_convert_to_abi_arg(writer& w, std::string_view const& param_name, const metadata_type* type, bool is_out)
    {
        TypeDef signature_type;
        auto category = get_category(type, &signature_type);

        if (category == param_category::object_type)
        {
            if (is_interface(signature_type) || is_delegate(signature_type))
            {
                w.write("_%", param_name);
            }
            else
            {
                w.write("RawPointer(%)", param_name);
            }
        }
        else if (category == param_category::string_type)
        {
            if (!is_out)
            {
                w.write("_%.get()", param_name);
            }
            else
            {
                auto format = "try! HString(%).detach()";
                w.write(format, param_name);
            }
        }
        else if (category == param_category::struct_type)
        {
            if (is_struct_blittable(signature_type))
            {
                w.write(".from(swift: %)", param_name);
            }
            else
            {
                if (!is_out)
                {
                    w.write("_%.val", param_name);
                }
                else
                {
                    w.write("_%.detach()", param_name);
                }
            }
        }
        else if(category == param_category::generic_type)
        {
            // When passing generics to the ABI we wrap them before making the
            // api call for easy passing to the ABI
            w.write("_%", param_name);
        }
        else if (is_type_blittable(category))
        {
            // fundamentals and enums can be simply copied
            w.write(param_name);
        }
        else
        {
            w.write(".init(from: %)", param_name);
        }
    }

    static void write_implementation_args(writer& w, function_def const& function)
    {
        separator s{ w };
        for (auto& param: function.params)
        {
            s();
            if (param.in())
            {
                write_convert_to_abi_arg(w, get_swift_name(param), param.type, false);
            }
            else
            {
                auto category = get_category(param.type);
                bool is_blittable = is_type_blittable(param.signature.Type());
                if (category == param_category::struct_type)
                {
                    if (is_blittable)
                    {
                        w.write("&_%", get_swift_name(param));
                    }
                    else
                    {
                        w.write("&_%.val", get_swift_name(param));
                    }
                }
                else if (is_blittable)
                {
                    w.write("&%", get_swift_name(param));
                }
                else
                {
                    w.write("&_%", get_swift_name(param));
                }
            }
        }
    }

    static void write_abi_args(writer& w, function_def const& function)
    {
        separator s{ w };

        w.write("pThis");

        s();
        for (auto&& param : function.params)
        {
            s();
            if (param.def.Flags().In())
            {
                w.write(get_swift_name(param));
            }
            else
            {
                w.write("&%", get_swift_name(param));
            }
        }

        if (function.return_type)
        {
            s();
            auto param_name = function.return_type.value().name;
            w.write("&%", param_name);
        }
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
            if (is_interface(type))
            {
                w.write("%.unwrap_from(abi: %)",
                    bind_wrapper_fullname(type),
                    name);
            }
            else if (is_delegate(type))
            {
                if (w.consume_types)
                {
                    w.write("_%!", name);
                }
                else
                {
                    w.write("%.try_unwrap_from(abi: %)", bind_wrapper_name(type), name);
                }
            }
            else if (is_class(type))
            {
                auto format = ".from(abi: %)";
                w.write(format, name);
            }
            else
            {
                //TODO: implement generic object type
                w.write(".init(%)",
                    name);
            }

        }
        else if (category == param_category::struct_type)
        {
            if (w.abi_types)
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
            auto swift_name = w.write_temp("%", type);
            if (swift_name.find("?") != swift_name.npos)
            {
                w.write(".init(ref: %)", name);
            }
            else
            {
                w.write("%.unwrap_from(abi: %)",
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

    static void write_default_init_assignment(writer& w, metadata_type const* sig)
    {
        auto category = get_category(sig);
     
        if (category == param_category::object_type || category == param_category::generic_type)
        {
            // Projected to Optional and default-initialized to nil
        }
        else if (category == param_category::string_type)
        {
            w.write(" = \"\"");
        }
        else if (category == param_category::struct_type || is_guid(category))
        {
            w.write(" = .init()");
        }
        else if (category == param_category::enum_type)
        {
            w.write(" = .init(0)");
        }
        else if (is_boolean(sig))
        {
            w.write(" = %", w.abi_types ? "0" : "false");
        }
        else
        {
            w.write(" = %", is_floating_point(sig) ? "0.0" : "0");
        }
    }


    static void write_init_return_val_abi(writer& w, function_return_type const& signature)
    {
        auto category = get_category(signature.type);
        auto guard{ w.push_mangled_names_if_needed(category) };
        write_type_expression(w, *signature.type, ProjectionLayer::ABI);
        write_default_init_assignment(w, signature.type);
    }

    static void write_consume_return_statement(writer& w, function_def const& signature);
    static write_scope_guard<writer> write_local_param_wrappers(writer& w, function_def const& signature);

    static void write_abi_func_body(writer& w, typedef_base const& type, function_def const& function)
    {
        std::string_view func_name = get_abi_name(function);
        if (function.return_type)
        {
            auto return_val = function.return_type.value();
            w.write("var %: %\n",
                return_val.name,
                bind<write_init_return_val_abi>(return_val));
        }

        w.write(R"(_ = try perform(as: %.self) { pThis in
    try CHECKED(pThis.pointee.lpVtbl.pointee.%(%))
)",
bind_type_mangled(type),
func_name,
bind<write_abi_args>(function));

        w.write("}\n");
        if (function.return_type)
        {
            w.write("return %\n", function.return_type.value().name);
        }
    }

    static void write_return_type_declaration(writer& w, function_def function)
    {
        if (!function.return_type)
        {
            return;
        }

        w.write(" -> ");
        write_type_expression(w, function.return_type.value(), ProjectionLayer::ABI);
    }

    static void do_write_interface_abi(writer& w, typedef_base const& type, std::vector<function_def> const& methods)
    {
        auto factory_info = try_get_factory_info(w, type);
        const bool is_composable_factory = factory_info.has_value() && factory_info.value().composable;

        if (is_exclusive(type) && !can_write(w, get_exclusive_to(type)))
        {
            return;
        }
        const bool internal = is_composable_factory || can_mark_internal(type.type());
        w.write("% class %: %.IInspectable% {\n",
            internal ? "internal" : "open",
            type,
            w.support,
            is_composable_factory ? ", ComposableActivationFactory": "");

        auto class_indent_guard = w.push_indent();

        if (is_composable_factory)
        {
            auto overrides_format = "internal typealias Composable = %.Composable\n\n";
            w.write(overrides_format, get_full_swift_type_name(w, get_exclusive_to(type)));
        }
      
        auto abi_guard = w.push_abi_types(true);

        auto iid_format = "override public class var IID: IID { IID_% }\n\n";
        w.write(iid_format, bind_type_mangled(type));

        for (auto&& function : methods)
        {
            if (!can_write(w, function, true)) continue;
            try
            {
                // Composable factories have to have the method be called CreateInstanceImpl, even if the
                // metadata format specifies something else.
                auto func_name = is_composable_factory ? "CreateInstance" : get_abi_name(function);
                auto abi_guard2 = w.push_abi_types(true);
                w.write("% func %Impl(%) throws %{\n",
                    internal || is_exclusive(type) ? "internal" : "open",
                    func_name,
                    bind<write_params>(function),
                    bind<write_return_type_declaration>(function));
                {
                    auto function_indent_guard = w.push_indent();
                    write_abi_func_body(w, type, function);
                }
                w.write("}\n\n");
            }
            catch (std::exception const& e)
            {
                throw_invalid(e.what(),
                    "\n method: ", get_name(function),
                    "\n type: ", get_full_type_name(type),
                    "\n database: ", type.type().get_database().path());
            }
        }

        class_indent_guard.end();
        w.write("}\n\n");
    }

    static void write_implementable_interface(writer& w, interface_type const& type);

    static void write_interface_generic(writer& w, generic_inst const& type)
    {
        if (can_write(type))
        {
            type.write_swift_declaration(w);

            if (is_collection_type(type))
            {
                auto generic_params = w.push_generic_params(type);
                do_write_interface_abi(w, *type.generic_type(), type.functions);
            }
        }
    }

    static void write_ireference_init_extension(writer& w, generic_inst const& type)
    {
        if (!type.swift_full_name().starts_with("Windows.Foundation.IReference")) return;

        auto format = R"(fileprivate extension % {
    init?(ref: UnsafeMutablePointer<%>?) {
        guard let val = ref else { return nil }
        var result: % = .init()
        try! CHECKED(val.pointee.lpVtbl.pointee.get_Value(val, &result))
        %
    }
} 
)";
        auto generic_param = type.generic_params()[0];

        if (auto structType = dynamic_cast<const struct_type*>(generic_param))
        {
            w.write(format,
                get_full_swift_type_name(w, structType),
                type.mangled_name(),
                structType->mangled_name(),
                "self = .from(abi: result)");
        }
        else
        {
            bool blittable = true;
            if (auto elementType = dynamic_cast<const element_type*>(generic_param))
            {
                blittable = elementType->is_blittable();
            }
            w.write(format,
                get_full_swift_type_name(w, generic_param),
                type.mangled_name(),
                generic_param->cpp_abi_name(),
                blittable ? "self = result" : "self.init(from: result)");
        }
      
    }

    static void write_interface_abi(writer& w, interface_type const& type)
    {
        if (!can_write(w, type)) return;
   
        do_write_interface_abi(w, type, type.functions);
        if (!is_exclusive(type))
        {
            write_implementable_interface(w, type);
        }
    }

    static void write_vtable(writer& w, interface_type const& type);
    static void write_vtable(writer& w, delegate_type const& type);

    template <typename T>
    static void write_delegate_wrapper(writer& w, T const& type)
    {
        auto impl_name = w.write_temp("%", bind_impl_fullname(type));

        auto delegate_abi_name = w.write_temp("%", bind_type_mangled(type));
        auto wrapper_name = w.write_temp("%", bind_wrapper_name(type));

        auto format = R"(
class % : WinRTWrapperBase<%, %> {
    override class var IID: IID { IID_% }
    init(_ handler: %){
        let abi = withUnsafeMutablePointer(to: &%VTable) {
            %(lpVtbl:$0)
        }
        super.init(abi, handler)
    }
}
)";
        // for generic instantiations, the type name we use for the vtable is the mangled name,
        // so push the abi types
        constexpr bool is_generic = std::is_same_v<T, generic_inst>;
        auto abi_guard = w.push_abi_types(is_generic);
        w.write(format,
            wrapper_name, delegate_abi_name, impl_name,
            delegate_abi_name,
            impl_name,
            is_generic ? delegate_abi_name : w.write_temp("%", type),
            delegate_abi_name);
    }

    static void write_generic_delegate_wrapper(writer& w, generic_inst const& generic)
    {
        if (can_write(generic))
        {
            write_delegate_wrapper(w, generic);
        }
    }
    
    static void write_delegate_abi(writer& w, delegate_type const& type)
    {
        write_vtable(w, type);
        write_delegate_wrapper(w, type);
    }
    
    static void write_struct_abi(writer& w, struct_type const& type)
    {
        bool is_blittable = is_struct_blittable(type);
        if (is_blittable)
        {
            return;
        }

        // for non blittable structs we need to create a type which helps us convert between
        // swift and C land
        w.write("public class _ABI_% {\n", type.swift_type_name());
        {
            auto class_indent_guard = w.push_indent();
            w.write("public var val: % = .init()\n", bind_type_mangled(type));
            w.write("public init() { }\n");

            w.write("public init(from swift: %) {\n", get_full_swift_type_name(w, type));
            {
                auto push_abi = w.push_abi_types(true);
                auto indent = w.push_indent();
                for (auto&& field : type.members)
                {
                    // WIN-64 - swiftwinrt: support boxing/unboxing
                    // WIN-65 - swiftwinrt: support generic types
                    if (can_write(w, field.type))
                    {
                        std::string from = std::string("swift.").append(get_swift_name(field));
                        w.write("val.% = %\n",
                            get_swift_name(field),
                            bind<write_consume_type>(field.type, from)
                        );
                    }

                }
            }
            w.write("}\n\n");

            w.write("public func detach() -> % {\n", bind_type_mangled(type));
            {
                auto indent = w.push_indent();

                w.write("let result = val\n");
                for (auto&& member : type.members)
                {
                    auto field = member.field;
                    if (get_category(member.type) == param_category::string_type)
                    {
                        w.write("val.% = nil\n", get_swift_name(member));
                    }
                }
                w.write("return result\n");
            }
            w.write("}\n\n");

            w.write("deinit {\n");
            {
                auto indent = w.push_indent();
                for (auto&& member : type.members)
                {
                    if (get_category(member.type) == param_category::string_type)
                    {
                        w.write("WindowsDeleteString(val.%)\n", get_swift_name(member));
                    }
                }
            }
            w.write("}\n");
        }
        w.write("}\n");
    }

    static void write_consume_params(writer& w, function_def const& signature)
    {
        int param_number = 1;
        auto full_type_names = w.push_full_type_names(true);

        for (auto& param : signature.params)
        {
            if (param.signature.Type().is_szarray())
            {
                // TODO: WIN-32 swiftwinrt: add support for arrays
                w.write("**TODO: implement szarray in write_consume_params**");
            }
            else
            {
                std::string param_name = "$" + std::to_string(param_number);

                if (param.in())
                {
                    assert(!param.out());

                    bool isDelegate = is_delegate(param.type);
                    std::string format = "%let %: % = %%\n";
                    w.write(format,
                        isDelegate ? "guard " : "",
                        get_swift_name(param),
                        param.type,
                        bind<write_consume_type>(param.type, param_name),
                        isDelegate ? " else { return E_INVALIDARG }" : ""
                    );
                }
                else
                {
                    assert(!param.in());
                    assert(param.out());
                    w.write("var %: %\n",
                        get_swift_name(param), 
                        bind<write_init_val>(param.type));
                }
            }
            ++param_number;
        }
    }

    static void write_consume_return_statement(writer& w, function_def const& signature)
    {
        if (!signature.return_type)
        {
            return;
        }

        auto return_type = signature.return_type.value().type;
        auto consume_types = w.push_consume_types(true);
        TypeDef signature_type{};
        auto return_param_name = signature.return_type.value().name;
        if (is_delegate(return_type))
        {
            w.write("let _% = %.try_unwrap_from(abi: %)\n",
                return_param_name,
                bind_wrapper_fullname(return_type),
                return_param_name);
        }
     
        w.write("return %", bind<write_consume_type>(return_type, return_param_name));
    }

    static void write_consume_args(writer& w, function_def const& function)
    {
        separator s{ w };

        for (auto& param : function.params)
        {
            s();
            if (param.in())
            {
                w.write(get_swift_name(param));
            }
            else
            {

                
                w.write("&%", get_swift_name(param));
                
            }
        }
    }

    // PropertyValue is special, it's not an interface/prototype we expose to customers
    // and instead is the glue for boxing types which are of `Any` type in Swift
    static void write_property_value_wrapper(writer& w)
    {
        w.write(R"(public class IPropertyValueWrapper : WinRTWrapperBase<__x_ABI_CWindows_CFoundation_CIPropertyValue, %.IPropertyValue>
{
    override public class var IID: IID { IID___x_ABI_CWindows_CFoundation_CIPropertyValue }
    public init(_ value: Any) {
        let abi = withUnsafeMutablePointer(to: &IPropertyValueVTable) {
            __x_ABI_CWindows_CFoundation_CIPropertyValue(lpVtbl: $0)
        }
        super.init(abi, __IMPL_Windows_Foundation.IPropertyValueImpl(value: value))
    }

    public init?(_ impl: %.IPropertyValue?) {
        guard let impl = impl else { return nil }
        let abi = withUnsafeMutablePointer(to: &IPropertyValueVTable) {
            __x_ABI_CWindows_CFoundation_CIPropertyValue(lpVtbl: $0)
        }
        super.init(abi, impl)
    }
}
)", w.support, w.support);
}

    static void write_implementable_interface(writer& w, interface_type const& type)
    {
        write_vtable(w, type);

        if (get_full_type_name(type) == "Windows.Foundation.IPropertyValue")
        {
            write_property_value_wrapper(w);
            return;
        }
        // Define a struct that matches a C++ object with a vtable. As background:
        //
        // in C++ this looks like:
        //
        //   class CMyObject : public IMyInterface {
        //   {
        //     HRESULT Foo(int number);
        //   }
        //
        // in C it looks
        //
        //   struct __x_ABI_MyObjectVTable {
        //     ... AddRef, Release, QI, ...
        //     HRESULT (STDMETHODCALLTYPE * Foo)(__x_ABI_IMyInterface* pThis, int number)
        //   }
        // 
        //   struct __x_ABI_IMyInterface {
        //     const __x_ABI_MyObjectVTable* lpVtbl;
        //   }
        //
        // so in Swift we're using the pattern:
        //
        //   protocol IMyInterface
        //   {
        //      func Foo(number: Int32)
        //   }
        //
        //   private var myInterfaceVTable: __x_ABI_MyObjectVTable {
        //     Foo: {
        //       // In C, 'pThis' is always the first param
        //       guard let instance = MyInterfaceWrapper.try_unwrap_raw($0)?.takeUnretainedValue().swiftObj else {
        //         return E_INVALIDARG
        //       }
        //       let number = $1
        //       instance.Foo(number)
        //     }
        //   }
        //   ...
        //    class MyInterfaceWrapper : WinRTWrapperBase<__x_ABI_IMyInterface, IMyInterface> {
        //       init(impl: IMyInterface) {
        //         let abi = withUnsafeMutablePointer(to: &myInterfaceVTable) {
        //           __x_ABI_IMyInterface(lpVtbl: $0)
        //         }
        //       super.init(abi, impl)
        //    }
        // Where the WinRTWrapperBase defines the behavior for all objects which need to wrap a
        // swift object and pass it down to WinRT:
        // open class WinRTWrapperBase<CInterface, Prototype> {
        //    public struct ComObject {
        //      public var comInterface : CInterface
        //      public var wrapper : Unmanaged<WinRTWrapperBase>?
        //    }
        //    public var instance : ComObject
        //    public var swiftObj : Prototype
        //    ...
        // }

        w.write(R"(
public typealias % = InterfaceWrapperBase<%>
)",
bind_wrapper_name(type),
bind_impl_fullname(type));
    }

    static void write_class_impl_func(writer& w, function_def const& method, interface_info const& iface);
    static void write_class_impl_property(writer& w, property_def const& prop, interface_info const& iface);
    static void write_class_impl_event(writer& w, event_def const& event, interface_info const& iface);
    static void write_property_value_impl(writer& w)
    {
        w.write(R"(public class IPropertyValueImpl : IPropertyValue, IReference {
    var _value: Any
    var propertyType : PropertyType

    public init(value: Any) {
        _value = value
        if _value is Int32 {
            propertyType = .Int32
        } else if _value is UInt8 {
            propertyType = .UInt8
        } else if _value is Int16 {
            propertyType = .Int16
        } else if _value is UInt32 {
            propertyType = .UInt32
        } else if _value is Int64 {
            propertyType = .Int64
        } else if _value is UInt64 {
            propertyType = .UInt64
        } else if _value is Float {
            propertyType = .Single
        } else if _value is Double {
            propertyType = .Double
        } else if _value is Character {
            propertyType = .Char16
        } else if _value is Bool {
            propertyType = .Boolean
        } else if _value is DateTime {
            propertyType = .DateTime
        } else if _value is TimeSpan {
            propertyType = .TimeSpan
        } else if _value is IWinRTObject {
            propertyType = .Inspectable
        } else if _value is IInspectable {
            propertyType = .Inspectable
        } else {
            propertyType = .OtherType
        }
    }

    public var `Type`: PropertyType { propertyType }
    public var IsNumericScalar: Bool { 
        switch propertyType {
            case .Int16, .Int32, .UInt16, .UInt8, .Int64, .UInt64, .Single, .Double: return true
            default: return false
        }
    }

    public var Value: Any { _value }

    public func GetUInt8() -> UInt8 { _value as! UInt8 }
    public func GetInt16() -> Int16 { _value as! Int16 }
    public func GetUInt16() -> UInt16 { _value as! UInt16 }
    public func GetInt32() -> Int32 { _value as! Int32 }
    public func GetUInt32() -> UInt32 { _value as! UInt32 }
    public func GetInt64() -> Int64 { _value as! Int64 }
    public func GetUInt64() -> UInt64 { _value as! UInt64 }
    public func GetSingle() -> Float { _value as! Float }
    public func GetDouble() -> Double { _value as! Double }
    public func GetChar16() -> Character { _value as! Character }
    public func GetBoolean() -> Bool { _value as! Bool }
    public func GetString() -> String { _value as! String }
    public func GetGuid() -> UUID { _value as! UUID }
    public func GetDateTime() -> DateTime { _value as! DateTime } 
    public func GetTimeSpan() -> TimeSpan { _value as! TimeSpan }
    public func GetPoint() -> Point { _value as! Point }
    public func GetSize() -> Size { _value as! Size }
    public func GetRect() -> Rect { _value as! Rect }
}

)");
    }

    // Due to https://linear.app/the-browser-company/issue/WIN-148/investigate-possible-compiler-bug-crash-when-generating-collection
    // we have to generate the protocol conformance for the Collection protocol (see "// MARK: Collection" below). We shouldn't have to
    // do this because we define an extension on the protocol which does this.
    static void write_collection_protocol_conformance(writer& w, interface_info const& info)
    {
        auto modifier = w.impl_names ? "" : "public ";
        auto typeName = info.type->swift_type_name();
        if (typeName.starts_with("IVector"))
        {
            w.write(R"(// MARK: Collection
%var startIndex: Int { 0 }
%var endIndex: Int { Int(Size) }
%func index(after i: Int) -> Int {
    i+1
}

%func index(of: Element) -> Int? { 
    var index: UInt32 = 0
    let result = IndexOf(of, &index)
    guard result else { return nil }
    return Int(index)
}
%var count: Int { Int(Size) }
)", modifier, modifier, modifier, modifier, modifier);
            if (typeName.starts_with("IVectorView"))
            {
                w.write(R"(
%subscript(position: Int) -> Element {
    get {
        GetAt(UInt32(position))
    }
}
)", modifier);
            }
            else
            {
                w.write(R"(
%func append(_ item: Element) {
    Append(item)
}

%subscript(position: Int) -> Element {
    get {
        GetAt(UInt32(position))
    }
    set(newValue) {
        SetAt(UInt32(position), newValue)
    }
}

%func removeLast() {
    RemoveAtEnd()
}

%func clear() {
    Clear()
}
)", modifier, modifier, modifier, modifier);
            }
        }

        w.write("// MARK: WinRT\n");
    }

    static void write_interface_impl(writer& w, interface_type const& type)
    {
        if (is_exclusive(type) || !can_write(w, type)) return;

        if (get_full_type_name(type) == "Windows.Foundation.IPropertyValue")
        {
            write_property_value_impl(w);
            return;
        }

        auto format = "public class % : %, AbiInterfaceImpl {\n";
        w.write(format, bind_impl_name(type), type);

        auto class_indent_guard = w.push_indent();

        w.write(R"(public typealias c_ABI = %
public typealias swift_ABI = %.%
public typealias swift_Projection = %

private (set) public var _default: swift_ABI

public static func from(abi: UnsafeMutablePointer<c_ABI>?) -> swift_Projection {
    return %(abi)
}
public init(_ fromAbi: UnsafeMutablePointer<c_ABI>?) {
    _default = swift_ABI(fromAbi)
}

public static func makeAbi() -> c_ABI {
    let vtblPtr = withUnsafeMutablePointer(to: &%.%VTable) { $0 }
    return .init(lpVtbl: vtblPtr)
}
)",
            bind_type_mangled(type),
            abi_namespace(type),
            type,
            type,
            bind_impl_name(type),
            abi_namespace(type),
            type);

        interface_info type_info{ &type };
        type_info.is_default = true; // mark as default so we use the name "_default"
        for (auto&& method : type.functions)
        {
            write_class_impl_func(w, method, type_info);
        }
        for (auto&& prop : type.properties)
        {
            write_class_impl_property(w, prop, type_info);
        }

        for (auto&& [interface_name, info] : type.required_interfaces)
        {
            if (!can_write(w, info.type)) { continue; }

            if (!info.is_default || info.base)
            {
                w.write("internal lazy var %: % = try! _default.QueryInterface()\n",
                    get_swift_name(info),
                    bind<write_type_expression>(*info.type, ProjectionLayer::ABI));
            }

            if (auto iface = dynamic_cast<const interface_type*>(info.type))
            {
                for (auto&& method : iface->functions)
                {
                    write_class_impl_func(w, method, info);
                }

                for (auto&& prop : iface->properties)
                {
                    write_class_impl_property(w, prop, info);
                }
            }
        }

        class_indent_guard.end();
        w.write("}\n\n");
    }

    static void write_interface_proto(writer& w, interface_type const& type)
    {
        if (is_exclusive(type) || !can_write(w, type))
        {
            return;
        }
        auto format = R"(public protocol % : %% { %
}
)";
        auto typeName = type.swift_type_name();
        auto interfaces = type.required_interfaces;
        separator s{ w };
        auto implements = w.write_temp("%", bind_each([&](writer& w, std::pair<std::string, interface_info> const& iface) {
            // TODO: https://linear.app/the-browser-company/issue/WIN-103/swiftwinrt-write-iasyncinfo
            if (!iface.first.ends_with("IAsyncInfo") && can_write(w, iface.second.type))
            {
                s();
                w.write("%", iface.second.type);
            }}, interfaces));

        w.write(format, type, implements,
            implements.empty() ? "IWinRTObject" : "",
            bind([&](writer& w)
            {
                for (auto& method : type.functions)
                {
                    if (!can_write(w, method)) continue;

                    auto full_type_name = w.push_full_type_names(true);
                    w.write("\n        func %(%) %",
                        get_swift_name(method),
                        bind<write_params>(method),
                        bind<write_return_type_declaration>(method));
                }

                for (auto& prop : type.properties)
                {
                    if (!can_write(w, prop)) continue;
                    auto full_type_name = w.push_full_type_names(true);
                    w.write("\n        var %: % { get% }",
                        get_swift_name(prop),
                        prop.getter.value().return_type.value(),
                        prop.setter ? " set" : "");
                }
            }));

        // don't write this extension for property value since it isn't used publically
        if (get_full_type_name(type) == "Windows.Foundation.IPropertyValue")
        {
            return;
        }
        auto extension_format = R"(extension % {
    public static var none: % {
        %(nil)
    }
}

)";
        w.write(extension_format,
            type,
            type, 
            bind_impl_fullname(type));
    }

    static void write_ireference(writer& w)
    {
        w.write(R"(public protocol IReference : IPropertyValue {
    var Value: Any { get }
}
)");
    }

    static void write_delegate(writer& w, delegate_type const& type)
    {
        w.write("public typealias % = %\n", type, bind_impl_name(type));
    }

    static void write_delegate_return_type(writer& w, function_def const& sig)
    {
        if (sig.return_type)
        {
            w.write("%", sig.return_type.value().type);
        }
        else
        {
            w.write("()");
        }
    }

    static void write_comma_param_types(writer& w, std::vector<function_param> const& params)
    {
        separator s{ w };
        for (auto& param : params)
        {
            s();
            w.write(param.type);
        }
    }

    template <typename T>
    static void do_write_delegate_implementation(writer& w, T const& type, function_def const& invoke_method)
    {
        auto format = R"(% class % : WinRTDelegate {
    % typealias Data = (%)
    % typealias Return = %
    % var token: EventRegistrationToken?
    % var handler: (Data) -> Return
    % required init(handler: ^@escaping (Data) -> Return){
        self.handler = handler
    }
}
)";
        auto data = w.write_temp("%", bind<write_comma_param_types>(invoke_method.params));
        auto return_type = w.write_temp("%", bind<write_delegate_return_type>(invoke_method));
        constexpr bool is_generic = std::is_same_v<T, generic_inst>;
        auto access_level = is_generic ? "internal" : "public";
        auto abi_guard = w.push_abi_types(is_generic);
        w.write(format,
            access_level,
            bind_impl_name(type),
            access_level,
            data,
            access_level,
            return_type,
            access_level,
            access_level,
            access_level);
    }

    static void write_delegate_implementation(writer& w, delegate_type const& type)
    {
        if (can_write(w, type))
        {
            auto delegate_method = type.functions[0];
            do_write_delegate_implementation(w, type, delegate_method);
        }
    }

    static void write_collection_implementation(writer& w, generic_inst const& type)
    {
        // Due to https://linear.app/the-browser-company/issue/WIN-148/investigate-possible-compiler-bug-crash-when-generating-collection
        // we have to generate the protocol conformance for the Collection protocol (see "// MARK: Collection" below). We shouldn't have to
        // do this because we define an extension on the protocol which does this.
        w.write("internal class % : %, AbiInterfaceImpl {\n",
            bind_impl_name(type), type.generic_type_abi_name());

        auto indent_guard = w.push_indent();

        if (type.swift_type_name().starts_with("IVector")) // IVector and IVectorView
        {
            w.write("typealias Element = %\n", type.generic_params()[0]);
            w.write("typealias swift_Projection = any %<%>\n",
                type.generic_type_abi_name(), type.generic_params()[0]);
        }
        else if (type.swift_type_name().starts_with("IMap")) // IMap and IMapView
        {
            w.write("typealias Key = %\n", type.generic_params()[0]);
            w.write("typealias Value = %\n", type.generic_params()[1]);
            w.write("typealias swift_Projection = any %<%, %>\n",
                type.generic_type_abi_name(), type.generic_params()[0], type.generic_params()[1]);
        }
        else
        {
            assert(!"Unexpected collection type");
        }

        w.write("typealias c_ABI = %\n", bind_type_mangled(type));
        w.write("typealias swift_ABI = %.%\n", abi_namespace(w.type_namespace), bind_type_abi(type));
        w.write("\n");
        w.write("private (set) public var _default: swift_ABI\n");
        w.write("\n");

        w.write("static func from(abi: UnsafeMutablePointer<c_ABI>?) -> swift_Projection {\n");
        w.write("    return %(abi)\n", bind_impl_name(type));
        w.write("}\n\n");

        w.write("internal init(_ fromAbi: UnsafeMutablePointer<c_ABI>?) {\n");
        w.write("    _default = swift_ABI(fromAbi)\n");
        w.write("}\n\n");

        w.write("static func makeAbi() -> c_ABI {\n");
        w.write("    let vtblPtr = withUnsafeMutablePointer(to: &%.%VTable) { $0 }\n",
            abi_namespace(w.type_namespace), bind_type_mangled(type));
        w.write("    return.init(lpVtbl: vtblPtr)\n");
        w.write("}\n\n");
        
        interface_info info{ &type };
        info.is_default = true; // mark as default so we use the name "_default"
        write_collection_protocol_conformance(w, info);

        for (auto&& method : type.functions)
        {
            write_class_impl_func(w, method, info);
        }
        for (auto&& prop : type.properties)
        {
            write_class_impl_property(w, prop, info);
        }

        indent_guard.end();
        w.write("}\n\n");
    }

    static void write_generic_implementation(writer& w, generic_inst const& type)
    {
        if (!can_write(type)) return;

        auto generics_guard = w.push_generic_params(type);
        if (is_delegate(type))
        {
            auto delegate_method = type.functions[0];
            do_write_delegate_implementation(w, type, delegate_method);
        }
        else if (is_collection_type(type))
        {
            write_collection_implementation(w, type);
        }
    }

    // When converting from Swift <-> C we put some local variables on the stack in order to help facilitate
    // converting between the two worlds. This method will returns a scope guard which will write any necessary
    // code for after the ABI function is called (such as cleaning up references).
    static write_scope_guard<writer> write_local_param_wrappers(writer& w, function_def const& signature)
    {
        write_scope_guard guard{ w };

        for (auto& param : signature.params)
        {
            TypeDef signature_type{};
            auto category = get_category(param.type, &signature_type);

            if (param.in())
            {
                if (category == param_category::string_type)
                {
                    w.write("let _% = try! HString(%)\n",
                        get_swift_name(param),
                        get_swift_name(param));
                }
                else if (category == param_category::struct_type && !is_struct_blittable(signature_type))
                {
                    w.write("let _% = %._ABI_%(from: %)\n",
                        get_swift_name(param),
                        abi_namespace(signature_type),
                        signature_type.TypeName(),
                        get_swift_name(param));
                }
                else if (category == param_category::object_type)
                {
                    if (is_interface(param.type))
                    {
                        w.write("let %Wrapper = %(%)\n",
                            get_swift_name(param),
                            bind_wrapper_fullname(param.type),
                            get_swift_name(param));
                        w.write("let _% = try! %Wrapper?.to_abi { $0 }\n",
                            get_swift_name(param),
                            get_swift_name(param));
                    }
                    else if (is_delegate(param.type))
                    {
                        w.write("let %Wrapper = %(%)\n",
                            get_swift_name(param),
                            bind_wrapper_fullname(param.type),
                            get_swift_name(param));
                        w.write("let _% = try! %Wrapper.to_abi { $0 }\n",
                            get_swift_name(param),
                            get_swift_name(param));
                    }
                }
                else if (category == param_category::generic_type)
                {
                    auto swift_name = w.write_temp("%", param.type);
                    
                    if (swift_name.find("?") != swift_name.npos)
                    {
                        w.write("let %Wrapper = %(%)\n",
                            get_swift_name(param),
                            bind_wrapper_fullname(param.type),
                            get_swift_name(param));
                        w.write("let _% = try! %Wrapper?.to_abi { $0 }\n",
                            get_swift_name(param),
                            get_swift_name(param));
                    }
                    else if (is_collection_type(param.type))
                    {
                        w.write("let %Wrapper = %(%)\n",
                            get_swift_name(param),
                            bind_wrapper_fullname(param.type),
                            get_swift_name(param));
                        w.write("let _% = try! %Wrapper?.to_abi { $0 }\n",
                            get_swift_name(param),
                            get_swift_name(param));
                    }
                    else if (swift_name.starts_with("@escaping"))
                    {
                        w.write("let %Handler = %(handler: %)\n",
                            get_swift_name(param),
                            bind_impl_fullname(param.type),
                            get_swift_name(param));
                        w.write("let %Wrapper = %(%Handler)\n",
                            get_swift_name(param),
                            bind_wrapper_fullname(param.type),
                            get_swift_name(param));
                        w.write("let _% = try! %Wrapper.to_abi { $0 }\n",
                            get_swift_name(param),
                            get_swift_name(param));
                    }
                }
            }
            else
            {
                if (category == param_category::string_type)
                {
                    w.write("var _%: HSTRING?\n",
                        get_swift_name(param));
                    guard.push("% = .init(from: _%)\n",
                        get_swift_name(param),
                        get_swift_name(param));
                    guard.push("WindowsDeleteString(_%)\n", get_swift_name(param));
                }
                else if (category == param_category::struct_type &&
                    is_struct_blittable(signature_type) &&
                    !is_guid(category))
                {
                    w.write("var _%: % = .init()\n",
                        get_swift_name(param),
                        bind_type_mangled(param.type));
                    guard.push("% = .from(abi: _%)\n",
                        get_swift_name(param),
                        get_swift_name(param));
                }
                else if (category == param_category::struct_type)
                {
                    w.write("let _%: %._ABI_% = .init()\n",
                        get_swift_name(param),
                        abi_namespace(param.type),
                        param.type->swift_type_name());
                    guard.push("% = .from(abi: _%.val)\n",
                        get_swift_name(param),
                        get_swift_name(param));
                }
                else if (category == param_category::boolean_type || category == param_category::character_type)
                {
                    w.write("var _%: % = .init()\n",
                        get_swift_name(param),
                        bind_type_abi(param.type));
                    guard.push("% = .init(from: _%)\n",
                        get_swift_name(param),
                        get_swift_name(param));
                }
                else if (category == param_category::object_type)
                {
                    w.write("var _%: %?\n",
                        get_swift_name(param),
                        bind_type_abi(param.type));
                    if (is_interface(param.type))
                    {
                        w.write("%(%)", bind_impl_name(param.type), get_swift_name(param));
                    }
                    else if (auto default_interface = get_default_interface(signature_type))
                    {
                        auto [ns, default_interface_name] = type_name::get_namespace_and_name(default_interface);
                        guard.push("% = .init(%.%(_%))\n",
                            get_swift_name(param),
                            abi_namespace(ns),
                            default_interface_name,
                            get_swift_name(param));
                    }
                }
                else if (category == param_category::generic_type)
                {
                    w.write("var _%: UnsafeMutablePointer<%>?\n",
                        get_swift_name(param),
                        bind_type_mangled(param.type));

                    guard.push("% = %Impl(_%)\n",
                        get_swift_name(param),
                        bind_type_mangled(param.type),
                        get_swift_name(param));
                }
            }
        }

        return guard;
    }


    static void write_factory_body(writer& w, function_def const& method, interface_info const& factory, class_type const& type, metadata_type const& default_interface)
    {
        std::string_view func_name = get_abi_name(method);

        auto swift_name = get_swift_name(factory);
        auto return_name = method.return_type.value().name;
        {
            auto guard = write_local_param_wrappers(w, method);
            w.write("let % = try! Self.%.%Impl(%)\n",
                return_name,
                swift_name,
                func_name,
                bind<write_implementation_args>(method));
        }
        w.write("_default = %.%(consuming: %)\n", abi_namespace(type), default_interface, return_name);
        if (auto base_class = type.base_class)
        {
            w.write("super.init(fromAbi: try! _default.QueryInterface())\n");
        }
    }

    static void write_factory_constructors(writer& w, metadata_type const& factory, class_type const& type, metadata_type const& default_interface)
    {
        if (auto factoryIface = dynamic_cast<const interface_type*>(&factory))
        {
            interface_info factory_info{ factoryIface };
            auto swift_name = get_swift_name(factory_info);
            w.write("private static let %: %.% = try! RoGetActivationFactory(HString(\"%\"))\n",
                swift_name, abi_namespace(factory), factory, get_full_type_name(type));

            for (auto&& method : factoryIface->functions)
            {
                if (!can_write(w, method)) continue;

                w.write("public init(%) {\n", bind<write_params>(method));
                {
                    auto indent = w.push_indent();
                    write_factory_body(w, method, factory_info, type, default_interface);
                }
                w.write("}\n\n");
            }
        }
        else
        {
            auto base_class = type.base_class;

            w.write("%public init() {\n", base_class ? "override " : "");
            {
                auto indent = w.push_indent();
                w.write("try! _default = RoActivateInstance(HString(\"%\"))\n", get_full_type_name(type));
                if (base_class)
                {
                    w.write("super.init(fromAbi: try! _default.QueryInterface())\n");
                }
            }
            w.write("}\n\n");
        }
    }

    static void write_composable_constructor(writer& w, metadata_type const& factory, class_type const& type)
    {
        if (auto factoryIface = dynamic_cast<const interface_type*>(&factory))
        {
            w.write("private static var _% : %.% =  try! RoGetActivationFactory(HString(\"%\"))\n",
                factory,
                abi_namespace(factory),
                factory,
                get_full_type_name(type));

            auto base_class = type.base_class;
            if (!base_class)
            {
                auto base_composable_init = R"(public init() {
    self._default = MakeComposed(Self._%, &_inner, self)
}

public init<Factory: ComposableActivationFactory>(_ factory : Factory) {
    self._default = try! MakeComposed(factory, &_inner, self as! Factory.Composable.Default.swift_Projection).QueryInterface()
    _ = self._default.Release() // release to reset reference count since QI caused an AddRef on ourselves
}
)";
                w.write(base_composable_init, factory);
            }
            else
            {
                auto override_composable_init = R"(override public init() {
    super.init(Self._%) 
    let parentDefault: UnsafeMutablePointer<%>? = super._get_abi()
    self._default = try! IInspectable(parentDefault).QueryInterface()
    _ = self._default.Release() // release to reset reference count since QI caused an AddRef on ourselves
}

override public init<Factory: ComposableActivationFactory>(_ factory: Factory) {
    super.init(factory)
    let parentDefault: UnsafeMutablePointer<%>? = super._get_abi()
    self._default = try! IInspectable(parentDefault).QueryInterface()
    _ = self._default.Release() // release to reset reference count since QI caused an AddRef on ourselves
}
)";
                w.write(override_composable_init, factory, bind_type_abi(ElementType::Object), bind_type_abi(ElementType::Object));
            }
        }
    }

    static void write_default_constructor_declarations(writer& w, class_type const& type, metadata_type const& default_interface)
    {
        auto [ns, name] = get_type_namespace_and_name(default_interface);
        auto base_class = type.base_class;

        // We unwrap composable types to try and get to any derived type.
        // If not composable, then create a new instance
        w.write("public static func from(abi: UnsafeMutablePointer<%>?) -> % {\n",
            bind_type_mangled(default_interface), type);
        {
            auto indent = w.push_indent();
            w.write(type.is_composable()
                ? "UnsealedWinRTClassWrapper<Composable>.unwrap_from(base: abi!)\n"
                : ".init(fromAbi: .init(abi))\n");
        }
        w.write("}\n\n");

        w.write("%public init(fromAbi: %.IInspectable) {\n",
            base_class ? "override " : "", w.support);
        {
            auto indent = w.push_indent();
            w.write("_default = try! fromAbi.QueryInterface()\n");
            if (base_class)
            {
                w.write("super.init(fromAbi: fromAbi)\n");
            }
        }
        w.write("}\n\n");
    }

    static void write_class_func_body(writer& w, function_def const& function, interface_info const& iface)
    {
        std::string_view func_name = get_abi_name(function);
        {
            auto guard = write_local_param_wrappers(w, function);

            auto impl = get_swift_name(iface);

            if (function.return_type)
            {
                w.write("let % = try! %.%Impl(%)\n",
                    function.return_type.value().name,
                    impl,
                    func_name,
                    bind<write_implementation_args>(function));
            }
            else
            {
                w.write("try! %.%Impl(%)\n",
                    impl,
                    func_name,
                    bind<write_implementation_args>(function));
            }
        }
        if (function.return_type)
        {
            w.write("%\n", bind<write_consume_return_statement>(function));
        }
    }

    static void write_class_impl_property(writer& w, property_def const& prop, interface_info const& iface)
    {
        if (!can_write(w, prop)) return;

        // TODO: https://linear.app/the-browser-company/issue/WIN-82/support-setters-not-defined-in-same-api-contract-as-getters
        // right now require that both getter and setter are defined in the same version
        if (prop.getter)
        {
            w.write("public %var % : % {\n",
                iface.attributed ? "static " : "",
                get_swift_name(prop),
                prop.getter.value().return_type.value());
        }
        auto property_indent_guard = w.push_indent();

        auto impl = get_swift_name(iface);

        if (prop.getter)
        {
            auto getter_format = R"(get {
    let % = try! %.%Impl()
    %
}

)";
            auto getterSig = prop.getter.value();
            w.write(getter_format,
                getterSig.return_type.value().name,
                impl,
                get_swift_name(getterSig),
                bind<write_consume_return_statement>(getterSig));
        }

        // TODO: https://linear.app/the-browser-company/issue/WIN-82/support-setters-not-defined-in-same-api-contract-as-getters
        // right now require that both getter and setter are defined in the same version
        if (prop.setter && prop.getter)
        {
            w.write("set {\n");
            auto set_indent_guard = w.push_indent();

            std::string extra_init;

            TypeDef signature_type{};
            auto category = get_category(prop.type, &signature_type);
            if (category == param_category::string_type)
            {
                w.write("let _newValue = try! HString(newValue)\n");
            }
            else if (category == param_category::struct_type && !is_struct_blittable(signature_type))
            {
                w.write("let _newValue = %._ABI_%(from: newValue)\n", abi_namespace(signature_type), signature_type.TypeName());
            }
            else if (is_interface(prop.type))
            {
                w.write("let wrapper = %(newValue)\n", bind_wrapper_fullname(prop.type));
                w.write("let _newValue = try! wrapper?.to_abi { $0 }\n");
            }
            else if (is_delegate(prop.type))
            {
                w.write("let wrapper = %(newValue)\n", bind_wrapper_fullname(prop.type));
                w.write("let _newValue = try! wrapper.to_abi { $0 }\n");
            }
  
            w.write("try! %.%Impl(%)\n",
                impl,
                get_swift_name(prop.setter.value()),
                bind<write_convert_to_abi_arg>("newValue", prop.type, false));

            set_indent_guard.end();
            w.write("}\n");
        }
        
        // TODO: https://linear.app/the-browser-company/issue/WIN-82/support-setters-not-defined-in-same-api-contract-as-getters
        // right now require that both getter and setter are defined in the same version
        if (prop.getter)
        {
            property_indent_guard.end();
            w.write("}\n\n");
        }
    }

    static void write_class_impl_func(writer& w, function_def const& function, interface_info const& iface)
    {
        if (function.def.SpecialName() || !can_write(w, function))
        {
            // don't write methods which are really properties
            return;
        }

        w.write("% func %(%) %{\n",
            iface.overridable ? "open" : "public",
            get_swift_name(function),
            bind<write_params>(function),
            bind<write_return_type_declaration>(function));
        {
            auto indent = w.push_indent();
            write_class_func_body(w, function, iface);
        }
        w.write("}\n\n");
    }
    
    static void write_event_registrar_name(writer& w, winmd::reader::Event event)
    {
        w.write("%Registrar", event.Name());
    }

    static void write_event_registrar(writer& w, event_def const& event, interface_info const& iface)
    {
        auto abi_name = w.write_temp("%.%", abi_namespace(iface.type), iface.type->swift_type_name());
        auto format = R"(private class % : IEventRegistration {
    func add(delegate: any WinRTDelegate, for impl: %.IInspectable){
        let wrapper = %(delegate as! %)
        let abi = try! wrapper.to_abi { $0 }
        let impl:% = try! impl.QueryInterface()
        delegate.token = try! impl.add_%Impl(abi)
    }

    func remove(delegate: any WinRTDelegate, for impl: %.IInspectable){
        let impl: % = try! impl.QueryInterface()
            if let token = delegate.token {
            try! impl.remove_%Impl(token)
        }
    }
}
)";
        w.write(format,
            bind<write_event_registrar_name>(event.def), // class %Registrar
            w.support, // %.IInspectable
            bind_wrapper_fullname(event.type), // let wrapper = %
            bind_impl_fullname(event.type), // as! %
            abi_name,  // let impl:%
            event.def.Name(), // delegate.token = try! impl.add_%
            w.support, // %.IInspectable
            abi_name,  // let impl:%
            event.def.Name()); //try! impl.remove_%
        }

    static void write_class_impl_event(writer& w, event_def const& def, interface_info const& iface)
    {
        auto event = def.def;
        auto format = R"(private static let _% = %()
public % var % : Event<(%),%> = EventImpl<%>(register: %_%, owner:%)
)";
        auto registrar_name = w.write_temp("%", bind<write_event_registrar_name>(event));
        auto type = find_type(event.EventType());
        writer::generic_param_guard guard{};
        function_def delegate_method{};
        if (auto delegateType = dynamic_cast<const delegate_type*>(def.type))
        {
            delegate_method = delegateType->functions[0];
        }
        else if (auto genericInst = dynamic_cast<const generic_inst*>(def.type))
        {
            delegate_method = genericInst->functions[0];
            guard = w.push_generic_params(*genericInst);
        }
        assert(delegate_method.def);

        w.write(format,
            registrar_name, // private static let _% 
            registrar_name, // %()
            iface.attributed ? "static" : "lazy", // public %
            event.Name(), // var %
            bind<write_comma_param_types>(delegate_method.params), // Event<(%)
            bind<write_delegate_return_type>(delegate_method), // , %>
            bind_impl_fullname(def.type), // EventImpl<%>
            iface.attributed ? "" : "Self.", // register:%
            registrar_name, // _%
            get_swift_name(iface) // owner:%
        );

        write_event_registrar(w, def, iface);
    }

    static void write_statics_body(writer& w, function_def const& method, metadata_type const& statics)
    {
        std::string_view func_name = get_abi_name(method);

        {
            auto guard = write_local_param_wrappers(w, method);

            if (method.return_type)
            {
                w.write("let % = try! _%.%Impl(%)\n",
                    method.return_type.value().name,
                    statics.swift_type_name(),
                    func_name,
                    bind<write_implementation_args>(method));
            }
            else
            {
                w.write("try! _%.%Impl(%)\n",
                    statics.swift_type_name(),
                    func_name,
                    bind<write_implementation_args>(method));
            }
        }
        if (method.return_type)
        {
            w.write("%\n", bind<write_consume_return_statement>(method));
        }
    }

    static void write_static_methods(writer& w, attributed_type const& statics, class_type const& type)
    {
        if (auto ifaceType = dynamic_cast<const interface_type*>(statics.type))
        {
            interface_info static_info{ statics.type };
            auto impl_name = get_swift_name(static_info);
            w.write("private static let %: %.% = try! RoGetActivationFactory(HString(\"%\"))\n",
                impl_name,
                abi_namespace(statics.type),
                statics.type->swift_type_name(),
                get_full_type_name(type));

            for (auto&& method : ifaceType->functions)
            {
                if (!can_write(w, method))
                {
                    continue;
                }

                w.write("public static func %(%) %{\n",
                    get_swift_name(method),
                    bind<write_params>(method),
                    bind<write_return_type_declaration>(method));
                {
                    auto indent = w.push_indent();
                    write_statics_body(w, method, *statics.type);
                }
                w.write("}\n\n");
            }

            static_info.attributed = true;
            for (auto&& static_prop : ifaceType->properties)
            {
                write_class_impl_property(w, static_prop, static_info);
            }

            for (auto&& event : ifaceType->events)
            {
                write_class_impl_event(w, event, static_info);
            }
        }
    }

    static void write_composable_impl(writer& w, class_type const& parent, metadata_type const& overrides, bool compose)
    {
        auto default_interface = parent.default_interface;
        if (!default_interface)
        {
            throw_invalid("Could not find default interface for %s\n", parent.swift_type_name().data());
        }

        bool use_iinspectable_vtable = type_name(overrides) == type_name(*default_interface);

        auto format = R"(internal class % : ComposableImpl {
    internal typealias c_ABI = %
    internal typealias swift_ABI = %
    internal class Default : MakeComposedAbi {
        internal typealias swift_Projection = %
        internal typealias c_ABI = %
        internal typealias swift_ABI = %.%
        internal static func from(abi: UnsafeMutableRawPointer?) -> swift_Projection {
            .init(fromAbi: .init(abi!))
        }
    }
}
)";

        // If we're composing a type without any overrides, then we'll just create an IInspectable vtable which wraps
        // this object and provides facilities for reference counting and getting the class name of the swift object.
        w.write(format,
            overrides.swift_type_name(),
            bind([&](writer& w) {
                    if (use_iinspectable_vtable)
                    {
                        write_type_mangled(w, ElementType::Object);
                    }
                    else
                    {
                        write_type_mangled(w, overrides);
                    }}),
            bind([&](writer& w) {
                    if (use_iinspectable_vtable)
                    {
                        w.write(ElementType::Object);
                    }
                    else
                    {
                        w.write("%.%", abi_namespace(overrides), overrides.swift_type_name());
                    }}),
                parent,
            bind_type_mangled(default_interface),
            abi_namespace(parent),
            default_interface);

        if (compose)
        {
            auto modifier = parent.is_composable() ? "open" : "public";
            w.write("internal typealias Composable = %\n", overrides.swift_type_name());
            w.write("%% class var _makeFromAbi : any MakeFromAbi.Type { Composable.Default.self }\n",
                parent.base_class ? "override " : "", modifier);
        }
    }   

    // write the default implementation for makeAbi. this way we don't need to expose the internal implementation
    // details of the vtable to external modules.
    static void write_composable_impl_extension(writer& w, class_type const& overridable)
    {
        if (!overridable.is_composable())
        {
            return;
        }   

        for (auto&& [_, info] : overridable.required_interfaces)
        {
            if (!info.overridable || info.base) continue;

            w.write(R"(extension ComposableImpl where c_ABI == % {
    public static func makeAbi() -> c_ABI {
        let vtblPtr = withUnsafeMutablePointer(to: &%.%VTable) { $0 }
        return .init(lpVtbl: vtblPtr)
    }
}
)", bind_type_mangled(info.type),
    abi_namespace(info.type),
    info.type->swift_type_name());
        }
    }


    static void write_class_interface_impl(writer& w, interface_info const& info)
    {
        if (info.overridable)
        {
            // when implementing default overrides, we want to call to the inner non-delegating IUnknown
            // as this will get us to the inner object. otherwise we'll end up with a stack overflow 
            // because we'll be calling the same method on ourselves
            w.write("internal lazy var %: %.% = try! IUnknown(_inner).QueryInterface()\n",
                get_swift_name(info),
                abi_namespace(info.type->swift_logical_namespace()),
                info.type->swift_type_name());

        }
        else if (!info.is_default)
        {
            auto swiftAbi = w.write_temp("%.%", abi_namespace(info.type->swift_logical_namespace()), info.type->swift_type_name());
            if (is_collection_type(info.type))
            {
                w.generic_param_stack.push_back(info.generic_params);
                writer::generic_param_guard guard{ &w };
                swiftAbi = w.write_temp("%.%", abi_namespace(w.type_namespace), bind_type_abi(info.type));
            }
            w.write("internal lazy var %: % = try! _default.QueryInterface()\n",
                get_swift_name(info),
                swiftAbi);
        }

        if (is_collection_type(info.type))
        {
            write_collection_protocol_conformance(w, info);
        }

        if (auto iface = dynamic_cast<const interface_type*>(info.type))
        {
            for (auto&& method : iface->functions)
            {
                write_class_impl_func(w, method, info);
            }

            for (auto&& prop : iface->properties)
            {
                write_class_impl_property(w, prop, info);
            }

            for (auto&& event : iface->events)
            {
                write_class_impl_event(w, event, info);
            }
        }
        else if (auto gti = dynamic_cast<const generic_inst*>(info.type))
        {
            for (auto&& method : gti->functions)
            {
                write_class_impl_func(w, method, info);
            }

            for (auto&& prop : gti->properties)
            {
                write_class_impl_property(w, prop, info);
            }

            for (auto&& event : gti->events)
            {
                write_class_impl_event(w, event, info);
            }
        }
    }

    static void write_class_impl(writer& w, class_type const& type)
    {
        if (!can_write(w, type)) return;

        auto default_interface = type.default_interface;
        auto typeName = type.swift_type_name();
        auto base_class = type.base_class;
        bool composable = type.is_composable();
        auto modifier = composable ? "open" : "public final";

        if (base_class)
        {
            w.write("% class % : %", modifier, typeName, get_full_swift_type_name(w, base_class));
        }
        else if (default_interface)
        {
            auto base_class_string = composable ? "UnsealedWinRTClass" : "WinRTClass";
            w.write("% class % : %", modifier, typeName, base_class_string);
        }
        else
        {
            w.write("public final class %", typeName);
        }

        auto interfaces = type.required_interfaces;
        separator s{ w };
        s();

        std::vector<std::string> collection_type_aliases;
        for (auto&& [interface_name, info] : type.required_interfaces)
        {
            // Filter out which interfaces we actually want to declare on the class. We don't want to specify interfaces which come from the base class or which
            // ones are exclusive
            if (info.base || info.exclusive || interface_name.empty()) continue;

            // Avoid writing both IIterable and IVector/IMap
            if (interface_name.starts_with("IIterable<") && type.required_interfaces.size() > 1) continue;

            s();

            // when deriving from collections we want to just derive from `IVector` and will use a typealias to set the Element (this is required by Swift)
            auto name_to_write = interface_name;
            if (is_collection_type(info.type))
            {
                if (interface_name.starts_with("IVector"))
                {
                    collection_type_aliases.emplace_back(w.write_temp("Element = %", info.generic_params[0]));
                }
                else if (interface_name.starts_with("IMap"))
                {
                    collection_type_aliases.emplace_back(w.write_temp("Key = %", info.generic_params[0]));
                    collection_type_aliases.emplace_back(w.write_temp("Value = %", info.generic_params[1]));
                }
                name_to_write = interface_name.substr(0, interface_name.find_first_of('<'));
            }
            w.write(name_to_write);
        }

        w.write(" {\n");

        auto class_indent_guard = w.push_indent();

        if (composable && !base_class)
        {
            w.write("private (set) public var _inner: UnsafeMutablePointer<%.IInspectable>?\n", w.c_mod);
        }

        for (const auto& collection_type_alias : collection_type_aliases)
        {
            w.write("public typealias %\n", collection_type_alias);
        }
        writer::generic_param_guard guard;

        if (default_interface)
        {
            auto [ns, name] = get_type_namespace_and_name(*default_interface);
            auto swiftAbi = w.write_temp("%.%", abi_namespace(ns), name);
            std::string defaultVal = "";
            if (is_collection_type(default_interface))
            {
                auto generic_type = dynamic_cast<const generic_inst*>(default_interface);
                guard = w.push_generic_params(*generic_type);
                swiftAbi = w.write_temp("%.%", abi_namespace(w.type_namespace), bind_type_abi(generic_type));
            }

            w.write(R"(private typealias swift_ABI = %
private typealias c_ABI = %
private var _default: swift_ABI = .init(UnsafeMutableRawPointer.none)
% func _get_abi<T>() -> UnsafeMutablePointer<T>? {
    if T.self == c_ABI.self {
        return RawPointer(_default)
    }   
    if T.self == %.IInspectable.self {
        return RawPointer(_default)
    }
    return %
}

)",
                swiftAbi,
                bind_type_mangled(default_interface),
                base_class ? 
                            composable ? "override open" : 
                                         "override public" :
                            composable ? "open" :
                                         "public",
                w.c_mod,
                base_class ? "super._get_abi()" : "nil");
            write_default_constructor_declarations(w, type, *default_interface);
        }
        for (auto&& [interface_name, factory] : type.factories)
        {
            if (factory.activatable)
            {
                write_factory_constructors(w, *factory.type, type, *default_interface);
            }

            if (factory.statics)
            {
                write_static_methods(w, factory, type);
            }

            if (factory.composable)
            {
                write_composable_constructor(w, *factory.type, type);
            }
        }

        bool has_overrides = false;
        for (auto&& [interface_name, info] : type.required_interfaces)
        {
            if (interface_name.empty() || !can_write(w, info.type)) { continue; }

            w.generic_param_stack.push_back(info.generic_params);
            writer::generic_param_guard guard2{ &w };
            // Don't reimplement P/M/E for interfaces which are implemented on a base class
            if (!info.base)
            {
                write_class_interface_impl(w, info);
            }
          
            // Generate definitions for how to compose overloads and create wrappers of this type.
            if (info.overridable && !info.base)
            {
                // the very first override is the one we use for composing the class and can respond to QI
                // calls for all the other overloads
                const bool compose = !has_overrides;
                write_composable_impl(w, type, *info.type, compose);
                has_overrides = true;
            }
            else if (info.overridable && info.base && !has_overrides)
            {
                // This unsealed class doesn't have an overridable interface of it's own, so use the first base
                // interface we come across for the composable implementation. This is used by the factory method
                // when we are creating an aggregated type and enables the app to override the base class methods
                // on this type
                const bool compose = true;
                write_composable_impl(w, type, *info.type, compose);
                has_overrides = true;
            }

        }

        if (composable && !has_overrides && default_interface)
        {
            write_composable_impl(w, type, *default_interface, true);
        }

        class_indent_guard.end();
        w.write("}\n\n");
    }

    static void write_class(writer& w, class_type const& type)
    {
        write_class_impl(w, type);
    }

    static void write_struct_initializer_params(writer& w, struct_type const& type)
    {
        separator s{ w };

        for (auto&& field : type.members)
        {
            // WIN-64 - swiftwinrt: support boxing/unboxing
            // WIN-65 - swiftwinrt: support generic types
            if (can_write(w, field.type))
            {
                s();

                w.write("%: %", get_swift_name(field), field.type);
            }
           
        }
    }

    static void write_struct_init_extension(writer& w, struct_type const& type)
    {
        bool is_blittable = is_struct_blittable(type);
        if (is_blittable)
        {
            w.write(R"(extension % {
    public static func from(swift: %) -> % {
        .init(%)
    }
}
)", bind_type_mangled(type), get_full_swift_type_name(w,type), bind_type_mangled(type), bind([&](writer& w) {
                    separator s{ w };
                    for (auto&& field : type.members)
                    {
                        s();

                        if (dynamic_cast<const struct_type*>(field.type))
                        {
                            w.write("%: .from(swift: swift.%)",
                                get_swift_name(field),
                                get_swift_name(field)
                            );
                        }
                        else
                        {
                            w.write("%: swift.%",
                                get_swift_name(field),
                                get_swift_name(field)
                            );
                        }
  
                    }
    }));
        }
    }

    static void write_struct(writer& w, struct_type const& type)
    {
        w.write("public struct % {\n", type);
        {
            auto indent_guard1 = w.push_indent();
            for (auto&& field : type.members)
            {
                auto field_type = field.type;
                // WIN-64 - swiftwinrt: support boxing/unboxing
                // WIN-65 - swiftwinrt: support generic types
                if (!can_write(w, field_type)) continue;

                w.write("public var %: %\n", 
                    get_swift_name(field), 
                    bind<write_init_val>(field_type));
            }

            w.write("public init() {}\n");
            w.write("public init(%) {\n", bind<write_struct_initializer_params>(type));
            {
                auto indent_guard2 = w.push_indent();
                for (auto&& field : type.members)
                {
                    // WIN-64 - swiftwinrt: support boxing/unboxing
                    // WIN-65 - swiftwinrt: support generic types
                    if (can_write(w, field.type))
                    {
                        auto field_name = get_swift_name(field);
                        w.write("self.% = %\n", field_name, field_name);
                    }
                }
            }
            w.write("}\n");

            w.write(R"(public static func from(abi: %) -> % {
    .init(%)
}
)", bind_type_mangled(type),
    type,
    bind([&](writer& w) {
                    separator s{ w };
                    for (auto&& field : type.members)
                    {
                        if (can_write(w, field.type))
                        {
                            s();
                            std::string from = std::string("abi.").append(get_swift_name(field));
                            w.write("%: %",
                                get_swift_name(field),
                                bind<write_consume_type>(field.type, from)
                            );
                        }
                    }
    }));
         
        }
        w.write("}\n\n");
    }

    template <typename T>
    static void write_iunknown_methods(writer& w, T const& type, std::vector<named_interface_info> const& interfaces, bool composed = false)
    {
        w.write(R"(QueryInterface: {
    guard let pUnk = $0, let riid = $1, let ppvObject = $2 else { return E_INVALIDARG }
%
    guard riid.pointee == IUnknown.IID ||
          riid.pointee == IInspectable.IID || 
          riid.pointee == ISwiftImplemented.IID ||
          riid.pointee == IIAgileObject.IID ||
          riid.pointee == %.IID else { 
        %
    }
    _ = pUnk.pointee.lpVtbl.pointee.AddRef(pUnk)
    ppvObject.pointee = UnsafeMutableRawPointer(pUnk)
    return S_OK
},

)",
bind([&](writer& w) {
                auto format = R"(if riid.pointee == %.IID {
    guard let instance = %.try_unwrap_from(raw: pUnk)% else { return E_NOINTERFACE }
    guard let inner = %(instance) else { return E_INVALIDARG }
    let pThis = try! inner.to_abi { $0 }
    return pThis.pointee.lpVtbl.pointee.QueryInterface(pThis, riid, ppvObject)
}
)";
                type_name typeName(type);
                // workaround right now for the fact that if the relationship isn't specified in metadata
                
                if (typeName.name == "IApplicationOverrides" && typeName.name_space == "Microsoft.UI.Xaml")
                {
                    auto metadata_provider = &w.cache->find("Microsoft.UI.Xaml.Markup", "IXamlMetadataProvider");
                    auto cast = w.write_temp(" as? %", metadata_provider);

                    w.write(format,
                        bind_wrapper_fullname(metadata_provider), // if riid.pointee == %
                        bind_wrapper_name(type), // guard let instance = %
                        cast,
                        bind_wrapper_fullname(metadata_provider)); // let inner = %
                }
                for (auto&& iface : interfaces)
                {
                    if (!can_write(w, iface.second.type)) continue;

                    w.write(format,
                        bind_wrapper_fullname(iface.second.type), // if riid.pointee == %
                        bind_wrapper_name(type), // guard let instance = %
                        "", // no casting
                        bind_wrapper_fullname(iface.second.type) // let inner = %
                    );

                   
                }}),
            bind_wrapper_fullname(type),
            bind([&](writer & w) {
               if (!composed)
               {
                        w.write(R"(ppvObject.pointee = nil
                return E_NOINTERFACE
)");
               }
               else
               {
                   w.write(R"(    guard let instance = %.try_unwrap_from(raw: $0),
                    let inner = instance._inner else { return E_INVALIDARG }
                
            return inner.pointee.lpVtbl.pointee.QueryInterface(inner, riid, ppvObject)
)", bind_wrapper_name(type));
               }
            })
    );

        w.write(R"(AddRef: {
     guard let wrapper = %.from_raw($0) else { return 1 }
     _ = wrapper.retain()
     return ULONG(_getRetainCount(wrapper.takeUnretainedValue().swiftObj))
},

)",
            bind_wrapper_name(type)
        );

        w.write(R"(Release: {
    guard let wrapper = %.from_raw($0) else { return 1 }
    return ULONG(_getRetainCount(wrapper.takeRetainedValue()))
},

)",
            bind_wrapper_name(type)
);
    }

    static void write_iunknown_methods(writer& w, generic_inst const& type)
    {
        if (auto ifaceType = dynamic_cast<const interface_type*>(type.generic_type()))
        {
            write_iunknown_methods(w, type, ifaceType->required_interfaces);

        }
        else
        {
            write_iunknown_methods(w, type, {});
        }
    }
    
    template <typename T>
    static void write_iinspectable_methods(writer& w, T const& type, std::vector<named_interface_info> const& interfaces, bool composed = false)
    {
        // 3 interfaces for IUnknown, IInspectable, type.
        auto interface_count = 3 + interfaces.size();
        w.write("GetIids: {\n");
        {
            auto indent_guard = w.push_indent();
            w.write("let size = MemoryLayout<IID>.size\n");
            w.write("let iids = CoTaskMemAlloc(UInt64(size) * %).assumingMemoryBound(to: IID.self)\n", interface_count);
            w.write("iids[0] = IUnknown.IID\n");
            w.write("iids[1] = IInspectable.IID\n");
            w.write("iids[2] = %.IID\n", bind_wrapper_fullname(type));
            
            auto iface_n = 3;
            for (auto&& iface : interfaces)
            {
                if (!can_write(w, iface.second.type)) continue;

                w.write("iids[%] = %.%.IID\n",
                    iface_n++,
                    abi_namespace(iface.second.type),
                    bind_wrapper_name(iface.second.type)
                );
            }

            w.write("$1!.pointee = %\n", interface_count);
            w.write("$2!.pointee = iids\n");
            w.write("return S_OK\n");
        }
        w.write("},\n\n");

        if (composed)
        {
            // for composed types, get the swift object and grab the typename
            w.write(R"(GetRuntimeClassName: {
    guard let instance = %.try_unwrap_from(raw: $0) else { return E_INVALIDARG }
    let hstring = instance.GetRuntimeClassName().detach()
    $1!.pointee = hstring
    return S_OK
},

)", bind_wrapper_name(type));
        }
        else
        {
            w.write(R"(GetRuntimeClassName: {
    _ = $0
    let hstring = try! HString("%").detach()
    $1!.pointee = hstring
    return S_OK
},

)", get_full_type_name(type));
        }


        w.write(R"(GetTrustLevel: {
    _ = $0
    $1!.pointee = TrustLevel(rawValue: 0)
    return S_OK
})"
);
    }

    static void write_iinspectable_methods(writer& w, generic_inst const& type)
    {
        if (auto ifaceType = dynamic_cast<const interface_type*>(type.generic_type()))
        {
            write_iinspectable_methods(w, type, ifaceType->required_interfaces);

        }
        else
        {
            write_iinspectable_methods(w, type, {});
        }
    }
    
    // assigns return or out parameters in vtable methods
    static void do_write_abi_val_assignment(writer& w, const metadata_type* type, std::string_view const& param_name, std::string_view const& return_param_name)
    {
        TypeDef signature_type{};
        auto category = get_category(type, &signature_type);

        if (category == param_category::struct_type && !is_struct_blittable(signature_type))
        {
            w.write("let _% = %._ABI_%(from: %)\n\t",
                param_name,
                abi_namespace(type),
                type->swift_type_name(),
                param_name);
        }
        else if (is_interface(type))
        {
            w.write("let %Wrapper = %(%)\n",
                param_name,
                    bind_wrapper_fullname(type),
                param_name);
            w.write("let _% = try! %Wrapper?.to_abi { $0 }\n",
                param_name,
                param_name);
        }
        else if (is_delegate(type))
        {
            w.write("let %Wrapper = %(%)\n",
                param_name,
                bind_wrapper_fullname(type),
                param_name);
            w.write("let _% = try! %Wrapper.to_abi { $0 }\n",
                param_name,
                param_name);
        }

        w.write("%?.initialize(to: %)\n",
            return_param_name,
            bind<write_convert_to_abi_arg>(param_name, type, true)
        );
    }

    static void write_abi_ret_val(writer& w, function_def signature)
    {
        int param_number = 1;
        for (auto& param : signature.params)
        {
            if (param.out())
            {
                auto return_param_name = "$" + std::to_string(param_number);
                auto param_name = get_swift_name(param);
                do_write_abi_val_assignment(w, param.type, param_name, return_param_name);
            }
            param_number++;
        }

        if (signature.return_type)
        {
            auto return_param_name = "$" + std::to_string(signature.params.size() + 1);
            do_write_abi_val_assignment(w, signature.return_type.value().type, signature.return_type.value().name, return_param_name);
        }
    }

    static void write_not_implementable_vtable_method(writer& w, function_def const& sig)
    {
        w.write("%: { _, % in return failWith(err: E_NOTIMPL) }", get_abi_name(sig), bind([&](writer& w) {
            separator s{ w };
            for (auto& param : sig.params)
            {
                s();
                if (param.signature.Type().is_szarray())
                {
                    w.write("_, ");
                }
                w.write("_");
            }
            if (sig.return_type)
            {
                s();
                if (sig.return_type.value().signature.Type().is_szarray())
                {
                    w.write("_, ");
                }
                w.write("_");
            }}));
    }

    template<typename T>
    static void write_vtable_method(writer& w, function_def const& function, T const& type)
    {
        constexpr bool isInterface = std::is_same_v<T, interface_type>;
        constexpr bool isDelegate = std::is_same_v<T, delegate_type>;
        constexpr bool isGeneric = std::is_same_v<T, generic_inst>;
        static_assert(isInterface || isDelegate | isGeneric);

        auto method = function.def;
        auto func_name = get_abi_name(method);

        // https://linear.app/the-browser-company/issue/WIN-104/swiftwinrt-support-authoring-winrt-events-in-swift
        if (!can_write(w, function, true) ||
            is_add_overload(method) ||
            is_remove_overload(method))
        {
            write_not_implementable_vtable_method(w, function);
            return;
        }
        // we have to write the methods in a certain order and so we will have to detect here whether
        // this method is a property getter/setter and adjust the call so we use the swift syntax
        std::string func_call;
        if (is_get_overload(method))
        {
            func_call += w.write_temp("%", method.Name().substr(4));
            if (isGeneric)
            {
                auto genericInst = (const generic_inst&)type;
                if (genericInst.generic_type()->swift_full_name().starts_with("Windows.Foundation.IReference"))
                {
                    func_call.append(w.write_temp(" as! %", get_full_swift_type_name(w, genericInst.generic_params()[0])));
                }
            }
        }
        else if (is_put_overload(method))
        {
            func_call += w.write_temp("% = %", method.Name().substr(4), bind<write_consume_args>(function));
        }
        else
        {
            // delegate arg types are a tuple, so wrap in an extra paranthesis
            auto format = is_delegate(type) ? "%((%))" : "%(%)";
            func_call += w.write_temp(format, get_swift_name(method), bind<write_consume_args>(function));
        }

        w.write("%: {\n", func_name);
        {
            auto indent_guard = w.push_indent();
            w.write("guard let __unwrapped__instance = %.try_unwrap_from(raw: $0) else { return E_INVALIDARG }\n",
                bind_wrapper_name(type));
            write_consume_params(w, function);

            if (function.return_type)
            {
                w.write("let % = ", function.return_type.value().name);
            }
            w.write("__unwrapped__instance.%\n", func_call);
            write_abi_ret_val(w, function);
            w.write("return S_OK\n");
        }
        w.write("}");
    }

    template <typename T>
    static void do_write_vtable(writer& w, T const& type, std::vector<named_interface_info> interfaces)
    {
        constexpr bool isInterface = std::is_same_v<T, interface_type>;
        constexpr bool isDelegate = std::is_same_v<T, delegate_type>;
        constexpr bool isGeneric = std::is_same_v<T, generic_inst>;
        static_assert(isInterface || isDelegate | isGeneric);
        w.write("internal static var %VTable: %Vtbl = .init(\n",
            type,
            bind_type_mangled(type));
        {
            auto indent = w.push_indent();
            write_iunknown_methods(w, type, interfaces);
            separator s{ w, ",\n\n" };

            if (!is_delegate(type))
            {
                write_iinspectable_methods(w, type, interfaces);
                s(); // get first separator out of the way for no-op
            }

            for (auto&& method : type.functions)
            {
                if (method.def.Name() != ".ctor")
                {
                    s();
                    write_vtable_method(w, method, type);
                }

            }
        }

        w.write(R"(
)
)");

    }
    static void write_vtable(writer& w, interface_type const& type)
    {
        do_write_vtable(w, type, type.required_interfaces);
    }

    static void write_vtable(writer& w, delegate_type const& type)
    {
        do_write_vtable(w, type, {});
    }

    static void write_overrides_vtable(writer& w, class_type const& type, interface_info const& overrides, std::vector<named_interface_info> const& other_interfaces)
    {
        w.write("internal typealias % = UnsealedWinRTClassWrapper<%.%>\n",
            bind_wrapper_name(overrides.type),
            get_full_swift_type_name(w, type),
            overrides.type
        );
        w.write("internal static var %VTable: %Vtbl = .init(\n",
            overrides.type,
            bind_type_mangled(overrides.type));

        {
            auto indent = w.push_indent();
            write_iunknown_methods(w, overrides.type, other_interfaces, true);
            write_iinspectable_methods(w, overrides.type, other_interfaces, true);

            separator s{ w, ",\n\n" };
            s(); // get first separator out of the way for no-op

            if (auto iface = dynamic_cast<const interface_type*>(overrides.type))
            {
                for (auto&& method : iface->functions)
                {
                    if (method.def.Name() != ".ctor")
                    {
                        s();
                        write_vtable_method(w, method, *iface);
                    }
                }
            }
           
        }

        w.write(R"(
)
)");
    }

    static void write_class_abi(writer& w, class_type const& type)
    {
        if (!type.is_composable())
        {
            return;
        }

        std::vector<named_interface_info> other_composable_interfaces;
        auto interfaces = type.required_interfaces;
        for (auto iter = interfaces.rbegin(); iter != interfaces.rend(); iter++)
        {
            auto&& [interface_name, info] = *iter;
            if (!info.overridable) { continue; }

            // Generate definitions for how to compose overloads and create wrappers of this type.
            if (!info.base)
            {
                write_overrides_vtable(w, type, info, other_composable_interfaces);
            }
            other_composable_interfaces.push_back(*iter);
        }
    }

}
