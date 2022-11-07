#pragma once

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

    static void write_type_sig_abi(writer& w, TypeSig const& type)
    {
        auto push_abi = w.push_abi_types(true);
        w.write("%", type);
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
                auto use_abi_type = w.push_abi_types(true);
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

            if (w.type_namespace != type_name.name_space || w.abi_types || w.full_type_names)
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
            auto mangled_name = w.push_abi_types(true);
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

    // The following two methods are hacks to work around TypeSig authoring wrapping types
    // in UnsafeMutablePointer<%>. We just want the mangled name, so set mangled_names to true
    // on the writer.
    static void write_wrapper_type_sig(writer& w, TypeSig const& type)
    {
        auto mangled_names = w.push_mangled_names(true);
        write_wrapper_name(w, type);
    }

    static void write_impl_type_sig(writer& w, TypeSig const& type)
    {
        auto mangled_names = w.push_mangled_names(true);
        write_impl_name(w, type);
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

    auto bind_event_delegate_wrapper_fullname(Event const& event)
    {
        return [&](writer& w)
        {
            // Due to https://linear.app/the-browser-company/issue/WIN-106/swiftwinrt-properly-compose-types
            // we need to use the namespace of the owner type for this event.
            std::string temp_ns = w.type_namespace;
            w.type_namespace = event.Parent().TypeNamespace();
            auto full_name = w.push_full_type_names(true);
            write_wrapper_name(w, event.EventType());
            w.type_namespace = temp_ns;
        };
    }

    auto bind_event_delegate_impl_fullname(Event const& event)
    {
        return [&](writer& w)
        {
            // Due to https://linear.app/the-browser-company/issue/WIN-106/swiftwinrt-properly-compose-types
            // we need to use the namespace of the owner type for this event.
            std::string temp_ns = w.type_namespace;
            w.type_namespace = event.Parent().TypeNamespace();
            auto full_name = w.push_full_type_names(true);
            write_impl_name(w, event.EventType());
            w.type_namespace = temp_ns;
        };
    }

    static bool can_write(writer& w, TypeDef const& type);

    static void write_enum_def(writer& w, TypeDef const& type)
    {
        // Async status is defined in it's own header with a nice name of AsyncStatus.
        // Metadata attributes don't have backing code
        if (get_full_type_name(type) == "Windows.Foundation.AsyncStatus" ||
            type.TypeNamespace() == "Windows.Foundation.Metadata" || !can_write(w, type))
        {
            return;
        }
        w.write("public typealias % = %\n", type, bind_type_abi(type));
    }

    static void write_enum_extension(writer& w, TypeDef const& type)
    {
        if (get_full_type_name(type) == "Windows.Foundation.AsyncStatus" ||
            type.TypeNamespace() == "Windows.Foundation.Metadata" || !can_write(w, type))
        {
            return;
        }
        w.write("extension % {\n", get_full_swift_type_name(w, type));
        {
            auto format = R"(    public static var % : % {
        %_%
    }
)";
            for (auto&& field : type.FieldList())
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
                        w.write(format, get_swift_name(field), get_full_swift_type_name(w, type), bind_type_abi(type), field.Name());
                    }
                }
            }
        }
        w.write("}\n\n");
    }

    static void write_generic_typenames(writer& w, std::pair<GenericParam, GenericParam> const& params)
    {
        separator s{ w };

        for (auto&& param : params)
        {
            s();
            w.write("typename %", param);
        }
    }

    static void write_comma_generic_types(writer& w, std::pair<GenericParam, GenericParam> const& params)
    {
        for (auto&& param : params)
        {
            w.write(", %", param);
        }
    }

    static bool can_write(writer& w, TypeDef const& parent, TypeSig const& type)
    {
        TypeDef actual_type{};
        auto category = get_category(type, &actual_type);
        if (category == param_category::object_type || category == param_category::enum_type)
        {
            if (actual_type == TypeDef{}) { return true; }
            if (!can_write(w, actual_type))
            {
                return false;
            }
        }

        // TODO: WIN-65 swiftwinrt: support generic types
        // TODO: WIN-32 swiftwinrt: support array types
        if (category == param_category::generic_type)
        {
            // don't write abi types bc that will put it in a UnsafeMutablePointer<>
            auto non_abi = w.push_abi_types(false);
            // if the writer returns something, then it can be written
            auto written = w.write_temp("%", type);
            auto can_write = !written.empty();
            if (!can_write)
            {
                return false;
            }
        }

        if (category == param_category::array_type)
        {
            return false;
        }

        return true;
    }

    static bool can_write(writer& w, Property const& prop)
    {
        if (!can_write(w, prop.Parent(), prop.Type().Type()))
        {
            return false;
        }
        
        return true;
    }

    static bool can_write(writer& w, generic_inst const& type)
    {
        auto name = type.generic_type_abi_name();
        return name == "IReference" || name == "IEventHandler" || name == "ITypedEventHandler";
    }

    static bool can_write(writer& w, MethodDef const& method, bool allow_special = false)
    {
        // Don't support writing specials (events/properties) unless told to do so (i.e. for vtable)
        if (method.SpecialName() && !allow_special) return false;

        auto method_name = get_swift_name(method);
        method_signature sig{ method };

        // TODO: WIN-30 swiftwinrt: support async/await
        if (sig.is_async()) return false;
        for (auto&& [param, param_signature] : sig.params())
        {
            auto param_name = get_swift_name(param);

            // TODO: WIN-32 swiftwinrt: support array types
            if (param_signature->Type().is_szarray() ||
                param_signature->Type().is_array())
            {
                return false;
            }

            // TODO: support reference parameters
            if (param_signature->ByRef() && is_guid(param_signature->Type()))
            {
                return false;
            }

            if (!can_write(w, method.Parent(), param_signature->Type()))
            {
                return false;
            }
        }

        if (auto return_sig = sig.return_signature())
        {
            if (!can_write(w, method.Parent(), return_sig.Type()))
            {
                return false;
            }
        }

        return true;
    }

    static bool can_write(writer& w, function_def const& function, bool allow_special = false)
    {
        auto method = function.def;

        // Don't support writing specials (events/properties) unless told to do so (i.e. for vtable)
        if (method.SpecialName() && !allow_special) return false;

        if (is_add_overload(method) || is_remove_overload(method)) return false;
        auto method_name = get_swift_name(method);
        method_signature sig{ function };

        // TODO: WIN-30 swiftwinrt: support async/await
        if (sig.is_async()) return false;
        for (auto&& [param, param_signature] : sig.params())
        {
            auto param_name = get_swift_name(param);

            // TODO: WIN-32 swiftwinrt: support array types
            if (param_signature->Type().is_szarray() ||
                param_signature->Type().is_array())
            {
                return false;
            }

            // TODO: support reference parameters
            if (param_signature->ByRef() && is_guid(param_signature->Type()))
            {
                return false;
            }

            if (!can_write(w, method.Parent(), param_signature->Type()))
            {
                return false;
            }
        }

        if (auto return_sig = sig.return_signature())
        {
            if (!can_write(w, method.Parent(), return_sig.Type()))
            {
                return false;
            }
        }

        return true;
    }
    static bool can_write(writer& w, TypeDef const& type)
    {
        auto typeName = get_full_type_name(type);
        if (!w.filter.includes(type)) return false;

        auto category = get_category(type);
        if (category == category::enum_type) return true;

        // TODO: https://linear.app/the-browser-company/issue/WIN-103/swiftwinrt-write-iasyncinfo
        if (type.TypeName() == "IAsyncInfo") return false;
        // TODO: WIN-65 swiftwinrt: support generic types
        auto generics = type.GenericParam();
        if (!empty(generics))
        {
            // don't write abi types bc that will put it in a UnsafeMutablePointer<>
            auto non_abi = w.push_abi_types(false);
            auto written = w.write_temp("%", type);
            auto can_write = !written.empty();
            if (!can_write)
            {
                return false;
            }
        }

        for (auto&& iface : get_interfaces(w, type))
        {
            // don't write abi types bc that will put it in a UnsafeMutablePointer<>
            auto non_abi = w.push_abi_types(false); 
            auto written = w.write_temp("%", iface.second.type);
            auto can_write = !written.empty();
            // TODO: WIN-65 swiftwinrt: support generic types
            if (!can_write)
            {
                return false;
            }
        }

        // TODO: WIN-90 swiftwinrt: support delegate types
        if (category == category::delegate_type)
        {
            return true;
        }

        return true;
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

    static void write_guid(writer& w, TypeDef const& type)
    {
        auto attribute = get_attribute(type, "Windows.Foundation.Metadata", "GuidAttribute");

        if (!attribute)
        {
            throw_invalid("'Windows.Foundation.Metadata.GuidAttribute' attribute for type '", type.TypeNamespace(), ".", type.TypeName(), "' not found");
        }

        auto abi_guard = w.push_abi_types(true);

        auto generics = type.GenericParam();
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
        auto abi_guard = w.push_abi_types(true);

        sha1 signatureHash;
        static constexpr std::uint8_t namespaceGuidBytes[] =
        {
            0x11, 0xf4, 0x7a, 0xd5,
            0x7b, 0x73,
            0x42, 0xc0,
            0xab, 0xae, 0x87, 0x8b, 0x1e, 0x16, 0xad, 0xee
        };
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

    static void write_default_interface(writer& w, TypeDef const& type)
    {
        if (auto default_interface = get_default_interface(type))
        {
            auto format = R"(    template <> struct default_interface<%>{ using type = %; };
)";
            w.write(format, type, default_interface);
        }
    }

    static void write_array_size_name(writer& w, Param const& param)
    {
        w.write(" __%Size", get_swift_name(param));
    }

    static void write_convert_to_abi_arg(writer& w, std::string_view const& param_name, TypeSig const& type, bool is_out)
    {
        TypeDef signature_type;
        auto category = get_category(type, &signature_type);

        if (std::holds_alternative<GenericTypeIndex>(type.Type()))
        {
            auto genericTypeIndex = std::get<GenericTypeIndex>(type.Type());
            auto type = w.generic_param_stack.back()[genericTypeIndex.index];
            switch (type.second)
            {
            case valuetype_copy_semantics::equal:
                w.write(param_name);
                break;
            case valuetype_copy_semantics::blittable:
            {
                auto mangled_name = w.push_mangled_names(true);
                w.write("unsafeBitCast(%, to: %.self)", param_name, type.first);
                break;
            }
            case valuetype_copy_semantics::nonblittable:
                w.write(".init(from: %)", param_name);
                break;
            }
        }
        else if (category == param_category::object_type)
        {
            if (is_interface(type) || is_delegate(type))
            {
                w.write("_%", param_name);
            }
            else
            {
                w.write("RawPointer(%)", param_name);
            }
        }
        else if (is_type_blittable(type))
        {
            if (is_struct(type) && !is_guid(type))
            {
                auto guard = w.push_abi_types(true);
                w.write("unsafeBitCast(%, to: %.self)", param_name, type);
            }
            else
            {
                w.write(param_name);
            }
        }
        else
        {
            if (category == param_category::string_type)
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
                if (!is_out)
                {
                    w.write("_%.val", param_name);
                }
                else
                {
                    w.write("_%.detach()", param_name);
                }
            }
            else if (category == param_category::generic_type)
            {
                // When passing generics to the ABI we wrap them before making the
                // api call for easy passing to the ABI
                w.write("_%", param_name);
            }
            else
            {
                w.write(".init(from: %)", param_name);
            }
        }
    }

    static void write_abi_arg_in(writer& w, Param const& param, TypeSig const& type)
    {
        if (std::holds_alternative<GenericTypeIndex>(type.Type()))
        {
            // TODO: WIN-65 swiftwinrt: support generic types
            XLANG_ASSERT("**TODO: implement write_abi_arg_in**");
        }
        else
        {
            auto push_abi_guard = w.push_abi_types(true);
            auto category = get_category(type);
            if (category == param_category::object_type || category == param_category::string_type || category == param_category::generic_type)
            {
                w.write("_ %: %?", get_swift_name(param), type);
            }
            else
            {
                w.write("_ %: %", get_swift_name(param), type);
            }
        }
    }

    static void write_abi_arg_out(writer& w, Param const& param, TypeSig const& type)
    {
        if (std::holds_alternative<GenericTypeIndex>(type.Type()))
        {
            // TODO: WIN-65 swiftwinrt: support generic types
            XLANG_ASSERT("**TODO: implement write_abi_arg_out**");
        }
        else
        {
            auto category = get_category(type);
            if (category == param_category::object_type || category == param_category::string_type)
            {
                w.write("_ %: inout %?", get_swift_name(param), type);
            }
            else
            {
                w.write("_ %: inout %", get_swift_name(param), type);
            }
        }
    }

    static void write_swift_arg_in(writer& w, std::string_view const& param_name, TypeSig const& type)
    {
        if (std::holds_alternative<GenericTypeIndex>(type.Type()))
        {
            // TODO: WIN-65 swiftwinrt: support generic types
            XLANG_ASSERT("**TODO: implement write_swift_arg_in**");
        }
        else
        {
            w.write("_ %: %", param_name, type);
        }
    }

    static void write_swift_arg_out(writer& w, std::string_view const& param_name, TypeSig const& type)
    {
        if (std::holds_alternative<GenericTypeIndex>(type.Type()))
        {
            // TODO: WIN-65 swiftwinrt: support generic types
            XLANG_ASSERT("**TODO: implement write_swift_arg_out**");
        }
        else
        {
            auto category = get_category(type);
            if (category == param_category::string_type)
            {
                w.write("_ %: inout %?", param_name, type);
            }
            else
            {
                w.write("_ %: inout %", param_name, type);
            }
        }
    }

    static void write_params(writer& w, method_signature const& method_signature)
    {
        separator s{ w };

        for (auto&& [param, param_signature] : method_signature.params())
        {
            s();

            if (param_signature->Type().is_szarray())
            {
                // TODO: WIN-32 swiftwinrt: add support for array types
                XLANG_ASSERT("**TODO: implement szarray in write_params**");
            }
            else
            {
                if (param.Flags().In())
                {
                    if (w.abi_types)
                    {
                        write_abi_arg_in(w, param, param_signature->Type());
                    }
                    else
                    {
                        write_swift_arg_in(w, get_swift_name(param), param_signature->Type());
                    }
                }
                else
                {
                    if (w.abi_types)
                    {
                        write_abi_arg_out(w, param, param_signature->Type());
                    }
                    else
                    {
                        write_swift_arg_out(w, get_swift_name(param), param_signature->Type());
                    }
                }
            }
        }

    }

    static void write_implementation_args(writer& w, method_signature const& method_signature)
    {
        separator s{ w };
        for (auto&& [param, param_signature] : method_signature.params())
        {
            s();
            if (param.Flags().In())
            {
                w.write("%",
                    bind<write_convert_to_abi_arg>(get_swift_name(param), param_signature->Type(), false));
            }
            else
            {
                auto category = get_category(param_signature->Type());
                bool is_blittable = is_type_blittable(param_signature->Type());
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

    static void write_abi_args(writer& w, method_signature const& method_signature)
    {
        separator s{ w };

        w.write("pThis");

        s();
        for (auto&& [param, param_signature] : method_signature.params())
        {
            s();
            if (param.Flags().In())
            {
                w.write(get_swift_name(param));
            }
            else
            {
                w.write("&%", get_swift_name(param));
            }
        }

        if (method_signature.return_signature())
        {
            s();
            auto param_name = method_signature.return_param_name();
            w.write("&%", param_name);
        }
    }

    static void write_consume_type(writer& w, TypeSig const& type, std::string_view const& name)
    {
        TypeDef signature_type{};
        auto category = get_category(type, &signature_type);
        if (std::holds_alternative<GenericTypeIndex>(type.Type()))
        {
            auto genericTypeIndex = std::get<GenericTypeIndex>(type.Type());
            auto type = w.generic_param_stack.back()[genericTypeIndex.index];
            auto category = get_category(type.first, &signature_type);

            if (category == param_category::object_type)
            {
                if (signature_type)
                {
                    auto signature_category = get_category(signature_type);
                    if (signature_category == category::interface_type)
                    {
                        w.write("%.unwrap_from(abi: %)",
                            bind_wrapper_fullname(signature_type),
                            name);
                    }
                    else if (signature_category == category::class_type)
                    {
                        w.write(".from(abi: %)", name);
                    }
                    else if (signature_category == category::delegate_type)
                    {
                        
                    }
                }
                else
                {
                    w.write("%.IInspectable(%)", w.support, name);
                }
            }
            else if (category == param_category::fundamental_type || category == param_category::enum_type)
            {
                w.write(name);
            }
            else if (category == param_category::struct_type && is_struct_blittable(signature_type))
            {
                auto mangled_name = w.push_mangled_names(w.abi_types);
                w.write("unsafeBitCast(%, to: %.self)", name, type.first);
            }
            else
            {
                // other non-blittable types (structs, bool, char, string) all have an initialiazer
                // which converts from the ABI value
                w.write(".init(from: %)", name);
            }
        }
        else if (category == param_category::array_type)
        {
            // TODO: WIN-32 swiftwinrt: add support for array types
            XLANG_ASSERT("**TODO: implement array type in write_consume_return_type");
        }
        else if (category == param_category::object_type)
        {
            if (is_interface(type))
            {
                w.write("%.unwrap_from(abi: %)",
                    bind_wrapper_fullname(signature_type),
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
                    w.write("%.try_unwrap_from(abi: %)", bind_wrapper_name(signature_type), name);
                }
            }
            else if (is_class(type))
            {
                auto format = "%.from(abi: %)";
                w.write(format, get_full_swift_type_name(w, signature_type), name);
            }
            else
            {
                //TODO: implement generic object type
                w.write(".init(%%)",
                    w.consume_types ? "consuming: " : "",
                    name);
            }

        }
        else if (is_type_blittable(type))
        {
            if (category == param_category::struct_type && !is_guid(type))
            {
                auto format = "unsafeBitCast(%, to: %.self)";
                // disambiguate variables with matching name as type. for example
                // public var MouseDelta : MouseDelta { get { ... } }
                auto full_name = w.push_full_type_names(true);
                w.write(format, name, type);

            }
            else
            {
                // fundamental types can just be simply copied to since the types match
                w.write(name);
            }
        }
        else if (w.abi_types && category == param_category::string_type)
        {
            auto format = "try! HString(%).detach()";
            w.write(format, name);
        }
        else if (category == param_category::generic_type)
        {
            auto format = ".init(ref: %)";
            w.write(format, name);
        }
        else
        {
            auto format = ".init(from: %)";
            w.write(format, name);
        }
    }

    static void write_init_val(writer& w, TypeSig const& sig)
    {
        auto category = get_category(sig);
        if (category == param_category::object_type || category == param_category::string_type || category == param_category::generic_type)
        {
            w.write("%?", sig);
        }
        else if (category == param_category::struct_type)
        {
            auto format = "% = .init()";
            w.write(format,
                sig);
        }
        else if (category == param_category::enum_type)
        {
            auto format = "% = .init(0)";
            w.write(format,
                sig);
        }
        else if (is_boolean(sig))
        {
            auto format = "% = %";
            w.write(format,
                sig,
                w.abi_types ? "0" : "false");
        }
        else
        {
            auto format = "% = %";
            w.write(format,
                sig,
                is_floating_point(sig) ? "0.0" : "0");
        }
    }


    static void write_init_return_val(writer& w, RetTypeSig const& signature)
    {
        write_init_val(w, signature.Type());
    }

    static void write_consume_return_statement(writer& w, method_signature const& signature);
    static write_scope_guard<writer> write_local_param_wrappers(writer& w, method_signature const& signature);

    static void write_interface_swift_body(writer& w, winmd::reader::MethodDef method)
    {
        auto indent = w.push_indent({ 2 });

        std::string_view func_name = get_abi_name(method);
        method_signature signature{ method };
        auto return_sig = signature.return_signature();
        {
            auto guard = write_local_param_wrappers(w, signature);

            if (return_sig)
            {
                w.write("let % = try %Impl(%)\n",
                    signature.return_param_name(),
                    func_name,
                    bind<write_implementation_args>(signature));
            }
            else
            {
                w.write("try %Impl(%)\n",
                    func_name,
                    bind<write_implementation_args>(signature));
            }
        }
        if (return_sig)
        {
            w.write("%\n", bind<write_consume_return_statement>(signature));
        }
    }

    static void write_abi_func_body(writer& w, TypeDef const& type, winmd::reader::MethodDef method)
    {
        std::string_view func_name = get_abi_name(method);
        method_signature signature{ method };
        auto indent = w.push_indent({ 2 });
        auto return_sig = signature.return_signature();
        if (return_sig)
        {
            w.write("var %: %\n",
                signature.return_param_name(),
                bind<write_init_return_val>(return_sig));
        }

        w.write(R"(_ = try perform(as: %.self) { pThis in
    try CHECKED(pThis.pointee.lpVtbl.pointee.%(%))
)",
bind_type_abi(type),
func_name,
bind<write_abi_args>(signature));

        w.write("}\n");
        if (return_sig)
        {
            w.write("return %\n", signature.return_param_name());
        }
    }

    static void write_return_type_declaration(writer& w, method_signature signature)
    {
        auto return_sig = signature.return_signature();
        if (!return_sig)
        {
            return;
        }

        auto format = "-> % ";
        w.write(format,
            return_sig);
    }

    static void do_write_interface_abi(writer& w, TypeDef const& type)
    {
        auto factory_info = try_get_factory_info(type);
        const bool is_composable_factory = factory_info.has_value() && factory_info.value().composable;

        if (is_exclusive(type) && !can_write(w, get_exclusive_to(type)))
        {
            return;
        }
        const bool internal = is_composable_factory || can_mark_internal(type);
        auto format = R"(% class %: %.IInspectable% {
)";
        w.write(format,
            internal ? "internal" : "open",
            type,
            w.support,
            is_composable_factory ? ", ComposableActivationFactory": "");

        if (is_composable_factory)
        {
            auto overrides_format = "    internal typealias Composable = %.Composable\n\n";
            w.write(overrides_format, get_full_swift_type_name(w, get_exclusive_to(type)));
        }
      
        auto abi_guard = w.push_abi_types(true);

        auto iid_format = "    override public class var IID: IID { IID_% }\n\n";
        w.write(iid_format, type);


        auto format_method = R"(    % func %Impl(%) throws %{
%    }
)";


        for (auto&& method : type.MethodList())
        {
            if (!can_write(w, method, true)) continue;
            try
            {
                method_signature signature{ method };
                auto guard = w.push_abi_types(true);
                // Composable factories have to have the method be called CreateInstanceImpl, even if the
                // metadata format specifies something else.
                auto func_name = is_composable_factory ? "CreateInstance" : get_abi_name(method);
                w.write(format_method,
                     internal || is_exclusive(type) ? "internal" : "open",
                    func_name,
                    bind<write_params>(signature),
                    bind<write_return_type_declaration>(signature),
                    bind<write_abi_func_body>(type, method));
            }
            catch (std::exception const& e)
            {
                std::string message = e.what();
                message.append("\n method: ").append(get_name(method));
                message.append("\n type: ").append(get_full_type_name(type));
                message.append("\n database: ").append(type.get_database().path());
                printf(message.data());
                /*
                throw_invalid(e.what(),
                    "\n method: ", get_name(method),
                    "\n type: ", type.TypeNamespace(), ".", type.TypeName(),
                    "\n database: ", type.get_database().path());
                    */
            }
        }

        //write_fast_interface_abi(w, type);
        w.write("}\n\n");
    }
    static void write_implementable_interface(writer& w, TypeDef const& type);

    static void write_interface_generic(writer& w, generic_inst const& type)
    {
        if (can_write(w, type))
        {
            type.write_swift_declaration(w);
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
            bool blittable = is_struct_blittable(structType->type());
            w.write(format,
                get_full_swift_type_name(w, structType),
                type.mangled_name(),
                structType->mangled_name(),
                blittable ? 
                        "self = unsafeBitCast(result, to: Self.self)" :
                        "self.init(from: result)");
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

    static void write_interface_abi(writer& w, TypeDef const& type)
    {
        if (!can_write(w, type)) return;
        //auto generics = type.GenericParam();
        //auto guard{ w.push_generic_params(generics) };

        //if (empty(generics))
        
        do_write_interface_abi(w, type);
        if (!is_exclusive(type))
        {
            write_implementable_interface(w, type);
        }
    }
    static void write_vtable(writer& w, TypeDef const& type);

    template <typename T>
    static void write_delegate_wrapper(writer& w, T const& type)
    {
        auto impl_name = w.write_temp("%", bind_impl_fullname(type));

        auto delegate_abi_name = w.write_temp("%", bind_type_abi(type));
        auto wrapper_name = w.write_temp("%", bind_wrapper_name(type));

        auto format = R"(
class % : WinRTWrapperBase<%, %> {
    override class var IID: IID { IID_% }
    init(handler: %){
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
            wrapper_name,
            delegate_abi_name,
            impl_name,
            delegate_abi_name,
            impl_name,
            type,
            delegate_abi_name);
    }

    static void write_generic_delegate_wrapper(writer& w, generic_inst const& generic)
    {
        if (can_write(w, generic))
        {
            write_delegate_wrapper(w, generic);
        }
    }
    
    static void write_delegate_abi(writer& w, TypeDef const& type)
    {
        write_vtable(w, type);
        write_delegate_wrapper(w, type);
    }

    static void write_field_abi(writer& w, Field const& field)
    {
        w.write("        % %;\n", get_field_abi(w, field), get_swift_name(field));
    }
    
    static void write_struct_abi(writer& w, TypeDef const& type)
    {
        bool is_blittable = true;
        for (auto&& field : type.FieldList())
        {
            if (!is_type_blittable(field.Signature().Type()))
            {
                is_blittable = false;
                break;
            }   
        }

        if (is_blittable)
        {
            return;
        }

        w.write("public class _ABI_% {\n", type.TypeName());
        {
            auto push_indent = w.push_indent({ 1 });
            w.write("public var val: % = .init()\n", bind_type_abi(type));
            w.write("public init() { } \n");

            w.write("public init(from swift: %) {\n", get_full_swift_type_name(w, type));
            {
                auto push_abi = w.push_abi_types(true);
                auto indent = w.push_indent({ 1 });
                for (auto&& field : type.FieldList())
                {
                    // WIN-64 - swiftwinrt: support boxing/unboxing
                    // WIN-65 - swiftwinrt: support generic types
                    if (can_write(w, type, field.Signature().Type()))
                    {
                        std::string from = std::string("swift.").append(get_swift_name(field));
                        w.write("val.% = %\n",
                            get_swift_name(field),
                            bind<write_consume_type>(field.Signature().Type(), from)
                        );
                    }
                 
                }
            }
            w.write("}\n\n");

            w.write("public func detach() -> % {\n", bind_type_abi(type));
            {
                auto indent = w.push_indent({ 1 });

                w.write("let result = val\n");
                for (auto&& field : type.FieldList())
                {
                    if (get_category(field.Signature().Type()) == param_category::string_type)
                    {
                        w.write("val.% = nil\n", get_swift_name(field));
                    }
                }
                w.write("return result\n");
            }

            w.write("}\n\n");

            w.write("deinit {\n");
            {
                auto indent = w.push_indent({ 1 });
                for (auto&& field : type.FieldList())
                {
                    if (get_category(field.Signature().Type()) == param_category::string_type)
                    {
                        w.write("WindowsDeleteString(val.%)\n", get_swift_name(field));
                    }
                }
            }
            w.write("}\n");
        }
        w.write("}\n");
        
    }

    static void write_consume_params(writer& w, method_signature const& signature)
    {
        auto indent_guard = w.push_indent({ 1 });
        int param_number = 1;
        auto full_type_names = w.push_full_type_names(true);

        for (auto&& [param, param_signature] : signature.params())
        {
            if (param_signature->Type().is_szarray())
            {
                // TODO: WIN-32 swiftwinrt: add support for arrays
                w.write("**TODO: implement szarray in write_consume_params**");
            }
            else
            {
                std::string param_name = "$" + std::to_string(param_number);

                if (param.Flags().In())
                {
                    assert(!param.Flags().Out());

                    bool isDelegate = is_delegate(param_signature->Type());
                    std::string format = "%let %: % = %%\n";
                    w.write(format,
                        isDelegate ? "guard " : "",
                        get_swift_name(param),
                        param_signature->Type(),
                        bind<write_consume_type>(param_signature->Type(), param_name),
                        isDelegate ? " else { return E_INVALIDARG }" : ""
                    );
                }
                else
                {
                    assert(!param.Flags().In());
                    assert(param.Flags().Out());
                    w.write("var %: %\n",
                        get_swift_name(param), 
                        bind<write_init_val>(param_signature->Type()));
                }
            }
            ++param_number;
        }
    }

    static void write_consume_return_statement(writer& w, method_signature const& signature)
    {
        if (!signature.return_signature())
        {
            return;
        }

        auto return_type = signature.return_signature().Type();
        auto consume_types = w.push_consume_types(true);
        TypeDef signature_type{};
        auto category = get_category(return_type, &signature_type);
        if (is_delegate(return_type))
        {
            w.write("let _% = %.%.try_unwrap_from(abi: %)\n",
                signature.return_param_name(),
                abi_namespace(signature_type),
                bind_wrapper_name(signature_type),
                signature.return_param_name());
        }
     
        w.write("return %", bind<write_consume_type>(return_type, signature.return_param_name()));
    }

    static void write_consume_args(writer& w, method_signature const& signature)
    {
        separator s{ w };

        for (auto&& [param, param_signature] : signature.params())
        {
            s();
            if (param.Flags().In())
            {
                w.write(get_swift_name(param));
            }
            else
            {

                
                w.write("&%", get_swift_name(param));
                
            }
        }
    }

    static void write_vtable(writer& w, TypeDef const& type);

    // PropertyValue is special, it's not an interface/prototype we expose to customers
    // and instead is the glue for boxing types which are of `Any` type in Swift
    static void write_property_value_wrapper(writer& w)
    {
        w.write(R"(public class IPropertyValueWrapper : WinRTWrapperBase<__x_ABI_CWindows_CFoundation_CIPropertyValue, %.IPropertyValue>
{
    override public class var IID: IID { IID___x_ABI_CWindows_CFoundation_CIPropertyValue }
    public init(value: Any) {
        let abi = withUnsafeMutablePointer(to: &IPropertyValueVTable) {
            __x_ABI_CWindows_CFoundation_CIPropertyValue(lpVtbl: $0)
        }
        super.init(abi, __IMPL_Windows_Foundation.IPropertyValueImpl(value: value))
    }

    public init?(impl: %.IPropertyValue?) {
        guard let impl = impl else { return nil }
        let abi = withUnsafeMutablePointer(to: &IPropertyValueVTable) {
            __x_ABI_CWindows_CFoundation_CIPropertyValue(lpVtbl: $0)
        }
        super.init(abi, impl)
    }
}
)", w.support, w.support);
}

    static void write_implementable_interface(writer& w, TypeDef const& type)
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

    static void write_class_impl_func(writer& w, winmd::reader::MethodDef method, interface_info const& iface);
    static void write_class_impl_property(writer& w, winmd::reader::Property prop, interface_info const& iface);
    static void write_class_impl_event(writer& w, winmd::reader::Event event, interface_info const& iface);
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

    static void write_interface_impl(writer& w, TypeDef const& type)
    {
        if (is_exclusive(type) || !can_write(w, type)) return;

        if (get_full_type_name(type) == "Windows.Foundation.IPropertyValue")
        {
            write_property_value_impl(w);
            return;
        }

        auto format = "public class % : %, AbiInterfaceImpl {\n";
        w.write(format, bind_impl_name(type), type);
        {
            {
                auto indent = w.push_indent({ 1 });
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
)",

                    bind_type_abi(type),
                    abi_namespace(type),
                    type,
                    type,
                    bind_impl_name(type));
            
            }

            interface_info info{ type };
            info.is_default = true; // mark as default so we use the name "interface"
            for (auto&& method : type.MethodList())
            {
                write_class_impl_func(w, method, info);
            }
            for (auto&& prop : type.PropertyList())
            {
                write_class_impl_property(w, prop, info);
            }

            for (auto&& [interface_name, info] : get_interfaces(w, type))
            {
                if (!can_write(w, info.type)) { continue; }

                if (!info.is_default || info.base)
                {
                    w.write("    internal lazy var %: %.% = try! _default.QueryInterface()\n",
                        get_swift_name(info),
                        abi_namespace(info.type),
                        info.type.TypeName());
                }

                for (auto&& method : info.type.MethodList())
                {
                    write_class_impl_func(w, method, info);
                }

                for (auto&& prop : info.type.PropertyList())
                {
                    write_class_impl_property(w, prop, info);
                }
            }

        }
        w.write("}\n\n");
    }

    static void write_interface_proto(writer& w, TypeDef const& type)
    {
        if (is_exclusive(type) || !can_write(w, type))
        {
            return;
        }
        auto format = R"(public protocol %Prototype : IWinRTObject { %
}
)";
        auto typeName = type.TypeName();

        w.write(format, type, bind([&](writer& w)
            {
                for (auto& method : type.MethodList())
                {
                    if (!can_write(w, method)) continue;

                    auto full_type_name = w.push_full_type_names(true);
                    method_signature signature{ method };
                    w.write("\n        func %(%) %",
                        get_swift_name(method),
                        bind<write_params>(signature),
                        bind<write_return_type_declaration>(signature));
                }

                for (auto& prop : type.PropertyList())
                {
                    if (!can_write(w, prop)) continue;
                    auto full_type_name = w.push_full_type_names(true);
                    auto [_, setter] = get_property_methods(prop);
                    w.write("\n        var %: % { get% }",
                        get_swift_name(prop),
                        prop.Type().Type(),
                        setter ? " set" : "");
                }
            }));

        // don't write this extension for property value since it isn't used publically
        if (get_full_type_name(type) == "Windows.Foundation.IPropertyValue")
        {
            return;
        }
        auto extension_format = R"(extension %Prototype {
    public static var none: %Prototype {
        %(nil)
    }
}

)";
        w.write(extension_format,
            type,
            type, 
            bind_impl_fullname(type));
    }

    static void write_interface_alias(writer& w, TypeDef const& type)
    {
        if (is_exclusive(type) || !can_write(w, type))
        {
            return;
        }
        auto name = type.TypeName();
        w.write("public typealias % = %%.%Prototype\n\n",
            name,
            bind_each([&](writer& w, std::pair<std::string, interface_info> const& iface) {
                // TODO: https://linear.app/the-browser-company/issue/WIN-103/swiftwinrt-write-iasyncinfo
                if (!iface.first.ends_with("IAsyncInfo"))
                {
                    w.write("% & ", iface.second.type);
                }
                    }, get_interfaces(w, type)),
             get_swift_module(type),
            name);
    }

    static void write_ireference(writer& w)
    {
        w.write(R"(public protocol IReference : IPropertyValue {
    var Value: Any { get }
}
)");
    }

    static void write_delegate(writer& w, TypeDef const& type)
    {
        w.write("public typealias % = %\n", type, bind_impl_name(type));
    }

    static void write_delegate_return_type(writer& w, method_signature const& sig)
    {
        if (sig.return_signature())
        {
            w.write("%", sig.return_signature().Type());
        }
        else
        {
            w.write("()");
        }
    }

    static void write_comma_param_types(writer& w, std::vector<std::pair<Param, ParamSig const*>> const& params)
    {
        separator s{ w };
        for (auto& [param, sig] : params)
        {
            s();
            w.write(sig->Type());
        }
    }

    template <typename T>
    static void do_write_delegate_implementation(writer& w, T const& type, MethodDef const& invoke_method)
    {
        method_signature signature{ invoke_method };
        auto return_sig = signature.return_signature();
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
        auto data = w.write_temp("%", bind<write_comma_param_types>(signature.params()));
        auto return_type = w.write_temp("%", bind<write_delegate_return_type>(signature));
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

    static void write_delegate_implementation(writer& w, TypeDef const& type)
    {
        if (can_write(w, type))
        {
            auto delegate_method = get_delegate_method(type);
            do_write_delegate_implementation(w, type, delegate_method);
        }
    }

    static void write_generic_implementation(writer& w, generic_inst const& type)
    {
        if (!is_delegate(type) || !can_write(w, type)) return;

        auto generics_guard = w.push_generic_params(type);
        auto delegate_method = type.functions[0].def;
        do_write_delegate_implementation(w, type, delegate_method);
    }
    static write_scope_guard<writer> write_local_param_wrappers(writer& w, method_signature const& signature)
    {
         // This code isn't ideal, but due to the difference in strings between Swift and WinRT
         // we need to allocate an HString on the stack so that the lifetime is properly handled.
         // If we pass the HString in directly to a function like this:
         //        interface.load(HString(url).get()).
         // It doesn't work because the lifetime of the HString is shorter than the method
         // it's passed into and so the string is destroyed before the load method is called.
         // 
         //  Also because the strings are different, we can't create something like an HStringReference,
         //  because the underlying buffers are differnet, we have to allocate a new HString to hold the
         //  memory.
        
        write_scope_guard guard{ w };

        for (auto&& [param, param_signature] : signature.params())
        {
            TypeDef signature_type{};
            auto category = get_category(param_signature->Type(), &signature_type);

            if (param.Flags().In())
            {
                if (category == param_category::string_type)
                {
                    w.write("let _% = try! HString(%)\n",
                        get_swift_name(param),
                        get_swift_name(param));
                }
                else if (category == param_category::struct_type && !is_type_blittable(param_signature->Type()))
                {
                    w.write("let _% = %._ABI_%(from: %)\n",
                        get_swift_name(param),
                        abi_namespace(signature_type),
                        signature_type.TypeName(),
                        get_swift_name(param));
                }
                else if (is_interface(param_signature->Type()))
                {
                    w.write("let %Wrapper = %.%(impl: %)\n",
                        get_swift_name(param),
                        abi_namespace(signature_type),
                        bind_wrapper_name(signature_type),
                        get_swift_name(param));
                    w.write("let _% = try! %Wrapper?.to_abi { $0 }\n",
                        get_swift_name(param),
                        get_swift_name(param));
                }
                else if (is_delegate(param_signature->Type()))
                {
                    w.write("let %Wrapper = %.%(handler: %)\n",
                        get_swift_name(param),
                        abi_namespace(signature_type),
                        bind_wrapper_name(signature_type),
                        get_swift_name(param));
                    w.write("let _% = try! %Wrapper.to_abi { $0 }\n",
                        get_swift_name(param),
                        get_swift_name(param));
                }
                else if (category == param_category::generic_type)
                {
                    auto swift_name = w.write_temp("%", param_signature->Type());
                    if (swift_name.find("?") != swift_name.npos)
                    {
                        w.write("let %Wrapper = %.%(value: %)\n",
                            get_swift_name(param),
                            abi_namespace(w.type_namespace), // generics are never public, so use the abi namespace of the current namespace
                            bind<write_wrapper_type_sig>(param_signature->Type()),
                            get_swift_name(param));
                        w.write("let _% = try! %Wrapper?.to_abi { $0 }\n",
                            get_swift_name(param),
                            get_swift_name(param));
                    }
                    else
                    {
                        w.write("let %Handler = %(handler: %)\n",
                            get_swift_name(param),
                            bind<write_impl_type_sig>(param_signature->Type()),
                            get_swift_name(param));
                        w.write("let %Wrapper = %.%(handler: %Handler)\n",
                            get_swift_name(param),
                            abi_namespace(w.type_namespace), // generics are never public, so use the abi namespace of the current namespace
                            bind<write_wrapper_type_sig>(param_signature->Type()),
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
                    is_type_blittable(param_signature->Type()) &&
                    !is_guid(param_signature->Type()))
                {
                    w.write("var _%: % = .init()\n",
                        get_swift_name(param),
                        bind_type_abi(signature_type));
                    guard.push("% = unsafeBitCast(_%, to: %.self)\n",
                        get_swift_name(param),
                        get_swift_name(param),
                        get_full_swift_type_name(w, signature_type));
                }
                else if (category == param_category::struct_type)
                {
                    w.write("let _%: %._ABI_% = .init()\n",
                        get_swift_name(param),
                        abi_namespace(signature_type),
                        signature_type.TypeName());
                    guard.push("% = .init(from: _%.val)\n",
                        get_swift_name(param),
                        get_swift_name(param));
                }
                else if (category == param_category::boolean_type || category == param_category::character_type)
                {
                    w.write("var _%: % = .init()\n",
                        get_swift_name(param),
                        bind<write_type_sig_abi>(param_signature->Type()));
                    guard.push("% = .init(from: _%)\n",
                        get_swift_name(param),
                        get_swift_name(param));
                }
                else if (category == param_category::object_type)
                {
                    w.write("var _%: %?\n",
                        get_swift_name(param),
                        bind<write_type_sig_abi>(param_signature->Type()));
                    if (is_interface(param_signature->Type()))
                    {
                        w.write("%(%)", bind_impl_name(signature_type), get_swift_name(param));
                    }
                    else if (auto default_interface = get_default_interface(param_signature->Type()))
                    {
                        auto [ns, default_interface_name] = get_type_namespace_and_name(default_interface);
                        guard.push("% = .init(%.%(consuming: _%))\n",
                            get_swift_name(param),
                            abi_namespace(ns),
                            default_interface_name,
                            get_swift_name(param));
                    }
                }
            }
        }

        return guard;
    }


    static void write_factory_body(writer& w, MethodDef const& method, interface_info const& factory, TypeDef const& type, coded_index<TypeDefOrRef> const& default_interface)
    {
        auto indent_guard = w.push_indent(indent{ 1 });

        std::string_view func_name = get_abi_name(method);
        method_signature signature{ method };

        auto swift_name = get_swift_name(factory);

        {
            auto guard = write_local_param_wrappers(w, signature);

            w.write("let % = try! Self.%.%Impl(%)\n",
                signature.return_param_name(),
                swift_name,
                func_name,
                bind<write_implementation_args>(signature));
        }
        w.write("_default = %.%(consuming: %)\n", abi_namespace(type), default_interface, signature.return_param_name());
        if (auto base_class = get_base_class(type))
        {
            w.write("super.init(fromAbi: try! _default.QueryInterface())\n");
        }
    }

    static void write_factory_constructors(writer& w, TypeDef const& factory, TypeDef const& type, coded_index<TypeDefOrRef> const& default_interface)
    {
        if (factory)
        {
            auto guard = w.push_indent(indent{ 1 });

            interface_info factory_info{ factory };
            auto swift_name = get_swift_name(factory_info);
            w.write(R"(private static let %: %.% = try! RoGetActivationFactory(HString("%"))
)",
swift_name,
abi_namespace(factory),
factory,
get_full_type_name(type));

            for (auto&& method : factory.MethodList())
            {
                if (!can_write(w, method)) continue;
                method_signature signature{ method };
                w.write(R"(public init(%) {
%}

)", 
                    bind<write_params>(signature),
                    bind<write_factory_body>(method, factory_info, type, default_interface)
                    );
            }
        }
        else
        {
            auto base_class = get_base_class(type);
            auto base_class_init = base_class ? "\n        super.init(fromAbi: try! _default.QueryInterface())" : "";
                w.write(R"^-^(    %public init() {
        try! _default = RoActivateInstance(HString("%"))%
    }

)^-^", base_class ? "override " : "",
get_full_type_name(type), base_class_init);

        }
        
    }

    static void write_composable_constructor(writer& w, TypeDef const& factory, TypeDef const& type)
    {
        // this factory doesn't have any constructor, don't generate methods. this is a pattern done
        // by the Microsoft.UI.Composition types where they don't want the app to derive from their types,
        // but they do provide a hierarchy
        if (!factory.MethodList().first)
        {
            return;
        }
        w.write("    private static var _% : %.% =  try! RoGetActivationFactory(HString(\"%\"))\n",
            factory,
            abi_namespace(factory),
            factory, 
            get_full_type_name(type));

        auto base_class = get_base_class(type);
        if (!base_class)
        {
            auto base_composable_init = R"(    public init() {
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
            auto override_composable_init = R"(    override public init() {
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

    static void write_default_constructor_declarations(writer& w, TypeDef const& type, coded_index<TypeDefOrRef> const& default_interface)
    {
        auto [ns, name] = get_type_namespace_and_name(default_interface);
        auto base_class = get_base_class(type);
        bool composable = is_composable(type);

        // We unwrap composable types to try and get to any derived type. If not composable, then create a new
        // instance
        auto from_abi = composable ? "UnsealedWinRTClassWrapper<Composable>.unwrap_from(base: abi!)" :
            ".init(fromAbi: .init(abi))";

        w.write(R"(    public static func from(abi: UnsafeMutablePointer<%>?) -> % {
         %
    }
)",
bind_type_abi(default_interface),
type,
from_abi);

       
        auto base_class_init = base_class ? "\n        super.init(fromAbi: try! _default.QueryInterface())" : "";
        w.write(R"^-^(    internal init(fromAbi: %.%) {
        _default = fromAbi%
    }

)^-^",
    abi_namespace(ns),
                name,
                base_class_init);

    }

    static void write_class_definitions(writer& w, TypeDef const& type)
    {
        if (settings.component_opt && settings.component_filter.includes(type))
        {
            return;
        }

        auto type_name = type.TypeName();
    }

    static void write_class_func_body(writer& w, winmd::reader::MethodDef method, interface_info const& iface)
    {
        auto indent_guard = w.push_indent(indent{ 1 });

        std::string_view func_name = get_abi_name(method);
        method_signature signature{ method };

        auto return_sig = signature.return_signature();

        {
            auto guard = write_local_param_wrappers(w, signature);

            auto impl = get_swift_name(iface);

            if (return_sig)
            {
                w.write("let % = try! %.%Impl(%)\n",
                    signature.return_param_name(),
                    impl,
                    func_name,
                    bind<write_implementation_args>(signature));
            }
            else
            {
                w.write("try! %.%Impl(%)\n",
                    impl,
                    func_name,
                    bind<write_implementation_args>(signature));
            }
        }
        if (return_sig)
        {
            w.write("%\n", bind<write_consume_return_statement>(signature));
        }
    }

    static void write_class_impl_property(writer& w, winmd::reader::Property prop, interface_info const& iface)
    {
        if (!can_write(w, prop)) return;

        auto format = "    public %var % : % {\n";
        auto [getter, setter] = get_property_methods(prop);

        auto propName = get_swift_name(prop);

        // TODO: https://linear.app/the-browser-company/issue/WIN-82/support-setters-not-defined-in-same-api-contract-as-getters
        // right now require that both getter and setter are defined in the same version
        if (getter)
        {
            //auto full_type_name_guard = w.push_full_type_names(true);
            w.write(format,
                iface.attributed ? "static " : "",
                propName,
                prop.Type().Type());
        }


        auto impl = get_swift_name(iface);

        if (getter)
        {
            auto format = R"(get {
    let % = try! %.%Impl()
    %
}

)";
            auto guard = w.push_indent(indent{ 2 });
            method_signature signature{ getter };
            w.write(format,
                signature.return_param_name(),
                impl,
                get_swift_name(getter),
                bind<write_consume_return_statement>(signature));
        }

        // TODO: https://linear.app/the-browser-company/issue/WIN-82/support-setters-not-defined-in-same-api-contract-as-getters
        // right now require that both getter and setter are defined in the same version
        if (setter && getter)
        {
            auto format = R"(set {
    %try! %.%Impl(%) 
}
)";
            auto guard = w.push_indent(indent{ 2 });
            std::string extra_init;

            if (!is_type_blittable(prop.Type().Type()))
            {
                TypeDef signature_type{};
                auto category = get_category(prop.Type().Type(), &signature_type);
                if (category == param_category::string_type)
                {
                    extra_init = "let _newValue = try! HString(newValue)\n    ";
                }
                else if (category == param_category::struct_type)
                {
                    extra_init = w.write_temp("let _newValue = %._ABI_", abi_namespace(signature_type))
                        .append(signature_type.TypeName())
                        .append("(from: newValue)\n    ");
                }
                else if (is_interface(prop.Type().Type()))
                {
                    extra_init = w.write_temp("let wrapper = %.%(impl: newValue)\n    ",
                        abi_namespace(signature_type),
                        bind_wrapper_name(signature_type));
                    extra_init.append("let _newValue = try! wrapper?.to_abi { $0 }\n    ");
                }
                else if (is_delegate(prop.Type().Type()))
                {
                    extra_init = w.write_temp("let wrapper = %.%(handler: newValue)\n    ",
                        abi_namespace(signature_type),
                        bind_wrapper_name(signature_type));
                    extra_init.append("let _newValue = try! wrapper.to_abi { $0 }\n    ");
                }
                else if (category == param_category::generic_type)
                {
                    extra_init = w.write_temp("let wrapper = %.%(value: newValue)\n    ",
                        abi_namespace(w.type_namespace), // generics are never public, so use the abi namespace of the current namespace
                        bind<write_wrapper_type_sig>(prop.Type().Type()));
                    extra_init.append("let _newValue = try! wrapper?.to_abi { $0 }\n    ");
                }
            }

            w.write(format,
                extra_init,
                impl,
                get_swift_name(setter),
                bind<write_convert_to_abi_arg>("newValue", prop.Type().Type(), false));
        }
        
        // TODO: https://linear.app/the-browser-company/issue/WIN-82/support-setters-not-defined-in-same-api-contract-as-getters
        // right now require that both getter and setter are defined in the same version
        if (getter)
        {
            w.write("    }\n\n");
        }
    }

    static void write_class_impl_func(writer& w, winmd::reader::MethodDef method, interface_info const& iface)
    {
        if (method.SpecialName() || !can_write(w, method))
        {
            // don't write methods which are really properties
            return;
        }

        auto format = R"(% func %(%) %{
%}

)";
        method_signature signature{ method };

        auto return_sig = signature.return_signature();
        auto guard = w.push_indent(indent{ 1 });
        w.write(format,
                iface.overridable ? "open" : "public",
            get_swift_name(method),
            bind<write_params>(signature),
            bind<write_return_type_declaration>(signature),
            bind<write_class_func_body>(method, iface));
    }
    
    static void write_event_registrar_name(writer& w, winmd::reader::Event event)
    {
        w.write("%Registrar", event.Name());
    }

    static void write_event_registrar(writer& w, winmd::reader::Event const& event, interface_info const& iface)
    {
        auto abi_name = w.write_temp("%.%", abi_namespace(iface.type), iface.type.TypeName());
        auto format = R"(    private class % : IEventRegistration {
        func add(delegate: any WinRTDelegate, for impl: %.IInspectable){
            let wrapper = %(handler: delegate as! %)
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
            bind<write_event_registrar_name>(event), // class %Registrar
            w.support, // %.IInspectable
            bind_event_delegate_wrapper_fullname(event), // let wrapper = %
            bind_event_delegate_impl_fullname(event), // as! %
            abi_name,  // let impl:%
            event.Name(), // delegate.token = try! impl.add_%
            w.support, // %.IInspectable
            abi_name,  // let impl:%
            event.Name()); //try! impl.remove_%
        }

    static void write_class_impl_event(writer& w, winmd::reader::Event event, interface_info const& iface)
    {
        auto format = R"(    private static let _% = %()
    public % var % : Event<(%),%> = EventImpl<%>(register: %_%, owner:%)
)";
        auto registrar_name = w.write_temp("%", bind<write_event_registrar_name>(event));
        auto type = find_type(event.EventType());
        writer::generic_param_guard guard{};
        if (is_generic(event.EventType()))
        {
            auto type_signature = event.EventType().TypeSpec().Signature();
            auto inst = type_signature.GenericTypeInst();
            guard = w.push_generic_params(inst);
        }
        auto delegate_method = get_delegate_method(type);
        method_signature delegate_sig{ delegate_method };
        w.write(format,
            registrar_name, // private static let _% 
            registrar_name, // Impl.%()
            iface.attributed ? "static" : "lazy", // public %
            event.Name(), // var %
            bind<write_comma_param_types>(delegate_sig.params()), // Event<(%)
            bind<write_delegate_return_type>(delegate_sig), // , %>
            bind_event_delegate_impl_fullname(event), // EventImpl<%>
            iface.attributed ? "" : "Self.", // register:%
            registrar_name, // _%
            get_swift_name(iface) // owner:%
        );

        write_event_registrar(w, event, iface);
    }

    static void write_statics_body(writer& w, MethodDef const& method, TypeDef const& statics, TypeDef const& type)
    {
        auto indent_guard = w.push_indent(indent{ 1 });

        std::string_view func_name = get_abi_name(method);
        method_signature signature{ method };
        auto return_sig = signature.return_signature();

        {
            auto guard = write_local_param_wrappers(w, signature);

            if (return_sig)
            {
                w.write("let % = try! _%.%Impl(%)\n",
                    signature.return_param_name(),
                    statics.TypeName(),
                    func_name,
                    bind<write_implementation_args>(signature));
            }
            else
            {
                w.write("try! _%.%Impl(%)\n",
                    statics.TypeName(),
                    func_name,
                    bind<write_implementation_args>(signature));
            }
        }
        if (return_sig)
        {
            w.write("%\n", bind<write_consume_return_statement>(signature));
        }
    }

    static void write_static_methods(writer& w, attributed_type const& statics, TypeDef const& type)
    {
        if (statics.type)
        {
            interface_info static_info{ statics.type };
            auto impl_name = get_swift_name(static_info);
            w.write("    private static let %: %.% = try! RoGetActivationFactory(HString(\"%\"))\n",
                impl_name,
                abi_namespace(statics.type),
                statics.type.TypeName(),
                get_full_type_name(type));
            auto guard = w.push_indent(indent{ 1 });

            for (auto&& method : statics.type.MethodList())
            {
                if (method.SpecialName() || !can_write(w, method))
                {
                    continue;
                }

                //auto full_type_names = w.push_full_type_names(true);
                method_signature signature{ method };
                w.write(R"(public static func %(%) %{
%}

)", get_swift_name(method),
bind<write_params>(signature),
bind<write_return_type_declaration>(signature),
bind<write_statics_body>(method, statics.type, type)
);

            }
        }

        for (auto&& static_prop : statics.type.PropertyList())
        {
            interface_info static_info{ statics.type };
            static_info.attributed = true;
            write_class_impl_property(w, static_prop, static_info);
        }

        for (auto&& event : statics.type.EventList())
        {
            interface_info static_info{ statics.type };
            static_info.attributed = true;
            write_class_impl_event(w, event, static_info);
        }
    }

    static void write_composable_impl(writer& w, TypeDef const& parent, TypeDef const& overrides, bool compose)
    {
        auto default_interface = get_default_interface(parent);
        if (!default_interface)
        {
            throw_invalid("Could not find default interface for %s\n", parent.TypeName().data());
        }

        bool use_iinspectable_vtable = type_name(overrides) == type_name(default_interface);

        auto format = R"(    internal class % : ComposableImpl {
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
            overrides.TypeName(),
            bind([&](writer& w) {
                if (use_iinspectable_vtable)
                {
                    write_type_abi(w, ElementType::Object);
                }
                else
                {
                    write_type_abi(w, overrides);
                }}), 
                bind([&](writer& w) {
                    if (use_iinspectable_vtable)
                    {
                        w.write(ElementType::Object);
                    }
                    else
                    {
                        w.write("%.%", abi_namespace(overrides), overrides.TypeName());
                    }}),
            parent,
            bind_type_abi(default_interface),
            abi_namespace(parent),
            default_interface);

        if (compose)
        {
            w.write("    internal typealias Composable = %\n", overrides.TypeName());
            w.write("    %public class var _makeFromAbi : any MakeFromAbi.Type { Composable.Default.self }\n",
                get_base_class(parent) ? "override " : "");

        }
    }   

    static void write_class_interface_impl(writer& w, interface_info const& info)
    {
        if (info.overridable)
        {
            // when implementing default overrides, we want to call to the inner non-delegating IUnknown
            // as this will get us to the inner object. otherwise we'll end up with a stack overflow 
            // because we'll be calling the same method on ourselves
            w.write("    internal lazy var %: %.% = try! IUnknown(_inner).QueryInterface()\n",
                get_swift_name(info),
                abi_namespace(info.type),
                info.type.TypeName());

        }
        else if (!info.is_default)
        {
            w.write("    internal lazy var %: %.% = try! _default.QueryInterface()\n",
                get_swift_name(info),
                abi_namespace(info.type),
                info.type.TypeName());
        }

        for (auto&& method : info.type.MethodList())
        {
            write_class_impl_func(w, method, info);
        }

        for (auto&& prop : info.type.PropertyList())
        {
            write_class_impl_property(w, prop, info);
        }

        for (auto&& event : info.type.EventList())
        {
            write_class_impl_event(w, event, info);
        }


    }
    static void write_class_impl(writer& w, TypeDef const& type)
    {
        if (!can_write(w, type)) return;

        auto default_interface = get_default_interface(type);
        auto typeName = type.TypeName();
        
        auto base_class = get_base_class(type);
        bool composable = is_composable(type);
        auto modifier = composable ? "open" : "public final";

        if (base_class)
        {
            w.write("% class %: % {\n", modifier, typeName, get_full_swift_type_name(w, base_class));
        }
        else if (default_interface)
        {
            auto base_class = composable ? "UnsealedWinRTClass" : "WinRTClass";
            w.write("% class %: % {\n", modifier, typeName, base_class);
        }
        else
        {
            w.write("public final class % {\n", typeName);
        }

        if (composable && !base_class)
        {
            w.write("    private (set) public var _inner: UnsafeMutablePointer<%.IInspectable>?\n", w.c_mod);
        }

        if (default_interface)
        {
            auto [ns, name] = get_type_namespace_and_name(default_interface);
            w.write(R"(    private typealias swift_ABI = %.%
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
            abi_namespace(ns), 
            name,
            bind_type_abi(default_interface),
                base_class ? 
                            composable ? "override open" : 
                                         "override public" :
                            composable ? "open" :
                                         "public",
                w.c_mod,
                base_class ? "super._get_abi()" : "nil");
            write_default_constructor_declarations(w, type, default_interface);
        }
        for (auto&& [interface_name, factory] : get_attributed_types(type))
        {
            if (factory.activatable)
            {
                write_factory_constructors(w, factory.type, type, default_interface);
            }

            if (factory.statics)
            {
                write_static_methods(w, factory, type);
            }

            if (factory.composable)
            {
                write_composable_constructor(w, factory.type, type);
            }
        }

        bool has_overrides = false;
        for (auto&& [interface_name, info] : get_interfaces(w, type))
        {
            if (!can_write(w, info.type)) { continue; }

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
                write_composable_impl(w, type, info.type, compose);
                has_overrides = true;
            }
            else if (info.overridable && info.base && !has_overrides)
            {
                // This unsealed class doesn't have an overridable interface of it's own, so use the first base
                // interface we come across for the composable implementation. This is used by the factory method
                // when we are creating an aggregated type and enables the app to override the base class methods
                // on this type
                const bool compose = true;
                write_composable_impl(w, type, info.type, compose);
                has_overrides = true;
            }

        }

        if (composable && !has_overrides && default_interface)
        {
            auto default_type = find_required(default_interface);
            write_composable_impl(w, type, default_type, true);
        }
        w.write("}\n\n");
    }

    static void write_class(writer& w, TypeDef const& type)
    {
        write_class_impl(w, type);
    }

    static void write_initializable_interface(writer& w, TypeDef const& type)
    {
        w.write(R"(extension % : Initializable {
    public init() {
        let vtblPtr = withUnsafeMutablePointer(to: &%.%VTable) { $0 }
        self.init(lpVtbl: vtblPtr)
    }
}
)", bind_type_abi(type),
abi_namespace(type),
type);
    }

    static void write_initializable_overrides(writer& w, TypeDef const& type)
    {
        if (!is_composable(type))
        {
            return;
        }

        for (auto&& [interface_name, info] : get_interfaces(w, type))
        {
            if (!info.overridable || info.base) { continue; }

            write_initializable_interface(w, info.type);
        }
    }

    static void write_initializable_implementable(writer& w, TypeDef const& type)
    {
        if (!can_write(w, type) || is_exclusive(type))
        {
            return;
        }

        write_initializable_interface(w, type);
    }

    static void write_struct_initializer_params(writer& w, TypeDef const& type)
    {
        separator s{ w };

        for (auto&& field : type.FieldList())
        {
            // WIN-64 - swiftwinrt: support boxing/unboxing
            // WIN-65 - swiftwinrt: support generic types
            if (can_write(w, type, field.Signature().Type()))
            {
                s();

                w.write("%: %", get_swift_name(field), field.Signature().Type());
            }
           
        }
    }

    static void write_struct(writer& w, TypeDef const& type)
    {
        if (type.TypeName() == "HResult" && type.TypeNamespace() == "Windows.Foundation")
        {
            return;
        }
        w.write("public struct % {\n", type);
        {
            bool is_blittable = true;
            auto indent = w.push_indent({ 1 });
            for (auto&& field : type.FieldList())
            {
                auto field_type = field.Signature().Type();
                is_blittable = is_blittable && is_type_blittable(field_type);
                // WIN-64 - swiftwinrt: support boxing/unboxing
                // WIN-65 - swiftwinrt: support generic types
                if (!can_write(w, type, field_type)) continue;

                w.write("public var %: %\n", 
                    get_swift_name(field), 
                    bind<write_init_val>(field_type));
            }

            w.write("public init() {}\n");
            w.write("public init(%) {\n", bind<write_struct_initializer_params>(type));
            {
                auto indent = w.push_indent({ 1 });
                for (auto&& field : type.FieldList())
                {
                    // WIN-64 - swiftwinrt: support boxing/unboxing
                    // WIN-65 - swiftwinrt: support generic types
                    if (can_write(w, type, field.Signature().Type()))
                    {
                        auto field_name = get_swift_name(field);
                        w.write("self.% = %\n", field_name, field_name);
                    }
                }
            }
            w.write("}\n");

            if (!is_blittable)
            {
                w.write("public init(from abi: %) {\n", bind_type_abi(type));
                {
                    auto indent = w.push_indent({ 1 });
                    for (auto&& field : type.FieldList())
                    {
                        if (can_write(w, type, field.Signature().Type()))
                        {
                            std::string from = std::string("abi.").append(get_swift_name(field));
                            w.write("self.% = %\n",
                                get_swift_name(field),
                                bind<write_consume_type>(field.Signature().Type(), from)
                            );
                        }
                    }
                }
                w.write("}\n");
            }
 
        }
        w.write("}\n\n");
    }

    template <typename T>
    static void write_iunknown_methods(writer& w, T const& type, get_interfaces_t const& interfaces, bool composed = false)
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
    guard let inner = %(impl: instance) else { return E_INVALIDARG }
    let pThis = try! inner.to_abi { $0 }
    return pThis.pointee.lpVtbl.pointee.QueryInterface(pThis, riid, ppvObject)
}
)";
                type_name typeName(type);
                // workaround right now for the fact that if the relationship isn't specified in metadata
                
                if (typeName.name == "IApplicationOverrides" && typeName.name_space == "Microsoft.UI.Xaml")
                {
                    auto metadata_provider = swiftwinrt::find_required(type, "Microsoft.UI.Xaml.Markup.IXamlMetadataProvider");
                    auto cast = w.write_temp(" as? %", metadata_provider);

                    w.write(format,
                        bind_wrapper_fullname(metadata_provider), // if riid.pointee == %
                        bind_wrapper_name(type), // guard let instance = %
                        cast,
                        bind_wrapper_fullname(metadata_provider)); // let inner = %
                }
                for (auto&& iface : interfaces)
                {
                    // TODO: https://linear.app/the-browser-company/issue/WIN-103/swiftwinrt-write-iasyncinfo
                    if (iface.first.ends_with("IAsyncInfo")) continue;
                    bool is_property_value = iface.first == "Windows.Foundation.IPropertyValue";
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
        auto interfaces = get_interfaces(w, type.generic_type()->type());
        write_iunknown_methods(w, type, interfaces);
    }
    
    template <typename T>
    static void write_iinspectable_methods(writer& w, T const& type, get_interfaces_t const& interfaces, bool composed = false)
    {
        // 3 interfaces for IUnknown, IInspectable, type.
        auto interface_count = 3 + interfaces.size();
        w.write(R"(GetIids: {
    let size = MemoryLayout<IID>.size
    let iids = CoTaskMemAlloc(UInt64(size) * %).assumingMemoryBound(to: IID.self)
    iids[0] = IUnknown.IID
    iids[1] = IInspectable.IID
    iids[2] = %.IID
    %
    $1!.pointee = %
    $2!.pointee = iids
    return S_OK
},

)",
interface_count,
bind_wrapper_fullname(type),
bind([&](writer& w) {
                auto format = "iids[%] = %.%.IID\n";
                auto iface_n = 4;
                for (auto&& iface : interfaces)
                {
                    // TODO: https://linear.app/the-browser-company/issue/WIN-103/swiftwinrt-write-iasyncinfo
                    if (iface.first.ends_with("IAsyncInfo")) continue;

                    w.write(format,
                        iface_n++,
                        abi_namespace(iface.second.type),
                        bind_wrapper_name(iface.second.type)
                    );
                }}),
            interface_count
                    );

        if (composed)
        {
            // for composed types, get the swift object and grab the typename
            w.write(R"(GetRuntimeClassName: {
    guard let instance = %.try_unwrap_from(raw: $0) else { return E_INVALIDARG }
    let hstring = instance.GetRuntimeClassName().detach()
    $1!.pointee = hstring
    return S_OK
},

)",
bind_wrapper_name(type)
);
        }
        else
        {
            w.write(R"(GetRuntimeClassName: {
    _ = $0
    let hstring = try! HString("%").detach()
    $1!.pointee = hstring
    return S_OK
},

)",
get_full_type_name(type)
);
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
        auto interfaces = get_interfaces(w, type.generic_type()->type());
        write_iinspectable_methods(w, type, interfaces);
    }
    
    static void do_write_abi_val_assignment(writer& w, TypeSig const& type_sig, std::string_view const& param_name, std::string_view const& return_param_name)
    {
        TypeDef type{};
        auto category = get_category(type_sig, &type);
        auto is_blittable = is_type_blittable(type_sig);
        if (category == param_category::struct_type && !is_blittable)
        {
            w.write("let _% = %._ABI_%(from: %)\n\t",
                param_name,
                abi_namespace(type),
                type.TypeName(),
                param_name);
        }
        else if (is_interface(type_sig))
        {
            w.write("let %Wrapper = %.%(impl: %)\n",
                param_name,
                abi_namespace(type),
                 bind_wrapper_name(type),
                param_name);
            w.write("let _% = try! %Wrapper?.to_abi { $0 }\n",
                param_name,
                param_name);
        }
        else if (category == param_category::generic_type)
        {
            w.write("let %Wrapper = %.%(value: %)\n",
                param_name,
                abi_namespace(w.type_namespace),
                bind<write_wrapper_type_sig>(type_sig),
                param_name);
            w.write("let _% = try! %Wrapper?.to_abi { $0 }\n",
                param_name,
                param_name);
        }
        else if (is_delegate(type_sig))
        {
            w.write("let %Wrapper = %.%(handler: %)\n",
                param_name,
                abi_namespace(type),
                bind_wrapper_name(type),
                param_name);
            w.write("let _% = try! %Wrapper.to_abi { $0 }\n",
                param_name,
                param_name);
        }

        w.write("%?.initialize(to: %)\n",
            return_param_name,
            bind<write_convert_to_abi_arg>(param_name, type_sig, true)
        );
    }

    static void write_abi_ret_val(writer& w, method_signature signature)
    {
        separator s{ w, "\n"};
        int param_number = 1;
        for (auto&& [param, param_signature] : signature.params())
        {
            if (param.Flags().Out())
            {
                auto return_param_name = "$" + std::to_string(param_number);
                auto param_name = get_swift_name(param);
                do_write_abi_val_assignment(w, param_signature->Type(), param_name, return_param_name);
            }
            param_number++;
        }

        if (signature.return_signature())
        {
            auto return_param_name = "$" + std::to_string(signature.params().size() + 1);
            do_write_abi_val_assignment(w, signature.return_signature().Type(), signature.return_param_name(), return_param_name);
        }
    }

    static void write_consume_swift_ret_val(writer& w, method_signature signature)
    {
        if (!signature.return_signature())
        {
            return;
        }

        w.write("let % = ",
            signature.return_param_name());
    }

    static void write_not_implementable_vtable_method(writer& w, method_signature const& sig)
    {
        w.write("%: { _, % in return E_NOTIMPL }", get_abi_name(sig.method()), bind([&](writer& w) {
            separator s{ w };
            for (auto& [_, param_sig] : sig.params())
            {
                s();
                if (param_sig->Type().is_szarray())
                {
                    w.write("_, ");
                }
                w.write("_");
            }
            if (sig.return_signature())
            {
                s();
                if (sig.return_signature().Type().is_szarray())
                {
                    w.write("_, ");
                }
                w.write("_");
            }}));
    }

    static void write_vtable_method(writer& w, MethodDef const& method, TypeDef const& type)
    {
        auto func_name = get_abi_name(method);
        auto signature = method_signature(method);

        // https://linear.app/the-browser-company/issue/WIN-104/swiftwinrt-support-authoring-winrt-events-in-swift
        if (!can_write(w, method, true) || 
            is_add_overload(method) || 
            is_remove_overload(method))
        {
            write_not_implementable_vtable_method(w, signature);
            return;
        }

        // we have to write the methods in a certain order and so we will have to detect here whether
        // this method is a property getter/setter and adjust the call so we use the swift syntax
        std::string func_call;
        if (is_get_overload(method))
        {
            func_call = w.write_temp("%", method.Name().substr(4));
        }
        else if (is_put_overload(method))
        {
            func_call = w.write_temp("% = %", method.Name().substr(4), bind<write_consume_args>(signature));
        }
        else
        {
            // delegate arg types are a tuple, so wrap in an extra paranthesis
            auto format = get_category(type) == category::delegate_type ? "%((%))" : "%(%)";
   
            func_call = w.write_temp(format, get_swift_name(method), bind<write_consume_args>(signature));
        }

        w.write(R"(%: {
    guard let __unwraped__instance = %.%.try_unwrap_from(raw: $0) else { return E_INVALIDARG }
%
    %__unwraped__instance.%
    %
    return S_OK
})", 
            func_name,
            abi_namespace(type),
            bind_wrapper_name(type),
            bind<write_consume_params>(signature),
            bind<write_consume_swift_ret_val>(signature),
            func_call,
            bind<write_abi_ret_val>(signature)
    );
    }

    static void write_vtable_method(writer& w, function_def const& function, generic_inst const& type)
    {
        auto method = function.def;
        auto func_name = get_abi_name(method);
        auto signature = method_signature(function);

        // we have to write the methods in a certain order and so we will have to detect here whether
        // this method is a property getter/setter and adjust the call so we use the swift syntax
        std::string func_call;
        if (is_get_overload(method))
        {
            func_call = w.write_temp("%", method.Name().substr(4));
            if (type.generic_type()->swift_full_name().starts_with("Windows.Foundation.IReference"))
            {
                func_call.append(w.write_temp(" as! %", get_full_swift_type_name(w, type.generic_params()[0])));
            }
        }
        else if (is_put_overload(method))
        {
            func_call = w.write_temp("% = %", method.Name().substr(4), bind<write_consume_args>(signature));
        }
        else
        {
            // delegate arg types are a tuple, so wrap in an extra paranthesis
            auto format = is_delegate(type) ? "%((%))" : "%(%)";
            func_call = w.write_temp(format, get_swift_name(method), bind<write_consume_args>(signature));
        }

        w.write(R"(%: {
    guard let __unwraped__instance = %.try_unwrap_from(raw: $0) else { return E_INVALIDARG }
%
    %__unwraped__instance.%
    %
    return S_OK
})",
func_name,
bind_wrapper_name(type),
bind<write_consume_params>(signature),
bind<write_consume_swift_ret_val>(signature),
func_call,
bind<write_abi_ret_val>(signature)
);
    }

    static void write_vtable(writer& w, TypeDef const& type)
    {
        w.write("fileprivate static var %VTable: %Vtbl = .init(\n",
            type,
            bind_type_abi(type));
        
        {
            auto indent = w.push_indent({ 1 });
            write_iunknown_methods(w, type, get_interfaces(w, type));
            separator s{ w, ",\n\n" };

            if (get_category(type) != category::delegate_type)
            {
                write_iinspectable_methods(w, type, get_interfaces(w, type));
                s(); // get first separator out of the way for no-op
            }

            for (auto&& method : type.MethodList())
            {
                if (method.Name() != ".ctor")
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


    static void write_overrides_vtable(writer& w, TypeDef const& type, interface_info const& overrides, get_interfaces_t const& other_interfaces)
    {
        w.write("internal typealias % = UnsealedWinRTClassWrapper<%.%>\n",
            bind_wrapper_name(overrides.type),
            get_full_swift_type_name(w, type),
            overrides.type
        );
        w.write("fileprivate static var %VTable: %Vtbl = .init(\n",
            overrides.type,
            bind_type_abi(overrides.type));

        {
            auto indent = w.push_indent({ 1 });
            write_iunknown_methods(w, overrides.type, other_interfaces, true);
            write_iinspectable_methods(w, overrides.type, other_interfaces, true);

            separator s{ w, ",\n\n" };
            s(); // get first separator out of the way for no-op

            for (auto&& method : overrides.type.MethodList())
            {
                if (method.Name() != ".ctor")
                {
                    s();
                    write_vtable_method(w, method, overrides.type);
                }

            }
        }

        w.write(R"(
)
)");
    }

    static void write_class_abi(writer& w, TypeDef const& type)
    {
        if (!is_composable(type))
        {
            return;
        }

        get_interfaces_t other_composable_interfaces;
        auto interfaces = get_interfaces(w, type);
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
