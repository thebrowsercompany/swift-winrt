#pragma once
#include "code_writers/common_writers.h"
#include "code_writers/type_writers.h"
#include "code_writers/can_write.h"
#include "metadata_cache.h"
namespace swiftwinrt
{
    static void write_enum_def(writer& w, enum_type const& type)
    {
        // Metadata attributes don't have backing code
        if (type.swift_logical_namespace() == "Windows.Foundation.Metadata" || !can_write(w, type))
        {
            return;
        }

        write_documentation_comment(w, type);
        w.write("public typealias % = %\n", type, bind_type_mangled(type));
    }

    static void write_enum_extension(writer& w, enum_type const& type)
    {
        if (type.swift_logical_namespace() == "Windows.Foundation.Metadata" || !can_write(w, type))
        {
            return;
        }
        w.write("extension % {\n", get_full_swift_type_name(w, type));
        {
            auto format = R"(    public static var % : % {
        %_%
    }
)";
            for (const auto& field : type.type().FieldList())
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
        w.write("}\n");

        w.write("extension %: ^@retroactive Hashable, ^@retroactive Codable, ^@retroactive ^@unchecked Sendable {}\n\n", get_full_swift_type_name(w, type));
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
        auto mangled = w.push_abi_types(true);
        auto guid = attribute.Value().FixedArgs();
        auto format = R"(private var IID_%: %.IID {
    .init(%)// %
}

)";

        w.write(format,
            type,
            w.support,
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
        auto format = R"(private var IID_%: %.IID {
    .init(%)// %
}

)";

        w.write(format,
            type.mangled_name(),
            w.support,
            bind<write_guid_value_hash>(iidHash),
            bind<write_guid_comment_hash>(iidHash));
    }

    static void write_array_size_name(writer& w, Param const& param)
    {
        w.write(" __%Size", get_swift_name(param));
    }
    static void write_function_params2(writer& w, std::vector<function_param> const& params, write_type_params const& type_params)
    {
        separator s{ w };

        for (const auto& param : params)
        {
            s();
            w.write("_ %: ", get_swift_name(param));
            if (param.out()) w.write("inout ");
            const bool is_array = param.is_array();
            if (is_array && type_params.layer == projection_layer::swift)
            {
                // Can't allow for implicit unwrap in arrays
                w.write("[%]", bind<write_type>(*param.type, write_type_params::swift));
            }
            else
            {
                write_type(w, *param.type, type_params);
            }
        }
    }
    static void write_function_params(writer& w, function_def const& function, write_type_params const& type_params)
    {
        write_function_params2(w, function.params, type_params);
    }

    template <typename Param>
    static void write_convert_to_abi_arg(writer& w, Param const& param)
    {
        TypeDef signature_type;
        auto type = param.type;
        auto param_name = param.name;
        auto is_out = param.out();

        auto category = get_category(type, &signature_type);

        auto local_name = local_swift_param_name(param_name);
        if (param.is_array())
        {
            if (is_out)
            {
                w.write("%");
            }
            else
            {
                // Arrays are all converted from the swift array to a c array, so they
                // use the local_param_name
                w.write("count, %", local_name);
            }
        }
        else if (category == param_category::object_type)
        {
            if (is_out) throw std::exception("out parameters of reference types should not be converted directly to abi types");

            if (is_class(signature_type))
            {
                w.write("RawPointer(%)", param_name);
            }
            else
            {
                w.write("%", local_name);
            }
        }
        else if (category == param_category::string_type)
        {
            if (!is_out)
            {
                w.write("%.get()", local_name);
            }
            else
            {
                auto format = "try! HString(%).detach()";
                w.write(format, param_name);
            }
        }
        else if (category == param_category::struct_type)
        {
            if (!is_out && type->swift_type_name() == "EventRegistrationToken")
            {
                w.write(param_name);
            }
            else if (is_struct_blittable(signature_type))
            {
                w.write(".from(swift: %)", param_name);
            }
            else
            {
                if (!is_out)
                {
                    w.write("%.val", local_name);
                }
                else
                {
                    w.write("%.detach()", local_name);
                }
            }
        }
        else if(category == param_category::generic_type)
        {
            if (is_out) throw std::exception("out parameters of generic types should not be converted directly to abi types");
            // When passing generics to the ABI we wrap them before making the
            // api call for easy passing to the ABI
            w.write("%", local_name);
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

        for (const auto& param : function.params)
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
    }

    static void write_abi_args(writer& w, function_def const& function)
    {
        separator s{ w };

        w.write("pThis");
        s();
        for (auto& param: function.params)
        {
            auto param_name = get_swift_name(param);
            auto local_param_name = local_swift_param_name(param_name);
            s();
            if (param.is_array())
            {
                if (param.in())
                {
                    w.write("%.count, %.start", local_param_name, local_param_name);
                }
                else if (param.signature.ByRef())
                {
                    w.write("&%.count, &%.start", local_param_name, local_param_name);
                }
                else
                {
                    w.write("%.count, %.start", local_param_name, local_param_name);
                }
            }
            else if (param.in())
            {
                write_convert_to_abi_arg(w, param);
            }
            else
            {
                auto category = get_category(param.type);
                bool is_blittable = is_type_blittable(param.signature.Type());
                if (needs_wrapper(category))
                {
                    w.write("&%Abi", local_param_name);
                }
                else if (category == param_category::struct_type)
                {
                    if (is_blittable)
                    {
                        w.write("&%", local_param_name);
                    }
                    else
                    {
                        w.write("&%.val", local_param_name);
                    }
                }
                else if (is_blittable)
                {
                    w.write("&%", param_name);
                }
                else
                {
                    w.write("&%", local_param_name);
                }
            }
        }

        if (function.return_type)
        {
            s();
            auto param_name = function.return_type.value().name;
            if (function.return_type.value().is_array())
            {
                w.write("&%.count, &%.start", param_name, param_name);
            }
            else if (needs_wrapper(get_category(function.return_type->type)))
            {
                w.write("&%Abi", param_name);
            }
            else
            {
                w.write("&%", param_name);
            }
        }
    }

    static std::optional<writer::indent_guard> write_init_return_val_abi(writer& w, function_return_type const& signature)
    {
        auto category = get_category(signature.type);
        if (signature.is_array())
        {
            w.write("var %: WinRTArrayAbi<%> = (0, nil)\n",
                signature.name,
                bind<write_type>(*signature.type, write_type_params::c_abi));
            return std::optional<writer::indent_guard>();
        }
        else if (needs_wrapper(category))
        {
            w.write("let (%) = try ComPtrs.initialize { %Abi in\n", signature.name, signature.name);
            return writer::indent_guard(w, 1);
        }
        else
        {
            w.write("var %: ", signature.name);
            auto guard{ w.push_mangled_names_if_needed(category) };
            write_type(w, *signature.type, write_type_params::c_abi);
            write_default_init_assignment(w, *signature.type, projection_layer::c_abi);
            w.write("\n");
            return std::optional<writer::indent_guard>();
        }
    }

    static void write_consume_return_statement(writer& w, function_def const& signature);
    static void write_return_type_declaration(writer& w, function_def function, write_type_params const& type_params)
    {
        if (!function.return_type)
        {
            return;
        }

        w.write(" -> ");
        if (function.return_type->is_array() && type_params.layer == projection_layer::swift)
        {
            w.write("[%]", bind<write_type>(*function.return_type->type, write_type_params::swift));
        }
        else
        {
            write_type(w, *function.return_type->type, type_params);
        }
    }

    static std::vector<function_param> get_projected_params(attributed_type const& factory, function_def const& func);
    static write_scope_guard<writer> write_local_param_wrappers(writer& w, std::vector<function_param> const& params);
    static void do_write_interface_abi(writer& w, typedef_base const& type, std::vector<function_def> const& methods)
    {
        auto factory_info = try_get_factory_info(w, type);
        auto classType = try_get_exclusive_to(w, type);

        if (classType && !can_write(w, classType))
        {
            return;
        }

        const bool isInitializer = factory_info.has_value() && (factory_info->activatable || factory_info->composable);
        const bool composableFactory = factory_info.has_value() && factory_info->composable;

        auto name = w.write_temp("%", type);
        auto baseClass = (is_delegate(type) || !type.type().Flags().WindowsRuntime()) ? "IUnknown" : "IInspectable";
        w.write("public class %: %.% {\n",
            bind_type_abi(type),
            w.support,
            baseClass);

        auto class_indent_guard = w.push_indent();

        auto iid_format = "override public class var IID: %.IID { IID_% }\n\n";
        w.write(iid_format, w.support, bind_type_mangled(type));

        for (const auto& function : methods)
        {
            if (!can_write(w, function, true)) continue;
            try
            {
                auto func_name = get_abi_name(function);
                auto full_names = w.push_full_type_names(true);

                auto returnStatement = isInitializer ?
                    w.write_temp(" -> %", bind_type_abi(classType->default_interface)) :
                    w.write_temp("%", bind<write_return_type_declaration>(function, write_type_params::swift));

                std::vector<function_param> params = composableFactory ? get_projected_params(factory_info.value(), function) : function.params;
                std::string written_params = w.write_temp("%", bind<write_function_params2>(params, write_type_params::swift));
                if (composableFactory)
                {
                    if (params.size() > 0) written_params.append(", ");
                    written_params.append(w.write_temp("_ baseInterface: UnsealedWinRTClassWrapper<%.Composable>?, _ innerInterface: inout %.IInspectable?", bind_bridge_name(*classType), w.support));
                }

                w.write("% func %(%) throws% {\n",
                    is_exclusive(type) ? "public" : "open",
                    func_name,
                    written_params,
                    returnStatement);
                {
                    auto function_indent_guard = w.push_indent();
                    std::vector<writer::indent_guard> initialize_result_indent;
                    if (function.return_type)
                    {
                        if (auto result = write_init_return_val_abi(w, function.return_type.value()))
                        {
                            initialize_result_indent.push_back(std::move(result.value()));
                        }
                    }

                    {
                        auto guard = write_local_param_wrappers(w, params);

                        if (composableFactory)
                        {
                            w.write("let _baseInterface = baseInterface?.toIInspectableABI { $0 }\n");
                            w.write("let (_innerInterface) = try ComPtrs.initialize { _innerInterfaceAbi in\n");
                            guard.push_indent();
                            guard.push("}\n");
                            guard.push("innerInterface = %.IInspectable(_innerInterface!)\n", w.support);
                        }
                        w.write(R"(_ = try perform(as: %.self) { pThis in
    try CHECKED(pThis.pointee.lpVtbl.pointee.%(%))
}
)",
bind_type_mangled(type),
func_name,
bind<write_abi_args>(function));
                    }

                    for (auto&& guard : initialize_result_indent)
                    {
                        guard.end();
                        w.write("}\n");
                    }


                    if (function.return_type && !isInitializer)
                    {
                        w.write("%\n", bind<write_consume_return_statement>(function));
                    }
                    else if (isInitializer)
                    {
                        w.write("return %(%!)\n", bind_type_abi(classType->default_interface), function.return_type->name);
                    }
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
        type.write_swift_declaration(w);

        if (!is_winrt_ireference(type))
        {
            auto generic_params = w.push_generic_params(type);
            do_write_interface_abi(w, *type.generic_type(), type.functions);
        }
    }

    static void write_ireference_init_extension(writer& w, generic_inst const& type)
    {
        if (!is_winrt_ireference(type)) return;
        auto generic_param = type.generic_params()[0];
        w.add_depends(*generic_param);

        auto impl_names = w.push_impl_names(true);

        w.write(R"(internal enum %: ReferenceBridge {
    typealias CABI = %
    typealias SwiftProjection = %
    static var IID: %.IID { IID_% }

    static func from(abi: ComPtr<CABI>?) -> SwiftProjection? {
        guard let val = abi else { return nil }
        var result: %%
        try! CHECKED(val.get().pointee.lpVtbl.pointee.get_Value(val.get(), &result))
        return %
    }

    static func makeAbi() -> CABI {
        let vtblPtr = withUnsafeMutablePointer(to: &%VTable) { $0 }
        return .init(lpVtbl: vtblPtr)
    }
}
)", bind_bridge_name(type),
    type.mangled_name(),
    get_full_swift_type_name(w, generic_param),
    w.support,
    type.mangled_name(),
    bind<write_type>(*generic_param, write_type_params::c_abi),
    bind<write_default_init_assignment>(*generic_param, projection_layer::c_abi),
    bind<write_consume_type>(generic_param, "result", true),
    type.mangled_name());
    }

    static void write_class_func_body(writer& w, function_def const& function, interface_info const& iface, bool is_noexcept);
    static void write_comma_param_names(writer& w, std::vector<function_param> const& params);
    template <typename T>
    static void write_delegate_extension(writer& w, T const& inst, function_def const& invoke_method)
    {
        auto delegate_abi_name = w.write_temp("%", bind_type_mangled(inst));

        constexpr bool is_generic = std::is_same_v<T, generic_inst>;

        w.write(R"(% extension WinRTDelegateBridge where CABI == % {
    static func makeAbi() -> CABI {
        let vtblPtr = withUnsafeMutablePointer(to: &%.%VTable) { $0 }
        return .init(lpVtbl:vtblPtr)
    }
}

)", is_generic ? "internal" : "public",
    delegate_abi_name,
    is_generic ? w.swift_module : abi_namespace(w.type_namespace),
    is_generic ? delegate_abi_name : w.write_temp("%", inst));
    }

    static void write_generic_extension(writer& w, generic_inst const& inst)
    {
        if (is_winrt_ireference(inst))
        {
            write_ireference_init_extension(w, inst);
        }
        else if (is_delegate(inst))
        {
            auto guard{ w.push_generic_params(inst) };
            write_delegate_extension(w, inst, inst.functions[0]);
        }
    }

    static void write_interface_abi(writer& w, interface_type const& type)
    {
        // Don't write generic interfaces defintions at the ABI layer, we need an actual
        // instantiation of the type in order to create vtables and actual implementations
        if (!can_write(w, type) || type.is_generic()) return;

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
        auto impl_name = w.write_temp("%", bind_bridge_fullname(type));
        auto wrapper_name = w.write_temp("%", bind_wrapper_name(type));
        auto format = R"(
typealias % = InterfaceWrapperBase<%>
)";
        w.write(format, wrapper_name, impl_name);
    }

    static void write_generic_delegate_wrapper(writer& w, generic_inst const& generic)
    {
        write_delegate_wrapper(w, generic);
    }

    static void write_delegate_abi(writer& w, delegate_type const& type)
    {
        if (type.is_generic()) return;
        w.write("// MARK - %\n", type);
        w.write("extension % {\n", abi_namespace(w.type_namespace));
        {
            auto guard(w.push_indent());
            do_write_interface_abi(w, type, type.functions);

            write_delegate_wrapper(w, type);
            write_vtable(w, type);
        }
        w.write("}\n");
        write_delegate_extension(w, type, type.functions[0]);
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
                for (const auto& field : type.members)
                {
                    // WIN-64 - swiftwinrt: support boxing/unboxing
                    // WIN-65 - swiftwinrt: support generic types
                    if (can_write(w, field.type))
                    {
                        std::string from = std::string("swift.").append(get_swift_name(field));

                        if (is_winrt_ireference(field.type))
                        {
                            w.write("let %Wrapper = %(%)\n", get_abi_name(field), bind_wrapper_fullname(field.type), from);
                            w.write("%Wrapper?.copyTo(&val.%)\n", get_abi_name(field), get_abi_name(field));
                        }
                        else
                        {
                            w.write("val.% = %\n",
                                get_abi_name(field),
                                bind<write_consume_type>(field.type, from, false)
                            );
                        }

                    }

                }
            }
            w.write("}\n\n");

            w.write("public func detach() -> % {\n", bind_type_mangled(type));
            {
                auto indent = w.push_indent();

                w.write("let result = val\n");
                for (const auto& member : type.members)
                {
                    auto field = member.field;
                    if (get_category(member.type) == param_category::string_type ||
                        is_winrt_ireference(member.type))
                    {
                        w.write("val.% = nil\n", get_abi_name(member));
                    }
                }
                w.write("return result\n");
            }
            w.write("}\n\n");

            w.write("deinit {\n");
            {
                auto indent = w.push_indent();
                for (const auto& member : type.members)
                {
                    if (get_category(member.type) == param_category::string_type)
                    {
                        w.write("WindowsDeleteString(val.%)\n", get_abi_name(member));
                    }
                    else if (is_winrt_ireference(member.type))
                    {
                        w.write("_ = val.%?.pointee.lpVtbl.pointee.Release(val.%)\n", get_abi_name(member), get_abi_name(member));
                    }
                }
            }
            w.write("}\n");
        }
        w.write("}\n");
    }

    static void write_convert_vtable_params(writer& w, function_def const& signature)
    {
        int param_number = 1;
        auto full_type_names = w.push_full_type_names(true);

        for (auto& param : signature.params)
        {
            std::string param_name = "$" + std::to_string(param_number);
            if (param.is_array())
            {
                auto array_param_name = "$" + std::to_string(param_number + 1);
                auto count_param_name = param_name;
                if (param.in())
                {
                    w.write("let %: [%] = %\n",
                        get_swift_name(param),
                        bind<write_type>(*param.type, write_type_params::swift),
                        bind<write_convert_array_from_abi>(*param.type, w.write_temp("(count: %, start: %)", count_param_name, array_param_name)));
                }
                else if (param.signature.ByRef())
                {
                    w.write("var % = [%]()\n",
                        get_swift_name(param),
                        bind<write_type>(*param.type, write_type_params::swift));
                }
                else
                {
                    w.write("var %: [%] = %\n",
                        get_swift_name(param),
                        bind<write_type>(*param.type, write_type_params::swift),
                        bind<write_convert_array_from_abi>(*param.type, w.write_temp("(count: %, start: %)", count_param_name, array_param_name)));
                }
                ++param_number;
            }
            else if (param.in())
            {
                assert(!param.out());

                if (is_delegate(param.type))
                {
                    w.write("guard let % = % else { return E_INVALIDARG }\n",
                        get_swift_name(param),
                        bind<write_consume_type>(param.type, param_name, false));
                }
                else
                {
                    w.write("let %: % = %\n",
                        get_swift_name(param),
                        bind<write_type>(*param.type, write_type_params::swift),
                        bind<write_consume_type>(param.type, param_name, false));
                }
            }
            else
            {
                assert(!param.in());
                assert(param.out());
                w.write("var %: %%\n",
                    get_swift_name(param),
                    bind<write_type>(*param.type, write_type_params::swift),
                    bind<write_default_init_assignment>(*param.type, projection_layer::swift));
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

        auto return_type = signature.return_type.value();
        auto return_param_name = put_in_backticks_if_needed(std::string(return_type.name));
        if (return_type.is_array())
        {
            w.write("defer { CoTaskMemFree(%.start) }\n", return_param_name);
            w.write("return %\n", bind<write_convert_array_from_abi>(*return_type.type, return_param_name));
        }
        else
        {
            w.write("return %", bind<write_consume_type>(return_type.type, return_param_name, true));
        }

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

    static void write_implementable_interface(writer& w, interface_type const& type)
    {
        write_vtable(w, type);

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
bind_bridge_fullname(type));
    }

    static void write_class_impl_func(writer& w, function_def const& method, interface_info const& iface, typedef_base const& type_definition);
    static void write_class_impl_property(writer& w, property_def const& prop, interface_info const& iface, typedef_base const& type_definition);
    static void write_class_impl_event(writer& w, event_def const& event, interface_info const& iface, typedef_base const& type_definition);
    static void write_property_value_impl(writer& w)
    {
        auto winrtInterfaceConformance = w.write_temp(R"(
    public func queryInterface(_ iid: %.IID) -> IUnknownRef? {
        guard iid == __ABI_Windows_Foundation.IPropertyValueWrapper.IID else { return nil }
        guard let thisAsIPropValue = __ABI_Windows_Foundation.IPropertyValueWrapper(self) else { fatalError("creating non-nil wrapper shouldn't fail") }
        return thisAsIPropValue.queryInterface(iid)
    }
)", w.support);

        w.write(R"(public class IPropertyValueImpl : IPropertyValue, IReference {
    public typealias T = Any
    var _value: Any
    var propertyType : PropertyType

    fileprivate init(_ abi: ComPtr<__x_ABI_CWindows_CFoundation_CIPropertyValue>) { fatalError("not implemented") }
    public init(value: Any) {
        _value = value
        propertyType = switch value {
            case is UInt8: .uint8
            case is Int16: .int16
            case is UInt16: .uint16
            case is Int32: .int32
            case is UInt32: .uint32
            case is Int64: .int64
            case is UInt64: .uint64
            case is Float: .single
            case is Double: .double
            case is Character: .char16
            case is Bool: .boolean
            case is String: .string
            case is DateTime: .dateTime
            case is TimeSpan: .timeSpan
            case is Foundation.UUID: .guid
            case is Point: .point
            case is Size: .size
            case is Rect: .rect
            case is IWinRTObject: .inspectable
            case is IInspectable: .inspectable
            case is [UInt8]: .uint8Array
            case is [Int16]: .int16Array
            case is [UInt16]: .uint16Array
            case is [Int32]: .int32Array
            case is [UInt32]: .uint32Array
            case is [Int64]: .int64Array
            case is [UInt64]: .uint64Array
            case is [Float]: .singleArray
            case is [Double]: .doubleArray
            case is [Character]: .char16Array
            case is [Bool]: .booleanArray
            case is [String]: .stringArray
            case is [DateTime]: .dateTimeArray
            case is [TimeSpan]: .timeSpanArray
            case is [Foundation.UUID]: .guidArray
            case is [Point]: .pointArray
            case is [Size]: .sizeArray
            case is [Rect]: .rectArray
            case is [Any?]: .inspectableArray
            default: .otherType
            }
    }

    public var type: PropertyType { propertyType }
    public var isNumericScalar: Bool {
        switch propertyType {
            case .int16, .int32, .int64, .uint8, .uint16, .uint32, .uint64, .single, .double: return true
            default: return false
        }
    }

    public var value: Any { _value }

    public func getUInt8() -> UInt8 { _value as! UInt8 }
    public func getInt16() -> Int16 { _value as! Int16 }
    public func getUInt16() -> UInt16 { _value as! UInt16 }
    public func getInt32() -> Int32 { _value as! Int32 }
    public func getUInt32() -> UInt32 { _value as! UInt32 }
    public func getInt64() -> Int64 { _value as! Int64 }
    public func getUInt64() -> UInt64 { _value as! UInt64 }
    public func getSingle() -> Float { _value as! Float }
    public func getDouble() -> Double { _value as! Double }
    public func getChar16() -> Character { _value as! Character }
    public func getBoolean() -> Bool { _value as! Bool }
    public func getString() -> String { _value as! String }
    public func getGuid() -> Foundation.UUID { _value as! Foundation.UUID }
    public func getDateTime() -> DateTime { _value as! DateTime }
    public func getTimeSpan() -> TimeSpan { _value as! TimeSpan }
    public func getPoint() -> Point { _value as! Point }
    public func getSize() -> Size { _value as! Size }
    public func getRect() -> Rect { _value as! Rect }
    public func getUInt8Array(_ value: inout [UInt8]) { value = _value as! [UInt8] }
    public func getInt16Array(_ value: inout [Int16]) { value = _value as! [Int16] }
    public func getUInt16Array(_ value: inout [UInt16]) { value = _value as! [UInt16] }
    public func getInt32Array(_ value: inout [Int32]) { value = _value as! [Int32] }
    public func getUInt32Array(_ value: inout [UInt32])  { value = _value as! [UInt32] }
    public func getInt64Array(_ value: inout [Int64]) { value = _value as! [Int64] }
    public func getUInt64Array(_ value: inout [UInt64]) { value = _value as! [UInt64] }
    public func getSingleArray(_ value: inout [Float]) { value = _value as! [Float] }
    public func getDoubleArray(_ value: inout [Double]) { value = _value as! [Double] }
    public func getChar16Array(_ value: inout [Character]) { value = _value as! [Character] }
    public func getBooleanArray(_ value: inout [Bool]) { value = _value as! [Bool] }
    public func getStringArray(_ value: inout [String]) { value = _value as! [String] }
    public func getGuidArray(_ value: inout [Foundation.UUID]) { value = _value as! [Foundation.UUID] }
    public func getDateTimeArray(_ value: inout [DateTime]) { value = _value as! [DateTime] }
    public func getTimeSpanArray(_ value: inout [TimeSpan]) { value = _value as! [TimeSpan] }
    public func getPointArray(_ value: inout [Point]) { value = _value as! [Point] }
    public func getSizeArray(_ value: inout [Size]) { value = _value as! [Size] }
    public func getRectArray(_ value: inout [Rect]) { value = _value as! [Rect] }
    public func getInspectableArray(_ value: inout [Any?]) { value = _value as! [Any?] }
    %
}

)", winrtInterfaceConformance);

    }

    // Due to https://linear.app/the-browser-company/issue/WIN-148/investigate-possible-compiler-bug-crash-when-generating-collection
    // we have to generate the protocol conformance for the Collection protocol (see "// MARK: Collection" below). We shouldn't have to
    // do this because we define an extension on the protocol which does this.
    static void write_collection_protocol_conformance(writer& w, interface_info const& info)
    {
        auto modifier = w.impl_names ? "" : "public ";
        auto typeName = info.type->swift_type_name();
        if (typeName.starts_with("IVector") || typeName.starts_with("IObservableVector"))
        {
            w.write(R"(// MARK: Collection
%typealias Element = T
%var startIndex: Int { 0 }
%var endIndex: Int { Int(size) }
%func index(after i: Int) -> Int {
    i+1
}

%func index(of: Element) -> Int? {
    var index: UInt32 = 0
    let result = indexOf(of, &index)
    guard result else { return nil }
    return Int(index)
}
%var count: Int { Int(size) }
)", modifier, modifier, modifier, modifier, modifier, modifier);
            if (typeName.starts_with("IVectorView"))
            {
                w.write(R"(
%subscript(position: Int) -> Element {
    get {
        getAt(UInt32(position))
    }
}
)", modifier);
            }
            else
            {
                w.write(R"(

%subscript(position: Int) -> Element {
    get {
        getAt(UInt32(position))
    }
    set(newValue) {
        setAt(UInt32(position), newValue)
    }
}

%func removeLast() {
    removeAtEnd()
}

)", modifier, modifier);
            }
        }

        w.write("// MARK: WinRT\n");
    }

    enum class member_type
    {
        property_or_method,
        event
    };

    static std::string modifier_for(typedef_base const& type_definition, interface_info const& iface, member_type member_type = member_type::property_or_method);
    static void write_bufferbyteaccess(writer& w, interface_info const& info, system_type const& type, typedef_base const& type_definition)
    {
        auto bufferType = type.swift_type_name() == "IBufferByteAccess" ? "UnsafeMutablePointer" : "UnsafeMutableBufferPointer";
        w.write(R"(%var buffer: %<UInt8>? {
    get throws {
        let bufferByteAccess: %.__ABI_.% = try %.QueryInterface()
        return try bufferByteAccess.Buffer()
    }
}
)", modifier_for(type_definition, info), bufferType,  w.support, type.swift_type_name(), get_swift_name(info));
    }

    static void write_interface_impl_members(writer& w, interface_info const& info, typedef_base const& type_definition)
    {
        w.add_depends(*info.type);
        bool is_class = swiftwinrt::is_class(&type_definition);

        if (!info.is_default || (!is_class && info.base))
        {
            auto swiftAbi = w.write_temp("%.%", abi_namespace(info.type->swift_logical_namespace()), info.type->swift_type_name());
            if (is_generic_inst(info.type))
            {
                auto guard{ w.push_generic_params(info) };
                swiftAbi = w.write_temp("%", bind_type_abi(info.type));
            }
            w.write("private lazy var %: %! = getInterfaceForCaching()\n",
                get_swift_name(info),
                swiftAbi);
        }

        if (auto iface = dynamic_cast<const interface_type*>(info.type))
        {
            for (const auto& method : iface->functions)
            {
                write_class_impl_func(w, method, info, type_definition);
            }

            for (const auto& prop : iface->properties)
            {
                write_class_impl_property(w, prop, info, type_definition);
            }

            for (const auto& event : iface->events)
            {
                write_class_impl_event(w, event, info, type_definition);
            }
        }
        else if (auto gti = dynamic_cast<const generic_inst*>(info.type))
        {
            for (const auto& method : gti->functions)
            {
                write_class_impl_func(w, method, info, type_definition);
            }

            for (const auto& prop : gti->properties)
            {
                write_class_impl_property(w, prop, info, type_definition);
            }

            for (const auto& event : gti->events)
            {
                write_class_impl_event(w, event, info, type_definition);
            }
        }
        else if (auto systemType = dynamic_cast<const system_type*>(info.type))
        {
            if (systemType->swift_type_name() == "IBufferByteAccess" || systemType->swift_type_name() == "IMemoryBufferByteAccess")
            {
                write_bufferbyteaccess(w, info, *systemType, type_definition);
            }
        }
        else
        {
            assert(!"Unexpected interface type.");
        }
    }

    static bool skip_write_from_abi(writer& w, metadata_type const& type)
    {
        if (auto interfaceType = dynamic_cast<const interface_type*>(&type))
        {
            return (interfaceType->is_generic() || is_exclusive(*interfaceType) || !can_write(w, interfaceType) || get_full_type_name(interfaceType) == "Windows.Foundation.IPropertyValue");
        }
        else if (auto classType = dynamic_cast<const class_type*>(&type))
        {
            return classType->default_interface == nullptr;
        }
        return true;
    }

    static void write_make_from_abi(writer& w, metadata_type const& type)
    {
        if (skip_write_from_abi(w, type)) return;

        std::string fromAbi;
        std::string swiftType;
        if (is_interface(type))
        {
            fromAbi = w.write_temp("let swiftAbi: %.% = try! abi.QueryInterface()\n", abi_namespace(type),
                type.swift_type_name());
            fromAbi += w.write_temp("        return %.from(abi: RawPointer(swiftAbi))!", bind_bridge_fullname(type));
            swiftType = w.write_temp("%", bind<write_swift_interface_existential_identifier>(type));
        }
        else if (is_class(&type))
        {
            fromAbi = w.write_temp("return %(fromAbi: abi)", type.swift_type_name());
            swiftType = w.write_temp("%", bind<write_swift_type_identifier>(type));
        }
        else
        {
            throw std::exception("Invalid type for MakeFromAbi");
        }

        w.write(R"(^@_spi(WinRTInternal)
public class %Maker: MakeFromAbi {
    public typealias SwiftType = %
    public static func from(abi: %.IInspectable) -> SwiftType {
        %
    }
}
)", type.swift_type_name(), swiftType, w.support, fromAbi);
    }

    static void write_interface_bridge(writer& w, metadata_type const& type)
    {
        std::string swiftABI;
        std::string vtable;
        bool is_generic = is_generic_inst(type);
        if (is_generic)
        {
            swiftABI = w.write_temp("%", bind_type_abi(type));
            vtable = w.write_temp("%", bind_type_mangled(type));
        }
        else
        {
            swiftABI = w.write_temp("%.%", abi_namespace(type), type);
            vtable = swiftABI;
        }

        auto modifier = is_generic ? "internal" : "public";
        w.write(R"(% enum % : AbiInterfaceBridge {
    % typealias CABI = %
    % typealias SwiftABI = %
    % typealias SwiftProjection = %
    % static func from(abi: ComPtr<CABI>?) -> SwiftProjection? {
        guard let abi = abi else { return nil }
        return %(abi)
    }

    % static func makeAbi() -> CABI {
        let vtblPtr = withUnsafeMutablePointer(to: &%VTable) { $0 }
        return .init(lpVtbl: vtblPtr)
    }
}

)",
modifier, bind_bridge_name(type),
modifier, bind_type_mangled(type),
modifier, swiftABI,
modifier, bind<write_swift_interface_existential_identifier>(type), // Do not include outer Optional<>
modifier,
bind_impl_name(type),
modifier,
vtable);
    }

    static void write_interface_impl(writer& w, interface_type const& type)
    {
        if (is_exclusive(type) || !can_write(w, type) || type.is_generic()) return;

        write_interface_bridge(w, type);
        if (get_full_type_name(type) == "Windows.Foundation.IPropertyValue")
        {
            write_property_value_impl(w);
            return;
        }

        w.write(R"(fileprivate class %: %, WinRTAbiImpl {
    fileprivate typealias Bridge = %
    fileprivate let _default: Bridge.SwiftABI
    fileprivate var thisPtr: %.IInspectable { _default }
    fileprivate init(_ fromAbi: ComPtr<Bridge.CABI>) {
        _default = Bridge.SwiftABI(fromAbi)
    }

)",
            bind_impl_name(type),
            type,
            bind_bridge_name(type),
            w.support);
        auto class_indent_guard = w.push_indent();

        write_generic_typealiases(w, type);

        interface_info type_info{ &type };
        type_info.is_default = true; // mark as default so we use the name "_default"
        for (const auto& method : type.functions)
        {
            write_class_impl_func(w, method, type_info, type);
        }
        for (const auto& prop : type.properties)
        {
            write_class_impl_property(w, prop, type_info, type);
        }

        for (const auto& event : type.events)
        {
            write_class_impl_event(w, event, type_info, type);
        }

        for (const auto& [interface_name, info] : type.required_interfaces)
        {
            if (!can_write(w, info.type)) { continue; }

            write_interface_impl_members(w, info, /* type_definition: */ type);
        }

        class_indent_guard.end();
        w.write("}\n\n");
    }

    static void write_comma_param_types(writer& w, std::vector<function_param> const& params);
    static void write_delegate_return_type(writer& w, function_def const& sig);
    static void write_query_interface_case(writer& w, interface_info const& iface);
    static void write_eventsource_invoke_extension(writer& w, metadata_type const* event_type)
    {
        writer::generic_param_guard guard{};
        function_def delegate_method{};
        auto access_level = "public";
        if (auto delegateType = dynamic_cast<const delegate_type*>(event_type))
        {
            delegate_method = delegateType->functions[0];
        }
        else if (auto genericInst = dynamic_cast<const generic_inst*>(event_type))
        {
            delegate_method = genericInst->functions[0];
            guard = w.push_generic_params(*genericInst);
        }
        std::string invoke_implementation;
        if (delegate_method.return_type)
        {
            invoke_implementation = w.write_temp(R"(var result:%%
        for handler in getInvocationList() {
            result = try handler(%)
        }
        return result)",
                bind<write_type>(*delegate_method.return_type->type, write_type_params::swift),
                bind<write_default_init_assignment>(*delegate_method.return_type->type, projection_layer::swift),
                bind<write_comma_param_names>(delegate_method.params));
        }
        else
        {
            invoke_implementation = w.write_temp(R"(for handler in getInvocationList() {
            try handler(%)
        })", bind<write_comma_param_names>(delegate_method.params));
        }

        assert(delegate_method.def);
        w.write(R"(% extension EventSource where Handler == % {
    %func invoke(%) throws% {
        %
    }
}

)", access_level, event_type,
    delegate_method.return_type ? "@discardableResult " : "",
    bind<write_function_params>(delegate_method, write_type_params::swift_allow_implicit_unwrap),
    bind<write_return_type_declaration>(delegate_method, write_type_params::swift_allow_implicit_unwrap),
    invoke_implementation);
    }
    static void write_interface_proto(writer& w, interface_type const& type)
    {
        if (is_exclusive(type) || !can_write(w, type))
        {
            return;
        }

        auto typeName = swiftwinrt::remove_backtick(type.swift_type_name());
        auto interfaces = type.required_interfaces;
        separator s{ w };
        auto implements = w.write_temp("%", bind_each([&](writer& w, std::pair<std::string, interface_info> const& iface) {
            s();
            write_swift_type_identifier(w, *iface.second.type);
        }, interfaces));

        // check the type name is a collection so we don't get any potential unknown or unwanted
        // typenames like IMapChangedEventArgs
        if (is_winrt_generic_collection(type))
        {
            if (typeName.starts_with("IVector"))
            {
                implements.append(", Collection where Element == T, Index == Int");
            }
            else if (typeName.starts_with("IMap"))
            {
                implements.append(" where T == AnyIKeyValuePair<K,V>?");
            }
        }

        std::vector<std::string> eventSourceInvokeLines;
        write_documentation_comment(w, type);
        w.write("public protocol % : %% {\n", type, implements,
            implements.empty() ? "WinRTInterface" : "");
        {
            auto body_indent = w.push_indent();
            if (type.is_generic())
            {
                for (const auto& param : type.type().GenericParam())
                {
                    w.write("associatedtype %\n", param.Name());
                }
            }
            for (const auto& method : type.functions)
            {
                if (!can_write(w, method)) continue;

                auto full_type_name = w.push_full_type_names(true);
                auto maybe_throws = is_noexcept(type, method) ? "" : " throws";
                auto type_params = swift_write_type_params_for(type);

                write_documentation_comment(w, type, method.def.Name());
                w.write("func %(%)%%\n",
                    get_swift_name(method),
                    bind<write_function_params>(method, type_params),
                    maybe_throws,
                    bind<write_return_type_declaration>(method, type_params));
            }

            for (auto& prop : type.properties)
            {
                if (!can_write(w, prop)) continue;
                auto full_type_name = w.push_full_type_names(true);
                auto format = prop.is_array() ? "[%]" : "%";
                auto propertyType = w.write_temp(format, bind<write_type>(*prop.getter->return_type->type, swift_write_type_params_for(type, prop.is_array())));
                write_documentation_comment(w, type, prop.def.Name());
                w.write("var %: % { get% }\n",
                    get_swift_name(prop),
                    propertyType,
                    prop.setter ? " set" : "");
            }

            for (const auto& event : type.events)
            {
                write_documentation_comment(w, type, event.def.Name());
                w.write("var %: Event<%> { get }\n",
                    get_swift_name(event.def),
                    event.type);
                // only write the eventsource extension for interfaces which could be implemented by a swift object
                // not only does this result in less code generated, it also helps alleviate the issue where different
                // interfaces define an event with the same type. For that scenario, we cache the event type on the
                // writer
                if (!type.is_generic())
                {
                    if (auto delegate = dynamic_cast<const delegate_type*>(event.type))
                    {
                        if (w.implementableEventTypes.find(delegate) == w.implementableEventTypes.end())
                        {
                            w.implementableEventTypes.insert(delegate);
                            eventSourceInvokeLines.push_back(w.write_temp("%", bind<write_eventsource_invoke_extension>(delegate)));
                        }
                    }
                }
            }
        }
        w.write("}\n\n");

        for (const auto& line : eventSourceInvokeLines)
        {
            w.write(line);
        }
        if (!type.is_generic() && type.swift_full_name() != "Windows.Foundation.IPropertyValue")
        {
            // write default queryInterface implementation for this interface. don't do
            // it for IPropertyValue since this has a custom wrapper implementation. We write
            // this implementation so when an app code derives from a single WinRT interface
            // they don't need to write the queryInterface implementation themselves. We
            // know for a fact that we're only here in the scenario that a single WinRT
            // interface is implemented because if they implement multiple interfaces, they
            // have to write the queryInterface implementation themselves.
            w.write("extension % {\n", typeName);
            w.write("    public func queryInterface(_ iid: %.IID) -> IUnknownRef? {\n", w.support);
            w.write("        switch iid {\n");
            auto indent{ w.push_indent({3}) };

            w.write("%", bind<write_query_interface_case>(interface_info{ &type }));

            for (auto [name, info] : type.required_interfaces) {
                if (can_write(w, info.type))
                {
                    w.write("%", bind<write_query_interface_case>(info));
                }
            }

            w.write("default: return nil\n");
            indent.end();
            w.write("        }\n");
            w.write("    }\n");
            w.write("}\n");

            if (type.swift_full_name() == "Windows.Storage.Streams.IBuffer")
            {
                w.write(R"(extension IBuffer {
    public var data: Data {
        guard let buffer = try? buffer else { return Data() }
        return Data(bytesNoCopy: buffer, count: Int(length), deallocator: .none)
    }
}
)");
            }
            else if (type.swift_full_name() == "Windows.Foundation.IMemoryBufferReference")
            {
                w.write(R"(extension IMemoryBufferReference {
    public var data: Data {
        guard let buffer = try? buffer, let ptr = buffer.baseAddress else { return Data() }
        return Data(bytesNoCopy: ptr, count: buffer.count, deallocator: .none)
    }
}
)");
            }
        }
        // Declare a short form for the existential version of the type, e.g. AnyClosable for "any IClosable"
        w.write("public typealias Any% = any %\n\n", type, type);

        if (is_winrt_async_result_type(type))
        {
            std::string return_clause;
            const metadata_type* result_type = nullptr;
            if (type.generic_params.size() > 0) {
                return_clause = w.write_temp(" -> %", type.generic_params[0]);
            }
            w.write(R"(public extension % {
    func get() async throws% {
        if status == .started {
            let event = WaitableEvent()
            completed = { _, _ in
                Task { await event.signal() }
            }
            await event.wait()
        }
        return try getResults()
    }
}

)", bind<write_swift_type_identifier>(type), return_clause);
        }
    }

    static void write_delegate_return_type(writer& w, function_def const& sig)
    {
        if (sig.return_type)
        {
            write_type(w, *sig.return_type->type, write_type_params::swift);
        }
        else
        {
            w.write("()");
        }
    }

    static void write_comma_param_names(writer& w, std::vector<function_param> const& params)
    {
        separator s{ w };
        for (auto& param : params)
        {
            s();
            w.write(get_swift_name(param));
        }
    }

    static void write_comma_param_types(writer& w, std::vector<function_param> const& params)
    {
        separator s{ w };
        for (auto& param : params)
        {
            s();
            if (param.is_array())
            {
                w.write("[%]", bind<write_type>(*param.type, write_type_params::swift));
            }
            else
            {
                write_type(w, *param.type, write_type_params::swift);
            }
        }
    }

    static void write_delegate(writer& w, delegate_type const& type)
    {
        // Delegates require tuples because of the way that the bridges are implemented.
        // The bridge classes have a typealias for the parameters, and we use those
        // parameters for the delegate signature to create the bridge. The swift compiler
        // complains if the typealias isn't placed in a tuple
        function_def delegate_method = type.functions[0];
        w.write("public typealias % = (%) throws -> %\n",
            type,
            bind<write_comma_param_types>(delegate_method.params),
            bind<write_delegate_return_type>(delegate_method));
    }

    template <typename T>
    static void do_write_delegate_implementation(writer& w, T const& type, function_def const& invoke_method)
    {
        auto format = R"(% class % : WinRTDelegateBridge {
    % typealias Handler = %
    % typealias CABI = %
    % typealias SwiftABI = %.%

    % static func from(abi: ComPtr<CABI>?) -> Handler? {
        guard let abi = abi else { return nil }
        let _default = SwiftABI(abi)
        let handler: Handler = { (%) in
%        }
        return handler
    }
}
)";
        auto data = w.write_temp("%", bind<write_comma_param_types>(invoke_method.params));
        auto return_type = w.write_temp("%", bind<write_delegate_return_type>(invoke_method));
        constexpr bool is_generic = std::is_same_v<T, generic_inst>;
        auto access_level = is_generic ? "internal" : "public";
        auto handlerType = w.write_temp("%", bind<write_swift_type_identifier>(type));
        auto abi_guard = w.push_abi_types(is_generic);
        w.write(format,
            access_level,
            bind_bridge_name(type),
            access_level,
            handlerType,
            access_level,
            bind_type_mangled(type),
            access_level,
            is_generic ? w.swift_module : abi_namespace(w.type_namespace),
            bind_type_abi(type),
            access_level,
            bind<write_comma_param_names>(invoke_method.params),
            bind([&](writer& w) {
                interface_info delegate{ &type };
                delegate.is_default = true; // so that the _default name is used
                auto indent_guard{ w.push_indent({3}) };
                write_class_func_body(w, invoke_method, delegate, false);
                }));
    }

    static void write_delegate_implementation(writer& w, delegate_type const& type)
    {
        if (can_write(w, type) && !type.is_generic())
        {
            auto delegate_method = type.functions[0];
            do_write_delegate_implementation(w, type, delegate_method);
        }
    }

    static void write_interface_bridge(writer& w, metadata_type const& type);

    static void write_generic_interface_implementation(writer& w, generic_inst const& type)
    {
        write_interface_bridge(w, type);

        w.write("fileprivate class % : %, AbiInterfaceImpl {\n",
            bind_impl_name(type), type.generic_type_abi_name());

        auto indent_guard = w.push_indent();

        write_generic_typealiases(w, type);


        w.write("typealias Bridge = %\n", bind_bridge_name(type));
        w.write("let _default: Bridge.SwiftABI\n");

        w.write("init(_ fromAbi: ComPtr<Bridge.CABI>) {\n");
        w.write("    _default = Bridge.SwiftABI(fromAbi)\n");
        w.write("}\n\n");

        interface_info info{ &type };
        info.is_default = true; // mark as default so we use the name "_default"

        // Due to https://linear.app/the-browser-company/issue/WIN-148/investigate-possible-compiler-bug-crash-when-generating-collection
        // we have to generate the protocol conformance for the Collection protocol (see "// MARK: Collection" below). We shouldn't have to
        // do this because we define an extension on the protocol which does this.
        write_collection_protocol_conformance(w, info);

        for (const auto& method : type.functions)
        {
            write_class_impl_func(w, method, info, *type.generic_type());
        }
        for (const auto& prop : type.properties)
        {
            write_class_impl_property(w, prop, info, *type.generic_type());
        }
        for (const auto& event : type.events)
        {
            write_class_impl_event(w, event, info, *type.generic_type());
        }

        for (const auto& [interface_name, info] : type.required_interfaces)
        {
            if (!can_write(w, info.type)) { continue; }

            write_interface_impl_members(w, info, *type.generic_type());
        }

        w.write("public func queryInterface(_ iid: %.IID) -> IUnknownRef? { nil }\n", w.support);
        indent_guard.end();
        w.write("}\n\n");
    }

    static void write_generic_implementation(writer& w, generic_inst const& type)
    {
        auto generics_guard = w.push_generic_params(type);
        if (is_delegate(type))
        {
            auto delegate_method = type.functions[0];
            do_write_delegate_implementation(w, type, delegate_method);
        }
        else if (!is_winrt_ireference(type))
        {
            write_generic_interface_implementation(w, type);
        }
    }

    static void write_param_names(writer& w, std::vector<function_param> const& params, std::string_view format)
    {
        separator s{ w };
        for (const auto& param : params)
        {
            s();
            w.write(format, local_swift_param_name(get_swift_name(param)));
        }
    }

    // When converting from Swift <-> C we put some local variables on the stack in order to help facilitate
    // converting between the two worlds. This method will returns a scope guard which will write any necessary
    // code for after the ABI function is called (such as cleaning up references).
    static write_scope_guard<writer> write_local_param_wrappers(writer& w, std::vector<function_param> const& params)
    {
        write_scope_guard guard{ w, w.swift_module };

        std::vector<function_param> com_ptr_initialize;
        for (auto& param : params)
        {
            TypeDef signature_type{};
            auto category = get_category(param.type, &signature_type);
            auto param_name = get_swift_name(param);
            auto local_param_name = local_swift_param_name(param_name);

            if (param.is_array())
            {
                if (param.signature.ByRef())
                {
                    w.write("var %: WinRTArrayAbi<%> = (0, nil)\n",
                        local_param_name,
                        bind<write_type>(*param.type, write_type_params::c_abi));

                    guard.insert_front("% = %\n", param_name, bind<write_convert_array_from_abi>(*param.type, local_param_name));
                    guard.insert_front("defer { CoTaskMemFree(%.start) }\n", local_param_name);
                }
                else
                {
                    // Array is passed by reference, so we need to convert the input to a buffer and then pass that buffer to C, then convert the buffer back to an array
                    if (is_reference_type(param.type))
                    {
                        w.write("try %.toABI(abiBridge: %.self) { % in\n", param_name, bind_bridge_name(*param.type), local_param_name);
                    }
                    else
                    {
                        w.write("try %.toABI { % in\n", param_name, local_param_name);
                    }

                    // Enums and fundamental (integer) types can just be copied directly into the ABI. So we can
                    // avoid an extra copy by simply passing the array buffer to C directly
                    if (!param.in() && category != param_category::enum_type && category != param_category::fundamental_type)
                    {
                        // While perhaps not the most effient to just create a new array from the elements (rather than filling an existing buffer), it is the simplest for now.
                        // These APIs are few and far between and rarely used. If needed, we can optimize later.
                        guard.insert_front("% = %\n", param_name, bind<write_convert_array_from_abi>(*param.type, local_param_name));
                    }

                    guard.push("}\n");
                    guard.push_indent();
                }
            }
            else if (param.in())
            {
                if (category == param_category::string_type)
                {
                    w.write("let % = try! HString(%)\n",
                        local_param_name,
                        param_name);
                }
                else if (category == param_category::struct_type && !is_struct_blittable(signature_type))
                {
                    w.write("let % = %._ABI_%(from: %)\n",
                        local_param_name,
                        abi_namespace(signature_type),
                        signature_type.TypeName(),
                        param_name);
                }
                else if (is_reference_type(param.type) && !is_class(param.type))
                {
                    w.write("let %Wrapper = %(%)\n",
                        param_name,
                        bind_wrapper_fullname(param.type),
                        param_name);
                    w.write("let % = try! %Wrapper?.toABI { $0 }\n",
                        local_param_name,
                        param_name);
                }
            }
            else
            {
                if (category == param_category::string_type)
                {
                    w.write("var %: HSTRING?\n",
                        local_param_name);
                    guard.push("% = .init(from: %)\n",
                        param_name,
                        local_param_name);
                    guard.push("WindowsDeleteString(%)\n", local_param_name);
                }
                else if (category == param_category::struct_type &&
                    is_struct_blittable(signature_type) &&
                    !is_guid(category))
                {
                    w.write("var %: % = .init()\n",
                        local_param_name,
                        bind_type_mangled(param.type));
                    guard.push("% = .from(abi: %)\n",
                        param_name,
                        local_param_name);
                }
                else if (category == param_category::struct_type)
                {
                    w.write("let %: %._ABI_% = .init()\n",
                        local_param_name,
                        abi_namespace(param.type),
                        param.type->swift_type_name());
                    guard.push("% = .from(abi: %.val)\n",
                        param_name,
                        local_param_name);
                }
                else if (category == param_category::boolean_type || category == param_category::character_type)
                {
                    w.write("var %: % = .init()\n",
                        local_param_name,
                        bind_type_abi(param.type));
                    guard.push("% = .init(from: %)\n",
                        param_name,
                        local_param_name);
                }
                else if (needs_wrapper(category))
                {
                    com_ptr_initialize.push_back(param);
                }
            }
        }

        // At initial writing, ComPtrs.initialize only has overloads for 5 parameters. If we have more than 5
        // then the generated code won't compile. Rather than check for the number here, just let generated
        // code not compile so that we can add the overload to ComPtrs.initialize later on. This would also
        // in theory let someone add a new overload to ComPtrs.initialize with a different number of parameters
        // on their own as a way to unblock themselves
        if (!com_ptr_initialize.empty())
        {
            w.write("let (%) = try ComPtrs.initialize { (%) in\n",
                bind<write_param_names>(com_ptr_initialize, "%"),
                bind<write_param_names>(com_ptr_initialize, "%Abi"));
            guard.push("}\n");
            guard.push_indent();

            for (const auto& param : com_ptr_initialize)
            {
                auto param_name = get_swift_name(param);
                auto local_param_name = local_swift_param_name(param_name);
                guard.push("% = %\n", param_name,
                    bind<write_consume_type>(param.type, local_param_name, true));
            }
        }

        return guard;
    }

    static void write_factory_body(writer& w, function_def const& method, interface_info const& factory, class_type const& type, metadata_type const& default_interface)
    {
        std::string_view func_name = get_abi_name(method);

        auto swift_name = get_swift_name(factory);
        auto return_name = method.return_type.value().name;
        auto func_call = w.write_temp("try! Self.%.%(%)",
            swift_name,
            func_name,
            bind<write_implementation_args>(method));
        if (auto base_class = type.base_class)
        {
            w.write("super.init(fromAbi: %)\n", func_call);
        }
        else
        {
            w.write("super.init(%)\n", func_call);
        }

    }

    // Check if the type has a default constructor. This is a parameterless constructor
    // in Swift. Note that we don't check the args like we do in base_has_matching_constructor
    // because composing constructors project as init() when they really have 2 parameters.
    static bool has_default_constructor(const class_type* type)
    {
        if (type == nullptr) return true;

        for (const auto& [_, factory] : type->factories)
        {
            if (factory.composable && factory.defaultComposable)
            {
                return true;
            }
            else if (factory.activatable && factory.type == nullptr)
            {
                return true;
            }
        }
        return false;
    }

    static std::vector<function_param> get_projected_params(attributed_type const& factory, function_def const& func)
    {
        if (factory.composable)
        {
            std::vector<function_param> result;
            result.reserve(func.params.size() - 2);
            // skip the last two which are the inner and base interface parameters
            for (size_t i = 0; i < func.params.size() - 2; ++i)
            {
                result.push_back(func.params[i]);
            }
            return result;
        }
        else
        {
            return func.params;
        }
    }

    static bool derives_from(class_type const& base, class_type const& derived)
    {
        class_type const* checking = &derived;
        while (checking != nullptr)
        {
            if (checking->base_class == &base)
            {
                return true;
            }
            checking = checking->base_class;
        }
        return false;
    }

    static bool base_matches(function_def const& base, function_def const& derived)
    {
        // Simple cases of name/param count/has return not matching
        if (base.def.Name() != derived.def.Name()) return false;
        if (base.return_type.has_value() != derived.return_type.has_value()) return false;
        if (base.params.size() != derived.params.size()) return false;

        // If they both have a return value, we need to check if the return values
        // are derived from each other. If you have two functions like this:
        //   (A.swift) class func make() -> A
        //   (B.swift) class func make() -> B
        // Then these would "match" according to the swift compiler
        if (base.return_type.has_value())
        {
            auto base_return = base.return_type.value().type;
            auto derived_return = derived.return_type.value().type;
            if (base_return != derived_return)
            {
                // Check if the types are derived
                auto base_return_class = dynamic_cast<const class_type*>(base_return);
                auto derived_return_class = dynamic_cast<const class_type*>(derived_return);
                if (base_return_class != nullptr && derived_return_class != nullptr)
                {
                    if (!derives_from(*base_return_class, *derived_return_class))
                    {
                        // Return types aren't a possible match, return false
                        return false;
                    }
                }
            }
        }

        size_t i = 0;
        while (i < base.params.size())
        {
            if (base.params[i].type != derived.params[i].type)
            {
                // param doesn't match
                return false;
            }
            ++i;
        }
        return true;
    }

    static bool base_has_matching_static_function(class_type const& type, attributed_type const& factory, function_def const& func)
    {
        if (type.base_class == nullptr)
        {
            return false;
        }

        for (const auto& [_, baseFactory] : type.base_class->factories)
        {
            // only look at statics
            if (!baseFactory.statics) continue;
            if (auto factoryIface = dynamic_cast<const interface_type*>(baseFactory.type))
            {
                for (const auto& baseMethod : factoryIface->functions)
                {
                    if (base_matches(baseMethod, func))
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    static bool base_has_matching_constructor(class_type const& type, attributed_type const& factory, function_def const& func)
    {
        auto projectedParams = get_projected_params(factory, func);

        if (type.base_class == nullptr)
        {
            return projectedParams.size() == 0;
        }

        for (const auto& [_, baseFactory] : type.base_class->factories)
        {
            // only look at activation or composing constructors
            if (!baseFactory.activatable && !baseFactory.composable) continue;
            if (auto factoryIface = dynamic_cast<const interface_type*>(baseFactory.type))
            {
                for (const auto& baseMethod : factoryIface->functions)
                {
                    auto baseProjectedParams = get_projected_params(baseFactory, baseMethod);
                    if (projectedParams.size() == baseProjectedParams.size())
                    {
                        size_t i = 0;
                        while(i < baseProjectedParams.size())
                        {
                            if (baseProjectedParams[i].type != projectedParams[i].type)
                            {
                                break;
                            }
                            ++i;
                        }
                        if (i == baseProjectedParams.size())
                        {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    static void write_factory_constructors(writer& w, attributed_type const& factory, class_type const& type, metadata_type const& default_interface)
    {
        if (auto factoryIface = dynamic_cast<const interface_type*>(factory.type))
        {
            interface_info factory_info{ factoryIface };
            auto swift_name = get_swift_name(factory_info);
            w.write("private static let %: %.% = try! RoGetActivationFactory(\"%\")\n",
                swift_name, abi_namespace(factoryIface), factory.type, get_full_type_name(type));
            for (const auto& method : factoryIface->functions)
            {
                if (!can_write(w, method)) continue;

                auto baseHasMatchingConstructor = base_has_matching_constructor(type, factory, method);
                w.write("%public init(%) {\n",
                    baseHasMatchingConstructor ? "override " : "",
                    bind<write_function_params>(method, write_type_params::swift_allow_implicit_unwrap));
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

            w.write("private static let _defaultFactory: %.IActivationFactory = try! RoGetActivationFactory(\"%\")\n",
                w.support, get_full_type_name(type));
            w.write("%public init() {\n", has_default_constructor(base_class) ? "override " : "");
            {
                auto indent = w.push_indent();
                auto activateInstance = "try! Self._defaultFactory.ActivateInstance()";
                if (base_class)
                {
                    w.write("super.init(fromAbi: %)\n", activateInstance);
                }
                else
                {
                    w.write("super.init(%)\n", activateInstance);
                }
            }
            w.write("}\n\n");
        }
    }

    // every composable type needs a special composing constructor to help satisfy swift type initializer
    // requirements. this constructor is marked as SPI so that it doesn't show up to normal developers
    // when they are building.
    static void write_internal_composable_constructor(writer& w, class_type const& type)
    {
        if (type.base_class)
        {
            w.write(R"(^@_spi(WinRTInternal)
override public init<Composable: ComposableImpl>(
    composing: Composable.Type,
    _ createCallback: (UnsealedWinRTClassWrapper<Composable>?, inout %.IInspectable?) -> Composable.Default.SwiftABI)
{
    super.init(composing: composing, createCallback)
}
)", w.support);
        }
        else
        {
            w.write(R"(^@_spi(WinRTInternal)
public init<Composable: ComposableImpl>(
    composing: Composable.Type,
    _ createCallback: (UnsealedWinRTClassWrapper<Composable>?, inout %.IInspectable?) -> Composable.Default.SwiftABI)
{
    super.init()
    MakeComposed(composing: composing, (self as! Composable.Class), createCallback)
}
)", w.support);
        }
    }

    static void write_composable_constructor(writer& w, attributed_type const& factory, class_type const& type)
    {
        if (auto factoryIface = dynamic_cast<const interface_type*>(factory.type))
        {
            w.write("private static var _% : %.% =  try! RoGetActivationFactory(\"%\")\n\n",
                    factory.type,
                    abi_namespace(factoryIface),
                    factory.type,
                    get_full_type_name(type));

            interface_info factory_info{ factoryIface };
            for (const auto& method : factoryIface->functions)
            {
                if (!can_write(w, method)) continue;

                auto baseHasMatchingConstructor = base_has_matching_constructor(type, factory, method);

                std::vector<function_param> params = get_projected_params(factory, method);

                w.write("%public init(%) {\n", baseHasMatchingConstructor ? "override " : "", bind<write_function_params2>(params, write_type_params::swift_allow_implicit_unwrap));
                {
                    auto indent = w.push_indent();
                    std::string_view func_name = get_abi_name(method);

                    auto return_name = method.return_type.value().name;
                    {
                        if (type.base_class)
                        {
                            w.write("super.init(composing: %.Composable.self) { baseInterface, innerInterface in \n", bind_bridge_name(type));
                        }
                        else
                        {
                            w.write("super.init()\n");
                            w.write("MakeComposed(composing: %.Composable.self, self) { baseInterface, innerInterface in \n", bind_bridge_name(type));
                        }
                        w.write("    try! Self.%.%(%)\n",
                            get_swift_name(factory_info),
                            func_name,
                            bind<write_implementation_args>(method));
                        w.write("}\n");
                    }
                }
                w.write("}\n\n");
            }
        }
    }

    static void write_composable_impl(writer& w, class_type const& parent, metadata_type const& overrides, bool compose);
    static void write_class_bridge(writer& w, class_type const& type)
    {
        if (auto default_interface = type.default_interface)
        {
            const bool composable = type.is_composable();
            w.write("public enum %: % {\n", bind_bridge_name(type), composable ? "ComposableBridge" : "AbiBridge");
            {
                auto indent = w.push_indent();
                w.write("public typealias SwiftProjection = %\n", type.swift_type_name());
                w.write("public typealias CABI = %\n", bind_type_mangled(default_interface));
                // We unwrap composable types to try and get to any derived type.
                // If not composable, then create a new instance
                w.write("public static func from(abi: ComPtr<%>?) -> %? {\n",
                    bind_type_mangled(default_interface), type);
                {
                    auto indent = w.push_indent();
                    w.write("guard let abi = abi else { return nil }\n");
                    if (type.is_composable())
                    {
                        w.write("return UnsealedWinRTClassWrapper<Composable>.unwrapFrom(base: abi)\n");
                    }
                    else
                    {
                        w.write("return .init(fromAbi: %.IInspectable(abi))\n", w.support);
                    }
                }
                w.write("}\n");

                if (composable)
                {
                    bool has_overrides = false;
                    for (const auto& [interface_name, info] : type.required_interfaces)
                    {
                        if (!info.overridable || interface_name.empty() || !can_write(w, info.type)) { continue; }

                        // Generate definitions for how to compose overloads and create wrappers of this type.
                        if (!info.base)
                        {
                            // the very first override is the one we use for composing the class and can respond to QI
                            // calls for all the other overloads
                            write_composable_impl(w, type, *info.type, !has_overrides);
                            has_overrides = true;
                        }
                        else if (info.base && !has_overrides)
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

                    if (!has_overrides)
                    {
                        write_composable_impl(w, type, *default_interface, true);
                    }
                }
            }
            w.write("}\n\n");
        }
    }

    static void write_default_constructor_declarations(writer& w, class_type const& type, metadata_type const& default_interface)
    {
        auto base_class = type.base_class;
        w.write("@_spi(WinRTInternal)\n");
        w.write("%public init(fromAbi: %.IInspectable) {\n",
            base_class ? "override " : "",
            w.support);
        {
            auto indent = w.push_indent();
            if (base_class)
            {
                w.write("super.init(fromAbi: fromAbi)\n");
            }
            else
            {
                w.write("super.init(fromAbi)\n");
            }
        }
        w.write("}\n\n");

        if (type.is_composable())
        {
            write_internal_composable_constructor(w, type);
        }
    }

    static void write_class_func_body(writer& w, function_def const& function, interface_info const& iface, bool is_noexcept)
    {
        std::string_view func_name = get_abi_name(function);
        auto impl = get_swift_name(iface);
        auto try_flavor = is_noexcept ? "try!" : "try";
        w.write("% %.%(%)\n",
            try_flavor,
            impl,
            func_name,
            bind<write_implementation_args>(function));
    }

    static std::string modifier_for(typedef_base const& type_definition, interface_info const& iface, member_type member)
    {
        std::string modifier;
        auto classType = dynamic_cast<const class_type*>(&type_definition);
        const bool isClass = classType != nullptr;
        if (isClass)
        {
            if (iface.overridable)
            {
                modifier = "open ";
            }
            else
            {
                modifier = "public ";
            }
        }
        else
        {
            modifier = "fileprivate ";
        }

        if (iface.attributed && isClass && classType->is_composable() && member == member_type::property_or_method)
        {
            modifier.append("class ");
        }
        else if (iface.attributed)
        {
            modifier.append("static ");
        }

        return modifier;
    }

    static std::string modifier_for(typedef_base const& type_definition, attributed_type const& attributedType, function_def const& func)
    {
        interface_info info { attributedType.type };
        info.attributed = true;
        auto modifier = modifier_for(type_definition, info);
        if (auto classType = dynamic_cast<const class_type*>(&type_definition))
        {
            if (base_has_matching_static_function(*classType, attributedType, func))
            {
                modifier.insert(0, "override ");
            }
        }
        return modifier;
    }


    static void write_class_impl_property(writer& w, property_def const& prop, interface_info const& iface, typedef_base const& type_definition)
    {
        if (!can_write(w, prop)) return;

        write_documentation_comment(w, type_definition, prop.def.Name());

        auto impl = get_swift_name(iface);

        if (prop.getter)
        {
            auto format = prop.is_array() ? "[%]" : "%";
            auto propertyType = w.write_temp(format, bind<write_type>(*prop.getter->return_type->type, swift_write_type_params_for(*iface.type, prop.is_array())));
            w.write("%var % : % {\n",
                modifier_for(type_definition, iface),
                get_swift_name(prop),
                propertyType);

            w.write("    get { try! %.%() }\n",
                impl,
                get_swift_name(prop.getter.value()));

            // TODO: https://linear.app/the-browser-company/issue/WIN-82/support-setters-not-defined-in-same-api-contract-as-getters
            // right now require that both getter and setter are defined in the same version
            if (prop.setter)
            {
                w.write("    set { try! %.%(newValue) }\n", impl, get_swift_name(prop.setter.value()));

            }
            w.write("}\n\n");
        }
    }

    static void write_class_impl_func(writer& w, function_def const& function, interface_info const& iface, typedef_base const& type_definition)
    {
        if (function.def.SpecialName() || !can_write(w, function))
        {
            // don't write methods which are really properties
            return;
        }

        write_documentation_comment(w, type_definition, function.def.Name());
        auto is_no_except = is_noexcept(*iface.type, function);
        auto type_params = swift_write_type_params_for(*iface.type);
        auto maybe_throws = is_no_except ? "" : " throws";
        w.write("%func %(%)%% {\n",
            modifier_for(type_definition, iface),
            get_swift_name(function),
            bind<write_function_params>(function, type_params),
            maybe_throws,
            bind<write_return_type_declaration>(function, type_params));
        {
            auto indent = w.push_indent();
            write_class_func_body(w, function, iface, is_no_except);
        }
        w.write("}\n\n");
    }

    static void write_class_impl_event(writer& w, event_def const& def, interface_info const& iface, typedef_base const& type_definition)
    {
        write_documentation_comment(w, type_definition, def.def.Name());

        auto event = def.def;
        auto format = R"(%var % : Event<%> = {
  .init(
    add: { [weak self] in
      guard let this = self?.% else { return .init() }
      return try! this.add_%($0)
    },
    remove: { [weak self] in
     try? self?.%.remove_%($0)
   }
  )
}()

)";

        auto static_format = R"(%var % : Event<%> = {
  .init(
    add: { try! %.add_%($0) },
    remove: { try? %.remove_%($0) }
  )
}()

)";
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

        auto modifier = modifier_for(type_definition, iface, member_type::event);
        if (!iface.attributed)
        {
            modifier.append("lazy ");
        }
        assert(delegate_method.def);
        w.write(iface.attributed ? static_format : format,
            modifier, // % var
            get_swift_name(event), // var %
            def.type, // Event<%>
            get_swift_name(iface), // weak this = %
            def.def.Name(), // add_&Impl
            get_swift_name(iface), // weak this = %
            def.def.Name() // remove_&Impl
        );
    }

    static void write_statics_body(writer& w, function_def const& method, metadata_type const& statics)
    {
        std::string_view func_name = get_abi_name(method);

        if (method.return_type)
        {
            w.write("return ");
        }
        w.write("try _%.%(%)\n",
            statics.swift_type_name(),
            func_name,
            bind<write_implementation_args>(method));
    }

    static void write_static_members(writer& w, attributed_type const& statics, class_type const& type)
    {
        if (auto ifaceType = dynamic_cast<const interface_type*>(statics.type))
        {
            interface_info static_info{ statics.type };
            static_info.attributed = true;

            auto impl_name = get_swift_name(static_info);
            w.write("private static let %: %.% = try! RoGetActivationFactory(\"%\")\n",
                impl_name,
                abi_namespace(statics.type),
                statics.type->swift_type_name(),
                get_full_type_name(type));

            for (const auto& method : ifaceType->functions)
            {
                if (!can_write(w, method))
                {
                    continue;
                }

                write_documentation_comment(w, type, method.def.Name());
                w.write("%func %(%) throws% {\n",
                    modifier_for(type, statics, method),
                    get_swift_name(method),
                    bind<write_function_params>(method, write_type_params::swift_allow_implicit_unwrap),
                    bind<write_return_type_declaration>(method, write_type_params::swift_allow_implicit_unwrap));
                {
                    auto indent = w.push_indent();
                    write_statics_body(w, method, *statics.type);
                }
                w.write("}\n\n");
            }

            for (const auto& static_prop : ifaceType->properties)
            {
                write_class_impl_property(w, static_prop, static_info, type);
            }

            for (const auto& event : ifaceType->events)
            {
                write_class_impl_event(w, event, static_info, type);
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

        auto format = R"(public enum % : ComposableImpl {
    public typealias CABI = %
    public typealias SwiftABI = %
    public typealias Class = %
    public typealias SwiftProjection = WinRTClassWeakReference<Class>
    public enum Default : AbiInterface {
        public typealias CABI = %
        public typealias SwiftABI = %
    }
}
)";

        auto composableName = w.write_temp("%", bind_type_abi(overrides));
        // If we're composing a type without any overrides, then we'll just create an IInspectable vtable which wraps
        // this object and provides facilities for reference counting and getting the class name of the swift object.
        w.write(format,
            composableName,
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
                    w.write("%.IInspectable", w.support);
                }
                else
                {
                    w.write("%.%", abi_namespace(overrides), composableName);
                }}),
            parent,
            bind_type_mangled(default_interface),
            bind([&](writer& w) {
                if (is_generic_inst(overrides))
                {
                    w.write("%.%", w.swift_module, composableName);
                }
                else
                {
                    w.write("%.%", abi_namespace(parent), default_interface);
                }
            }));

        if (compose)
        {
            w.write("@_spi(WinRTInternal)\n");
            w.write("public typealias Composable = %\n", composableName);
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

        for (const auto& [_, info] : overridable.required_interfaces)
        {
            if (!info.overridable || info.base) continue;

            w.write(R"(extension ComposableImpl where CABI == % {
    public static func makeAbi() -> CABI {
        let vtblPtr = withUnsafeMutablePointer(to: &%.%VTable) { $0 }
        return .init(lpVtbl: vtblPtr)
    }
}
)", bind_type_mangled(info.type),
    abi_namespace(info.type),
    info.type->swift_type_name());
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

        write_documentation_comment(w, type);
        if (base_class)
        {
            w.write("% class % : %", modifier, typeName, get_full_swift_type_name(w, base_class));
        }
        else if (default_interface)
        {
            w.write("% class % : WinRTClass",
                modifier,
                typeName);
        }
        else
        {
            w.write("public final class %", typeName);
        }

        auto interfaces = type.required_interfaces;
        separator s{ w };
        s();

        bool needsCustomQueryInterfaceConformance = composable;
        bool baseHasCustomQueryInterfaceConformance = base_class && base_class->is_composable();
        // list of overridable interfaces which are needed for the implementation of CustomQueryInterface.
        // when we get a delegating QI for one of these interfaces, we want to return ourselves instead of
        // delegating to the inner non-delegating QI. For any other interface, we will delegate to the inner
        // as that will be where the implementation is.
        std::vector<named_interface_info> overridable_interfaces;
        for (const auto& [interface_name, info] : type.required_interfaces)
        {
            if (info.base && !info.exclusive && !interface_name.empty())
            {
                baseHasCustomQueryInterfaceConformance = true;
            }
            if (info.overridable && !info.base && composable)
            {
                // overridable interfaces are still considered exclusive, so check here before
                // we skip this interface
                overridable_interfaces.push_back({ interface_name, info });
                // Don't need to set needsCustomQueryInterfaceConformance here since we set it to true
                // for composable types and an overridable interface implies composibility, but doing so for
                // posterity
                needsCustomQueryInterfaceConformance = true;
            }
            // Filter out which interfaces we actually want to declare on the class.
            // We don't want to specify interfaces which come from the base class or which ones are exclusive
            if (info.base || info.exclusive || interface_name.empty()) continue;

            s();
            // if the class also implements an interface, then it will need to conform to the protocol.
            needsCustomQueryInterfaceConformance = true;

            // when deriving from collections we want to just derive from `IVector` and will use a typealias to set the Element (this is required by Swift)
            auto name_to_write = interface_name;
            if (is_generic_inst(info.type))
            {
                name_to_write = interface_name.substr(0, interface_name.find_first_of('<'));
            }
            w.add_depends(*info.type);
            w.write(name_to_write);
        }

        w.write(" {\n");

        auto class_indent_guard = w.push_indent();

        write_generic_typealiases(w, type);

        writer::generic_param_guard guard;

        if (default_interface)
        {
            auto [ns, name] = get_type_namespace_and_name(*default_interface);
            auto swiftAbi = w.write_temp("%.%", abi_namespace(ns), name);
            std::string defaultVal = "";
            if (is_generic_inst(default_interface))
            {
                auto generic_type = dynamic_cast<const generic_inst*>(default_interface);
                guard = w.push_generic_params(*generic_type);
                swiftAbi = w.write_temp("%.%", w.swift_module, bind_type_abi(generic_type));
            }

            auto modifier = composable ? "open" : "public";

            w.write(R"(private typealias SwiftABI = %
private typealias CABI = %
private lazy var _default: SwiftABI! = getInterfaceForCaching()
^@_spi(WinRTInternal)
override % func _getABI<T>() -> UnsafeMutablePointer<T>? {
    if T.self == CABI.self {
        return RawPointer(_default)
    }
    return super._getABI()
}

)",
                swiftAbi,
                bind_type_mangled(default_interface),
                modifier);
            write_default_constructor_declarations(w, type, *default_interface);

            // composable types will always need CustomQueryInterface conformance so that derived types can
            // override the queryInterface call
            if (needsCustomQueryInterfaceConformance)
            {
                w.write("override % func queryInterface(_ iid: %.IID) -> IUnknownRef? {\n", modifier, w.support);

                // A WinRTClass needs CustomQueryInterface conformance when it derives from 1 or more interfaces,
                // otherwise it won't compile. At the end of the day, the winrt object it's holding onto will appropriately
                // respond to QueryInterface calls, so call into the default implementation.
                auto baseComposable = type.base_class && type.base_class->is_composable();
                std::string base_case = "super.queryInterface(iid)";
                if (overridable_interfaces.empty())
                {
                    w.write("    return %\n", base_case);
                }
                else
                {
                    w.write("    switch iid {\n");
                    for (auto& [_, info] : overridable_interfaces) {
                        auto indent{ w.push_indent({2}) };
                        w.write("%", bind<write_query_interface_case>(info));
                    }
                    w.write("        default: return %\n", base_case);
                    w.write("    }\n");
                }
                w.write("}\n");
            }
        }
        for (const auto& [interface_name, factory] : type.factories)
        {
            if (factory.activatable)
            {
                write_factory_constructors(w, factory, type, *default_interface);
            }

            if (factory.statics)
            {
                write_static_members(w, factory, type);
            }

            if (factory.composable)
            {
                write_composable_constructor(w, factory, type);
            }
        }

        bool has_collection_conformance = false;
        std::vector<std::string> interfaces_to_release;
        for (const auto& [interface_name, info] : type.required_interfaces)
        {
            if (interface_name.empty() || !can_write(w, info.type)) { continue; }

            auto guard2{ w.push_generic_params(info) };
            if (needs_collection_conformance(info.type) && !info.base && !has_collection_conformance)
            {
                has_collection_conformance = true;
                write_collection_protocol_conformance(w, info);
            }

            // Don't reimplement P/M/E for interfaces which are implemented on a base class
            if (!info.base)
            {
                // this is an overridable interface but the type can't actually
                // be overriden, so skip it
                if (info.overridable && !composable)
                {
                    continue;
                }
                interfaces_to_release.push_back(get_swift_name(info));
                write_interface_impl_members(w, info, /* type_definition: */ type);
            }
        }

        if (default_interface)
        {
            w.write("deinit {\n");
            for (const auto& iface : interfaces_to_release)
            {
                w.write("    % = nil\n", iface);
            }
            w.write("}\n");
        }

        class_indent_guard.end();
        w.write("}\n\n");
    }

    static void write_class(writer& w, class_type const& type)
    {
        write_class_impl(w, type);
    }

    static void write_query_interface_case(writer& w, interface_info const& iface)
    {
        w.write("case %.IID:\n", bind_wrapper_fullname(iface.type));
        w.write("    let wrapper = %(self)\n", bind_wrapper_fullname(iface.type));
        w.write("    return wrapper!.queryInterface(iid)\n");
    }

    static void write_iunknown_methods(writer& w, metadata_type const& type)
    {
        auto wrapper_name = w.write_temp("%", bind_wrapper_name(type));
        w.write("QueryInterface: { %.queryInterface($0, $1, $2) },\n", wrapper_name);
        w.write("AddRef: { %.addRef($0) },\n", wrapper_name);
        w.write("Release: { %.release($0) },\n", wrapper_name);
    }

    template <typename T>
    static void write_iinspectable_methods(writer& w, T const& type, std::vector<named_interface_info> const& interfaces, bool composed = false)
    {
        // 3 interfaces for IUnknown, IInspectable, type.
        auto interface_count = 3 + interfaces.size();
        w.write("GetIids: {\n");
        {
            auto indent_guard = w.push_indent();
            w.write("let size = MemoryLayout<%.IID>.size\n", w.support);
            w.write("let iids = CoTaskMemAlloc(UInt64(size) * %).assumingMemoryBound(to: %.IID.self)\n", interface_count, w.support);
            w.write("iids[0] = IUnknown.IID\n");
            w.write("iids[1] = IInspectable.IID\n");
            w.write("iids[2] = %.IID\n", bind_wrapper_fullname(type));

            auto iface_n = 3;
            for (const auto& iface : interfaces)
            {
                if (!can_write(w, iface.second.type)) continue;

                w.write("iids[%] = %.IID\n",
                    iface_n++,
                    bind_wrapper_fullname(iface.second.type)
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
    guard let instance = %.tryUnwrapFrom(raw: $0) else { return E_INVALIDARG }
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
        write_iinspectable_methods(w, type, type.required_interfaces);
    }

    // assigns return or out parameters in vtable methods
    template<typename T>
    static void do_write_abi_val_assignment(writer& w, T const& return_type, std::string_view return_param_name)
    {
        auto type = return_type.type;
        auto param_name = get_swift_member_name(return_type.name);
        TypeDef signature_type{};
        auto category = get_category(type, &signature_type);

        if (return_type.is_array())
        {
            if (is_reference_type(type))
            {
                w.write("%.fill(abi: %, abiBridge: %.self)\n",
                    param_name, return_param_name, bind_bridge_fullname(*type));
            }
            else if (category == param_category::enum_type || category == param_category::fundamental_type)
            {
                w.write("%.fill(abi: %)\n",
                    param_name, return_param_name);
            }
            else
            {
                w.write(R"(do {
    try %.fill(abi: %)
} catch { return failWith(error: error) }
)", param_name, return_param_name);
            }
            return;
        }
        else if (category == param_category::struct_type && !is_struct_blittable(signature_type))
        {
            w.write("let _% = %._ABI_%(from: %)\n\t",
                param_name,
                abi_namespace(type),
                type->swift_type_name(),
                param_name);
        }
        else if (is_reference_type(type))
        {
            if (!is_class(type))
            {
                w.write("let %Wrapper = %(%)\n",
                    param_name,
                    bind_wrapper_fullname(type),
                    param_name);
                w.write("%Wrapper?.copyTo(%)\n", param_name, return_param_name);
            }
            else
            {
                w.write("%?.copyTo(%)\n", param_name, return_param_name);
            }
            return;
        }

        w.write("%?.initialize(to: %)\n",
            return_param_name,
            bind([&](writer& w) {
                write_convert_to_abi_arg(w, return_type);
            })
        );
    }

    static void write_abi_ret_val(writer& w, function_def signature)
    {
        int param_number = 1;
        for (auto& param : signature.params)
        {
            auto param_name = get_swift_name(param);
            if (param.is_array())
            {
                if (param.signature.ByRef())
                {
                    w.write("$%?.initialize(to: UInt32(%.count))\n", param_number, param_name);
                }
                param_number++;
            }
            if (param.out())
            {
                auto return_param_name = "$" + std::to_string(param_number);
                do_write_abi_val_assignment(w, param, return_param_name);
            }

            param_number++;
        }

        if (signature.return_type)
        {
            if (signature.return_type.value().is_array())
            {
                w.write("$%?.initialize(to: UInt32(%.count))\n", param_number, signature.return_type.value().name);
                param_number++;
            }
            auto return_param_name = "$" + std::to_string(param_number);
            do_write_abi_val_assignment(w, signature.return_type.value(), return_param_name);
        }
    }

    static void write_not_implementable_vtable_method(writer& w, function_def const& sig)
    {
        w.write("%: { _, % in return failWith(hr: E_NOTIMPL) }", get_abi_name(sig), bind([&](writer& w) {
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
        if (!can_write(w, function, true))
        {
            write_not_implementable_vtable_method(w, function);
            return;
        }
        // we have to write the methods in a certain order and so we will have to detect here whether
        // this method is a property getter/setter and adjust the call so we use the swift syntax
        std::string func_call;
        bool is_get_or_put = true;
        if (is_get_overload(method))
        {
            if (!is_winrt_ireference(&type))
            {
                func_call += w.write_temp(".%", get_swift_name(method));
            }
        }
        else if (is_put_overload(method))
        {
            func_call += w.write_temp(".% = %", get_swift_name(method), bind<write_consume_args>(function));
        }
        else if (is_add_overload(method))
        {
            func_call += w.write_temp(".%.addHandler(%)", get_swift_name(method), bind<write_consume_args>(function));
        }
        else if (is_remove_overload(method))
        {
            assert(function.params.size() == 1);
            func_call += w.write_temp(".%.removeHandler(%)", get_swift_name(method), function.params[0].name);
        }
        else
        {
            is_get_or_put = false;
            // delegate arg types are a tuple, so wrap in an extra paranthesis
            auto format = is_delegate(type) ? "%(%)" : ".%(%)";
            func_call += w.write_temp(format, get_swift_name(method), bind<write_consume_args>(function));
        }

        bool needs_try_catch = !is_get_or_put && !is_winrt_generic_collection(type);
        w.write("%: {\n", func_name);
        if (needs_try_catch) {
            w.m_indent += 1;
            w.write("do {\n");
        }
        {
            auto indent_guard = w.push_indent();
            w.write("guard let __unwrapped__instance = %.tryUnwrapFrom(raw: $0) else { return E_INVALIDARG }\n",
                bind_wrapper_name(type));

            write_convert_vtable_params(w, function);

            if (function.return_type)
            {
                w.write("let % = ", function.return_type.value().name);
            }

            w.write("%__unwrapped__instance%\n",
                needs_try_catch ? "try " : "",
                func_call);

            write_abi_ret_val(w, function);
            w.write("return S_OK\n");
        }
        if (needs_try_catch) {
            w.write("} catch { return failWith(error: error) }\n");
            w.m_indent -= 1;
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
        w.write("internal %var %VTable: %Vtbl = .init(\n",
            isGeneric ? "" : "static ", // generics aren't namespaced since implementations are always internal
            type,
            bind_type_mangled(type));
        {
            auto indent = w.push_indent();
            write_iunknown_methods(w, type);
            separator s{ w, ",\n\n" };

            if (!is_delegate(type))
            {
                write_iinspectable_methods(w, type, interfaces);
                s(); // get first separator out of the way for no-op
            }

            for (const auto& method : type.functions)
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
            bind_bridge_fullname(type),
            overrides.type
        );
        w.write("internal static var %VTable: %Vtbl = .init(\n",
            overrides.type,
            bind_type_mangled(overrides.type));

        {
            auto indent = w.push_indent();
            write_iunknown_methods(w, *overrides.type);
            write_iinspectable_methods(w, overrides.type, other_interfaces, true);

            separator s{ w, ",\n\n" };
            s(); // get first separator out of the way for no-op

            if (auto iface = dynamic_cast<const interface_type*>(overrides.type))
            {
                for (const auto& method : iface->functions)
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
            const auto& [interface_name, info] = *iter;
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
