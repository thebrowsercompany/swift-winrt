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

    static void write_version_assert(writer& w)
    {
        w.write_root_include("base");
        auto format = R"(static_assert(winrt::check_version(CPPWINRT_VERSION, "%"), "Mismatched C++/WinRT headers.");
#define CPPWINRT_VERSION "%"
)";
        w.write(format, SWIFTWINRT_VERSION_STRING, SWIFTWINRT_VERSION_STRING);
    }

    static void write_type_abi(writer& w, TypeDef const& type)
    {
        auto push_abi = w.push_abi_types(true);
        // IAsync info is special in that it is defined in it's own header file and doesn't have a mangled name.
        if (type.TypeName() == "IAsyncInfo" && type.TypeNamespace() == "Windows.Foundation")
        {
            w.write("C%.%", w.support, type);
        }
        else
        {
            w.write("%", type);
        }
    }

    static void write_type_sig_abi(writer& w, TypeSig const& type)
    {
        auto push_abi = w.push_abi_types(true);
        w.write("%", type);
    }

    static void write_impl_name(writer& w, TypeDef const& type)
    {
        std::string implName = w.write_temp("Impl.%Impl", type.TypeName());
        if (w.type_namespace != type.TypeNamespace() || w.full_type_names)
        {
            w.write("%.%", get_swift_namespace(w, type), implName);
        }
        else
        {
            w.write(implName);
        }
    }

    static bool can_write(writer& w, TypeDef const& type, bool* can_write_members = nullptr);

    static void write_enum_def(writer& w, TypeDef const& type)
    {
        // Async status is defined in it's own header with a nice name of AsyncStatus.
        // Metadata attributes don't have backing code
        if (get_full_type_name(type) == "Windows.Foundation.AsyncStatus" ||
            type.TypeNamespace() == "Windows.Foundation.Metadata" || !can_write(w, type))
        {
            return;
        }
        w.write("public typealias % = %\n", type, bind<write_type_abi>(type));
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
                    w.write(format, get_swift_name(field), get_full_swift_type_name(w, type), type, field.Name());
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


    static void write_comma_generic_typenames(writer& w, std::pair<GenericParam, GenericParam> const& params)
    {
        for (auto&& param : params)
        {
            w.write(", typename %", param);
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
            if (actual_type == TypeDef{}) { return false; }
            if (!can_write(w, actual_type))
            {
                return false;
            }
        }

        // TODO: WIN-65 swiftwinrt: support generic types
        // TODO: WIN-32 swiftwinrt: support array types
        if (category == param_category::generic_type ||
            category == param_category::array_type)
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

    static bool can_write(writer& w, MethodDef const& method, bool allow_special = false)
    {
        // TODO: WIN-65 swiftwinrt: support generic types
        auto generics = method.GenericParam();
        if (!empty(generics))
        {
            return false;
        }

        // Don't support writing specials (events/properties) unless told to do so (i.e. for vtable)
        if (method.SpecialName() && !allow_special) return false;

        if (is_add_overload(method) || is_remove_overload(method)) return false;
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

    static bool can_write(writer& w, TypeDef const& type, bool* can_write_members)
    {
        auto typeName = get_full_type_name(type);

        if (!w.filter.includes(type)) return false;

        auto category = get_category(type);
        if (category == category::enum_type) return true;

        // TODO: WIN-65 swiftwinrt: support generic types
        auto generics = type.GenericParam();
        if (!empty(generics))
        {
            return false;
        }

        for (auto&& iface : get_interfaces(w, type))
        {
            // TODO: WIN-65 swiftwinrt: support generic types
            if (!iface.second.generic_param_stack.empty())
            {
                return false;
            }
        }

        // TODO: WIN-90 swiftwinrt: support delegate types
        if (category == category::delegate_type)
        {
            return false;
        }

        if (can_write_members)
        {
            *can_write_members = true;
            for (auto&& method : type.MethodList())
            {
                if (!can_write(w, method, true))
                {
                    *can_write_members = false;
                    // even if we can't write members, still return true because
                    // we can still use these types created by WinRT APIs, we just
                    // can't author them in swift
                    return true;
                }
            }

            for (auto&& prop : type.PropertyList())
            {
                if (!can_write(w, prop))
                {
                    *can_write_members = false;
                    // even if we can't write members, still return true because
                    // we can still use these types created by WinRT APIs, we just
                    // can't author them in swift
                    return true;
                }
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

    static void write_guid(writer& w, TypeDef const& type)
    {
        auto attribute = get_attribute(type, "Windows.Foundation.Metadata", "GuidAttribute");

        if (!attribute)
        {
            throw_invalid("'Windows.Foundation.Metadata.GuidAttribute' attribute for type '", type.TypeNamespace(), ".", type.TypeName(), "' not found");
        }

        if (!can_write(w, type)) return;

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
        if (std::holds_alternative<GenericTypeIndex>(type.Type()))
        {
            // TODO: WIN-65 swiftwinrt: support generic types
            XLANG_ASSERT("**TODO: implement write_convert_to_abi_arg**");
        }
        else if (is_interface(type))
        {
            w.write("RawPointer(%)", param_name);
        }
        else if (is_class(type))
        {
            w.write("RawPointer(%.interface)", param_name);
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
            auto category = get_category(type);
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
            if (category == param_category::object_type || category == param_category::string_type)
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

        if (category == param_category::array_type)
        {
            // TODO: WIN-32 swiftwinrt: add support for array types
            XLANG_ASSERT("**TODO: implement array type in write_consume_return_type");
        }
        else if (category == param_category::object_type)
        {
            if (is_interface(type))
            {
                w.write("%(%)", bind<write_impl_name>(signature_type), name);
            }
            else if (auto default_interface = get_default_interface(type))
            {
                auto [ns, default_interface_name] = get_type_namespace_and_name(default_interface);
                auto format = ".init(%.%(%%))";
                w.write(format,
                    abi_namespace(ns),
                    default_interface_name,
                    w.consume_types ? "consuming: " : "",
                    name);
            }
            else
            {
                //TODO: implement generic object type
                w.write("IInspectable(%%)",
                    w.consume_types ? "consuming: " : "",
                    name);
            }
           
        }
        else if (category == param_category::generic_type)
        {
            // TODO: WIN-65 swiftwinrt: support generic types
            XLANG_ASSERT("**TODO: implement generic type in write_consume_return_type");
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
        else
        {
            auto format = ".init(from: %)";
            w.write(format, name);
        }
    }

    static void write_init_return_val(writer& w, RetTypeSig const& signature)
    {
        auto category = get_category(signature.Type());
        if (category == param_category::object_type || category == param_category::string_type)
        {
            w.write(signature);
        }
        else if (category == param_category::struct_type)
        {
                auto format = "% = .init()";
                w.write(format,
                    signature.Type());
        }
        else if (category == param_category::enum_type)
        {
            auto format = "% = .init(0)";
            w.write(format,
                signature.Type());
        }
        else
        {
            auto format = "% = %";
            w.write(format,
                signature.Type(),
                is_floating_point(signature.Type()) ? "0.0" : "0");
        }
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

        auto return_sig = signature.return_signature();
        if (return_sig)
        {
            w.write("%var %: %\n",
                indent{ 2 },
                signature.return_param_name(),
                bind<write_init_return_val>(return_sig));
        }

        w.write(R"(%_ = try perform(as: %.self) { pThis in
%try CHECKED(pThis.pointee.lpVtbl.pointee.%(%))
)",
indent{ 2 },
bind<write_type_abi>(type),
indent{ 3 },
func_name,
bind<write_abi_args>(signature));

        w.write("%}\n", indent{ 2 });
        if (return_sig)
        {
            w.write("%return %\n", indent{ 2 }, signature.return_param_name());
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

    static void write_interface_abi(writer& w, TypeDef const& type)
    {
        if (!can_write(w, type)) return;
        //auto generics = type.GenericParam();
        //auto guard{ w.push_generic_params(generics) };

        //if (empty(generics))
        
        const bool internal = can_mark_internal(type);

        auto format = R"(% class %: %.IInspectable {
)";
        w.write(format,
            internal ? "internal" : "open",
            type,
            w.support);

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
                w.write(format_method,
                    internal || is_exclusive(type) ? "internal" : "open",
                    get_abi_name(method),
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

    static void write_delegate_abi(writer& w, TypeDef const& type)
    {
        // TODO: WIN-90
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
            w.write("public var val: % = .init()\n", bind<write_type_abi>(type));
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

            w.write("public func detach() -> % {\n", bind<write_type_abi>(type));
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
        //auto full_type_names = w.push_full_type_names(true);

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

                    std::string format = "let %:% = %\n";
                    w.write(format,
                        get_swift_name(param),
                        param_signature->Type(),
                        bind<write_consume_type>(param_signature->Type(), param_name)
                    );
                }
                else
                {
                    assert(!param.Flags().In());
                    assert(param.Flags().Out());
                    auto category = get_category(param_signature->Type());

                    if (category == param_category::string_type || category == param_category::object_type)
                    {
                        w.write("var %: %?\n",
                            get_swift_name(param),
                            param_signature->Type());
                    }
                    else if (category == param_category::struct_type)
                    {
                        w.write("var %: % = .init()\n",
                            get_swift_name(param),
                            param_signature->Type());
                    }
                    else if (category == param_category::enum_type)
                    {
                        w.write("var %: % = .init(0)\n",
                            get_swift_name(param),
                            param_signature->Type());
                    }
                    else if (is_boolean(param_signature->Type()))
                    {
                        auto format = "var %: % = false\n";
                        w.write(format,
                            get_swift_name(param),
                            param_signature->Type());
                    }
                    else
                    {
                        w.write("var %: % = %\n",
                            get_swift_name(param),
                            param_signature->Type(),
                            is_floating_point(param_signature->Type()) ? "0.0" : "0");
                    }
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

    static void write_wrapper_type(writer& w, TypeDef const& type)
    {
        w.write("%Base", type);
    }

    static void write_vtable(writer& w, TypeDef const& type);
    static void write_interface_impl(writer& w, TypeDef const& type)
    {
        bool canWriteMembers = false;
        bool canWrite = can_write(w, type, &canWriteMembers);
        if (is_exclusive(type) || !canWrite)
        {
            return;
        }
        // Only write vtables for interfaces which are implementable
        // by the app, so skip those which are exclusive

      
        std::string implementeable;
       
        if (canWriteMembers)
        {
            write_vtable(w, type);

            auto implementeable_format = R"(
        public init() {
            self.instance = withUnsafeMutablePointer(to: &Impl.%VTable) {
                ComObject(comInterface: %(lpVtbl: $0))
            }

            // Subtle: This makes the IUnknownRef point to the ComClass instance, which the RawPointer helper
            // then unwraps using 'RawPointer' when calling back into the C++ code. This is needed
            // to hand off the raw vtable (with access to this wrapping class) without Swift stomping
            // the vtable with things like the retain count.
            super.init(withUnsafeMutablePointer(to: &instance) { $0 })
            self.instance.wrapper = Unmanaged<%>.passUnretained(self)
        })";
            // If we can't write the vtable since we don't support this type, then don't let the
            // app implement this interface. this will still allow for types from windows to be
            // used
            implementeable = w.write_temp(implementeable_format,
                type,
                bind<write_type_abi>(type),
                bind<write_wrapper_type>(type)
                );
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
        //   struct MyObjectVTable {
        //     ... AddRef, Release, QI, ...
        //     HRESULT (STDMETHODCALLTYPE * Foo)(IMyInterface* pThis, int number)
        //   }
        // 
        //   struct IMyInterface {
        //     const MyObjectVTable* lpVtbl;
        //   }
        //
        // so in Swift we're using the pattern:
        //
        //   private var myInterfaceVTable: IMyInterface {
        //     Foo: {
        //       // In C, 'pThis' is always the first param
        //       guard let wrapper = MyObject.from($0) else {
        //         return E_INVALIDARG
        //       }
        //       let number = $1
        //       instance.Foo(number)
        //     }
        //   }
        //   ...
        //   private struct ComObject {
        //     var comInterface: IMyInterface
        //     var wrapper: MyInterfaceBase
        //   }
        w.write(R"(open class %: %.% {
    private struct ComObject {
        var comInterface: %
        var wrapper: Unmanaged<%>?
    }
     private var instance: ComObject
     %

    deinit {
        // nil out the wrapper, so that we don't try to decrememnt the ref count in the `Release` method, this
        // causes an infinite loop
        self.instance.wrapper = nil
    }

    public init(_ pointer: UnsafeMutablePointer<%>?) {
        if let pointee = pointer?.pointee {
            self.instance = ComObject(comInterface: pointee)
            super.init(withUnsafeMutablePointer(to: &instance) { $0 })

            // try to get the original wrapper so we can get the apps implementation. if that doesn't
            // exist, then we know this points to a C++ object and we will just use ourselves as the
            // wrapper    
            let delegate = IUnknown(pointer)
            let wrapperOpt: ISwiftImplemented? = try? delegate.QueryInterface()
            if let wrapper = wrapperOpt,
            let pUnk = UnsafeMutableRawPointer(wrapper.pUnk.borrow)   {
                self.instance.wrapper = pUnk.bindMemory(to: %.ComObject.self, capacity: 1).pointee.wrapper
            }
        } else {
            self.instance = ComObject(comInterface: .init())
            super.init(pointer)
        }
    }

    private convenience init(empty pointer:UnsafeMutablePointer<%>?)
    {
        self.init(pointer)
    }

    public static var none: % = %Impl(empty: nil)
    required public init(_ pointer: UnsafeMutablePointer<WinSDK.IUnknown>?) { fatalError("should never be called") }
    required public init(consuming pointer: UnsafeMutablePointer<WinSDK.IUnknown>?) { fatalError("should never be called") }

    fileprivate static func from(_ pUnk: UnsafeMutableRawPointer?) -> Unmanaged<%>? {
        return pUnk?.assumingMemoryBound(to: %.ComObject.self).pointee.wrapper
        }
}

)",
bind<write_wrapper_type>(type),
abi_namespace(type),
type,
bind<write_type_abi>(type),
bind<write_wrapper_type>(type),
implementeable,
bind<write_type_abi>(type),
bind<write_wrapper_type>(type),
bind<write_type_abi>(type),
type,
type,
bind<write_wrapper_type>(type),
bind<write_wrapper_type>(type)
);


        auto format = "internal class %Impl : % {\n";
        w.write(format, type, type);
        {
            auto indent = w.push_indent({ 1 });
            for (auto&& method : type.MethodList())
            {
                if (!can_write(w, method, true)) continue;
                method_signature signature{ method };
                auto method_format = R"(public func %(%) throws %{
%    }
)";
                auto push_abi_false = w.push_abi_types(false);
                auto full_type_names = w.push_full_type_names(true);
                w.write(method_format,
                    get_swift_name(method),
                    bind<write_params>(signature),
                    bind<write_return_type_declaration>(signature),
                    bind<write_interface_swift_body>(method));
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
        auto format = R"(public protocol %Proto { %
}
)";
        auto typeName = type.TypeName();

        w.write(format, type, bind([&](writer& w)
            {
                for (auto& method : type.MethodList())
                {
                    if (!can_write(w, method, true)) continue;

                    auto full_type_name = w.push_full_type_names(true);
                    method_signature signature{ method };
                    w.write("\n        func %(%) throws %",
                        get_swift_name(method),
                        bind<write_params>(signature),
                        bind<write_return_type_declaration>(signature));
                }
            }));
    }

    static void write_interface_alias(writer& w, TypeDef const& type)
    {
        if (is_exclusive(type) || !can_write(w, type))
        {
            return;
        }

        w.write("public typealias % = Impl.% & %Proto\n\n",
            type,
            bind<write_wrapper_type>(type),
            type);

    }

    static void write_delegate(writer& w, TypeDef const& type)
    {
        // TODO: WIN-90
    }

    static void write_delegate_implementation(writer& w, TypeDef const& type)
    {
        // TODO: WIN-90
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
                        bind<write_type_abi>(signature_type));
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
                else if (category == param_category::fundamental_type && !is_type_blittable(param_signature->Type()))
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
                        w.write("%(%)", bind<write_impl_name>(signature_type), get_swift_name(param));
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
        w.write("interface = %.%(consuming: %)\n", abi_namespace(type), default_interface, signature.return_param_name());
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
        
    }

    static void write_default_constructor_declarations(writer& w, TypeDef const& type, coded_index<TypeDefOrRef> const& default_interface)
    {
        auto [ns, name] = get_type_namespace_and_name(default_interface);
        w.write(R"^-^(    public init() {
        try! interface = RoActivateInstance(HString("%"))
    }

)^-^",
get_full_type_name(type));

            w.write(R"^-^(    public init(_ fromInterface: %.%) {
        interface = fromInterface
    }

)^-^",
    abi_namespace(ns),
                name);

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

        auto format = R"(public func %(%) %{
%}

)";
        method_signature signature{ method };
       // auto full_type_name_guard = w.push_full_type_names(true);

        auto return_sig = signature.return_signature();
        auto guard = w.push_indent(indent{ 1 });
        w.write(format,
            get_swift_name(method),
            bind<write_params>(signature),
            bind<write_return_type_declaration>(signature),
            bind<write_class_func_body>(method, iface));
    }
    
    static void write_equatable(writer& w, std::string_view const& type_name) {
      auto format =  R"(    public static func == (_ lhs: %, _ rhs: %) -> Bool {
        return lhs.interface == rhs.interface
    }
)";
      w.write(format, type_name, type_name);
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
    }

    static void write_class_impl(writer& w, TypeDef const& type)
    {
        if (!can_write(w, type)) return;

        auto default_interface = get_default_interface(type);
        auto typeName = type.TypeName();

        if (default_interface)
        {
            w.write("public class %: Equatable {\n", typeName);
        }
        else
        {
            w.write("public class % {\n", typeName);
        }

        if (default_interface)
        {
            auto [ns, name] = get_type_namespace_and_name(default_interface);
            w.write("    public var interface: %.%\n\n", abi_namespace(ns), name);
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
        }

        for (auto&& [interface_name, info] : get_interfaces(w, type))
        {
            if (!can_write(w, info.type)) { continue; }

            if (!info.is_default || info.base)
            {
                w.write("    internal lazy var %: %.% = try! interface.QueryInterface()\n",
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

        if (default_interface)
        {
            write_equatable(w, typeName);
        }

        w.write("}\n\n");
    }

    static void write_class(writer& w, TypeDef const& type)
    {
        write_class_impl(w, type);
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

                auto category = get_category(field_type);
                if (category == param_category::object_type || category == param_category::string_type)
                {
                    auto format = "public var %: %? = nil\n";
                    w.write(format, get_swift_name(field), field_type);
                }
                else if (category == param_category::struct_type)
                {
                    auto format = "public var %: % = .init()\n";
                    w.write(format, get_swift_name(field), field_type);
                }
                else if (is_boolean(field_type))
                {
                    auto format = "public var %: % = false\n";
                    w.write(format,
                        get_swift_name(field),
                        field_type);
                }
                else if (category == param_category::enum_type)
                {
                    auto format = "public var % : % = .init(0)\n";
                    w.write(format,
                        get_swift_name(field),
                        field_type);
                }
                else
                {
                    auto format = "public var %: % = %\n";
                    w.write(format,
                        get_swift_name(field),
                        field_type, is_floating_point(field_type) ? "0.0" :  "0");
                }
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
                w.write("public init(from abi: %) {\n", bind<write_type_abi>(type));
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

    static void write_iunknown_methods(writer& w, TypeDef const& type)
    {
        w.write(R"(QueryInterface: {
    guard let pUnk = $0, let riid = $1, let ppvObject = $2 else { return E_INVALIDARG }
    guard riid.pointee == IUnknown.IID ||
          riid.pointee == IInspectable.IID || 
          riid.pointee == ISwiftImplemented.IID ||
          riid.pointee == %.%.IID else { 
        ppvObject.pointee = nil
        return E_NOINTERFACE
    }
    _ = pUnk.pointee.lpVtbl.pointee.AddRef(pUnk)
    ppvObject.pointee = UnsafeMutableRawPointer(pUnk)
    return S_OK
},

)",
            abi_namespace(type),
            type
        );

        w.write(R"(AddRef: {
     guard let wrapper = %.from($0) else { return 1 }
     _ = wrapper.retain()
     return ULONG(_getRetainCount(wrapper.takeUnretainedValue()))
},

)",
            bind<write_wrapper_type>(type)
        );

        w.write(R"(Release: {
    guard let wrapper = %.from($0) else { return 1 }
    return ULONG(_getRetainCount(wrapper.takeRetainedValue()))
},

)",
            bind<write_wrapper_type>(type)
);
    }
    
    static void write_iinspectable_methods(writer& w, TypeDef const& type)
    {
        auto interfaces = get_interfaces(w, type);
       // 3 interfaces for IUnknown, IInspectable, type.
        auto interface_count = 3;
        w.write(R"(GetIids: {
    let size = MemoryLayout<IID>.size
    let iids = CoTaskMemAlloc(UInt64(size) * %).assumingMemoryBound(to: IID.self)
    iids[0] = IUnknown.IID
    iids[1] = IInspectable.IID
    iids[2] = %.%.IID
    $1!.pointee = %
    $2!.pointee = iids
    return S_OK
},

)",
            interface_count,
            abi_namespace(type),
            type,
            interface_count
        );

        w.write(R"(GetRuntimeClassName: {
    _ = $0
    let hstring = try! HString("%").detach()
    $1!.pointee = hstring
    return S_OK
},

)",
get_full_type_name(type)
);

        w.write(R"(GetTrustLevel: {
    _ = $0
    $1!.pointee = TrustLevel(rawValue: 0)
    return S_OK
})"
);
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

    static void write_vtable_method(writer& w, MethodDef const& method, TypeDef const& type)
    {
        auto func_name = get_abi_name(method);
  
        auto signature = method_signature(method);
        w.write(R"(%: {
    guard let wrapper = %.from($0) else { return E_INVALIDARG }
    guard let instance = wrapper.takeUnretainedValue() as? %Proto else { return E_NOINTERFACE }
%
    %try! instance.%(%)
    %
    return S_OK
})", 
            func_name,
            bind<write_wrapper_type>(type),
            type,
            bind<write_consume_params>(signature),
            bind<write_consume_swift_ret_val>(signature),
            get_swift_name(method),
            bind<write_consume_args>(signature),
            bind<write_abi_ret_val>(signature)
    );
    }

    static void write_vtable(writer& w, TypeDef const& type)
    {
        w.write("private static var %VTable: %Vtbl = .init(\n",
            type,
            bind<write_type_abi>(type));
        
        {
            auto indent = w.push_indent({ 1 });
            write_iunknown_methods(w, type);
            write_iinspectable_methods(w, type);

            separator s{ w, ",\n\n"};
            s(); // get first separator out of the way for no-op
            for (auto&& method : type.MethodList())
            {
                s();
                write_vtable_method(w, method, type);
            }
        }

        w.write(R"(
)
)");
    }

}
