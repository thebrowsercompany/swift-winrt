#pragma once
#include "code_writers/common_writers.h"
#include "code_writers/type_writers.h"
#include "code_writers/can_write.h"
#include "metadata_cache.h"
namespace swiftwinrt
{
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
        w.write("}\n");

        w.write("extension %: Hashable, Codable {}\n\n", get_full_swift_type_name(w, type));
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

    static void write_function_params(writer& w, function_def const& function, write_type_params const& type_params)
    {
        separator s{ w };

        for (auto&& param : function.params)
        {
            s();

            w.write("_ %: ", get_swift_name(param));
            if (param.out()) w.write("inout ");
            write_type(w, *param.type, type_params);
        }
    }

    static void write_convert_to_abi_arg(writer& w, std::string_view const& param_name, const metadata_type* type, bool is_out)
    {
        TypeDef signature_type;
        auto category = get_category(type, &signature_type);

        if (category == param_category::object_type)
        {
            if (is_out) throw std::exception("out parameters of reference types should not be converted directly to abi types");

            if (is_class(signature_type))
            {
                w.write("RawPointer(%)", param_name);
            }
            else
            {
                w.write("_%", param_name);
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
            if (is_out) throw std::exception("out parameters of generic types should not be converted directly to abi types");
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

    static void write_init_return_val_abi(writer& w, function_return_type const& signature)
    {
        auto category = get_category(signature.type);
        auto guard{ w.push_mangled_names_if_needed(category) };
        write_type(w, *signature.type, write_type_params::c_abi);
        write_default_init_assignment(w, *signature.type, projection_layer::c_abi);
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

    static void write_return_type_declaration(writer& w, function_def function, write_type_params const& type_params)
    {
        if (!function.return_type)
        {
            return;
        }

        w.write(" -> ");
        write_type(w, *function.return_type->type, type_params);
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
        auto baseClass = (is_delegate(type) || !type.type().Flags().WindowsRuntime()) ? "IUnknown" : "IInspectable";
        w.write("% class %: %.%% {\n",
            internal ? "internal" : "open",
            type,
            w.support,
            baseClass,
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
                w.write("% func %Impl(%) throws% {\n",
                    internal || is_exclusive(type) ? "internal" : "open",
                    func_name,
                    bind<write_function_params>(function, write_type_params::c_abi),
                    bind<write_return_type_declaration>(function, write_type_params::c_abi));
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

            if (!is_winrt_ireference(type))
            {
                auto generic_params = w.push_generic_params(type);
                do_write_interface_abi(w, *type.generic_type(), type.functions);
            }
        }
    }

    static void write_ireference_init_extension(writer& w, generic_inst const& type)
    {
        if (!is_winrt_ireference(type)) return;

        auto format = R"(internal extension % {
    init?(ref: UnsafeMutablePointer<%>?) {
        guard let val = ref else { return nil }
        var result: %%
        try! CHECKED(val.pointee.lpVtbl.pointee.get_Value(val, &result))
        %
    }
} 
)";
        auto generic_param = type.generic_params()[0];
        w.add_depends(*generic_param);
        if (auto structType = dynamic_cast<const struct_type*>(generic_param))
        {
            w.write(format,
                get_full_swift_type_name(w, structType),
                type.mangled_name(),
                structType->mangled_name(),
                bind<write_default_init_assignment>(*structType, projection_layer::c_abi),
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
                bind<write_default_init_assignment>(*generic_param, projection_layer::c_abi),
                blittable ? "self = result" : "self.init(from: result)");
        }
      
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
        else if (is_winrt_eventhandler(inst) || is_winrt_typedeventhandler(inst))
        {
            auto guard{ w.push_generic_params(inst) };
            write_delegate_extension(w, inst, inst.functions[0]);
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
        auto wrapper_name = w.write_temp("%", bind_wrapper_name(type));
        auto format = R"(
typealias % = InterfaceWrapperBase<%>
)";
        w.write(format, wrapper_name, impl_name);
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
                for (auto&& field : type.members)
                {
                    // WIN-64 - swiftwinrt: support boxing/unboxing
                    // WIN-65 - swiftwinrt: support generic types
                    if (can_write(w, field.type))
                    {
                        std::string from = std::string("swift.").append(get_swift_name(field));
                        w.write("val.% = %\n",
                            get_abi_name(field),
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
                        w.write("val.% = nil\n", get_abi_name(member));
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
                        w.write("WindowsDeleteString(val.%)\n", get_abi_name(member));
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

                    if (is_delegate(param.type))
                    {
                        w.write("guard let % = % else { return E_INVALIDARG }\n",
                            get_swift_name(param),
                            bind<write_consume_type>(param.type, param_name));
                    }
                    else
                    {
                        w.write("let %: % = %\n",
                            get_swift_name(param),
                            bind<write_type>(*param.type, write_type_params::swift),
                            bind<write_consume_type>(param.type, param_name));
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
        auto return_param_name = signature.return_type.value().name;
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
        auto winrtInterfaceConformance = w.write_temp(R"(
    public func queryInterface(_ riid: REFIID, _ ppvObj: UnsafeMutablePointer<LPVOID?>?) -> HRESULT { 
         if riid.pointee == __ABI_Windows_Foundation.IPropertyValueWrapper.IID {
            guard let thisAsIPropValue = __ABI_Windows_Foundation.IPropertyValueWrapper(self) else { fatalError("creating non-nil wrapper shouldn't fail") }
            return thisAsIPropValue.queryInterface(riid, ppvObj)
        } else {
            return E_NOINTERFACE
        }
    }
)");

        w.write(R"(public class IPropertyValueImpl : IPropertyValue, IReference {
    var _value: Any
    var propertyType : PropertyType

    public init(value: Any) {
        _value = value
        if _value is Int32 {
            propertyType = .int32
        } else if _value is UInt8 {
            propertyType = .uint8
        } else if _value is Int16 {
            propertyType = .int16
        } else if _value is UInt32 {
            propertyType = .uint32
        } else if _value is Int64 {
            propertyType = .int64
        } else if _value is UInt64 {
            propertyType = .uint64
        } else if _value is Float {
            propertyType = .single
        } else if _value is Double {
            propertyType = .double
        } else if _value is Character {
            propertyType = .char16
        } else if _value is Bool {
            propertyType = .boolean
        } else if _value is DateTime {
            propertyType = .dateTime
        } else if _value is TimeSpan {
            propertyType = .timeSpan
        } else if _value is IWinRTObject {
            propertyType = .inspectable
        } else if _value is IInspectable {
            propertyType = .inspectable
        } else {
            propertyType = .otherType
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
    public func getGuid() -> UUID { _value as! UUID }
    public func getDateTime() -> DateTime { _value as! DateTime } 
    public func getTimeSpan() -> TimeSpan { _value as! TimeSpan }
    public func getPoint() -> Point { _value as! Point }
    public func getSize() -> Size { _value as! Size }
    public func getRect() -> Rect { _value as! Rect }
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
        if (typeName.starts_with("IVector"))
        {
            w.write(R"(// MARK: Collection
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
)", modifier, modifier, modifier, modifier, modifier);
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

    static void write_interface_impl_members(writer& w, interface_info const& info, bool is_class)
    {
        if (info.overridable && is_class)
        {
            // when implementing default overrides, we want to call to the inner non-delegating IUnknown
            // as this will get us to the inner object. otherwise we'll end up with a stack overflow 
            // because we'll be calling the same method on ourselves
            w.write("internal lazy var %: %.% = try! IUnknown(_inner!).QueryInterface()\n",
                get_swift_name(info),
                abi_namespace(info.type->swift_logical_namespace()),
                info.type->swift_type_name());

        }
        else if (!info.is_default || (!is_class && info.base))
        {
            auto swiftAbi = w.write_temp("%.%", abi_namespace(info.type->swift_logical_namespace()), info.type->swift_type_name());
            if (is_winrt_generic_collection(info.type))
            {
                w.generic_param_stack.push_back(info.generic_params);
                writer::generic_param_guard guard{ &w };
                swiftAbi = w.write_temp("%", bind_type_abi(info.type));
            }
            w.write("internal lazy var %: % = try! _default.QueryInterface()\n",
                get_swift_name(info),
                swiftAbi);
        }

        if (is_class && is_winrt_generic_collection(info.type))
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
        else
        {
            assert(!"Unexpected interface type.");
        }
    }

    static void write_interface_abi_bridge(writer& w, interface_type const& type)
    {
        if (is_exclusive(type) || !can_write(w, type) ||get_full_type_name(type) == "Windows.Foundation.IPropertyValue") return;

        w.write(R"(^@_spi(__MakeFromAbi_DoNotImport)
public class %_MakeFromAbi : MakeFromAbi {
    public typealias CABI = %
    public typealias SwiftABI = %.%
    public typealias SwiftProjection = %
    public static func from(abi: UnsafeMutableRawPointer?) -> SwiftProjection? {
        guard let abi else { return nil }
        let swiftAbi: SwiftABI = try! %.IInspectable(abi).QueryInterface()
        return %(RawPointer(swiftAbi)!)
    }
}

)",
            type,
            bind_type_mangled(type),
            abi_namespace(type),
            type,
            bind<write_swift_interface_existential_identifier>(type), // Do not include outer Optional<>)
            w.support,
            bind_impl_fullname(type)
        );
    }

    static void write_interface_impl(writer& w, interface_type const& type)
    {
        if (is_exclusive(type) || !can_write(w, type)) return;

        if (get_full_type_name(type) == "Windows.Foundation.IPropertyValue")
        {
            write_property_value_impl(w);
            return;
        }

        auto format = "public class % : %, WinRTAbiBridge {\n";
        w.write(format, bind_impl_name(type), type);

        auto class_indent_guard = w.push_indent();

        w.write(R"(public typealias CABI = %
public typealias SwiftABI = %.%
public typealias SwiftProjection = %
private (set) public var _default: SwiftABI
public var thisPtr: %.IInspectable { _default }
public static func from(abi: UnsafeMutablePointer<CABI>?) -> SwiftProjection? {
    guard let abi = abi else { return nil }
    return %(abi)
}
public init(_ fromAbi: UnsafeMutablePointer<CABI>) {
    _default = SwiftABI(fromAbi)
}

public static func makeAbi() -> CABI {
    let vtblPtr = withUnsafeMutablePointer(to: &%.%VTable) { $0 }
    return .init(lpVtbl: vtblPtr)
}
)",
            bind_type_mangled(type),
            abi_namespace(type),
            type,
            bind<write_swift_interface_existential_identifier>(type), // Do not include outer Optional<>
            w.support,
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
        
        for (auto&& event : type.events)
        {
            write_class_impl_event(w, event, type_info);
        }

        for (auto&& [interface_name, info] : type.required_interfaces)
        {
            if (!can_write(w, info.type)) { continue; }

            write_interface_impl_members(w, info, /* is_class: */ false);
        }

        class_indent_guard.end();
        w.write("}\n\n");
    }

    static void write_comma_param_types(writer& w, std::vector<function_param> const& params);
    static void write_delegate_return_type(writer& w, function_def const& sig);

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
            result = handler(%)
        }
        return result)",
                bind<write_type>(*delegate_method.return_type->type, write_type_params::swift),
                bind<write_default_init_assignment>(*delegate_method.return_type->type, projection_layer::swift),
                bind<write_comma_param_names>(delegate_method.params));
        }
        else
        {
            invoke_implementation = w.write_temp(R"(for handler in getInvocationList() {
            handler(%)
        })", bind<write_comma_param_names>(delegate_method.params));
        }
        
        assert(delegate_method.def);
        w.write(R"(% extension EventSource where Handler == % {
    %func invoke(%)% {
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

        auto typeName = type.swift_type_name();
        bool is_property_set = typeName == "IPropertySet"sv;
        auto interfaces = type.required_interfaces;
        separator s{ w };
        auto implements = w.write_temp("%", bind_each([&](writer& w, std::pair<std::string, interface_info> const& iface) {
            // TODO: https://linear.app/the-browser-company/issue/WIN-103/swiftwinrt-write-iasyncinfo
            if (!iface.first.ends_with("IAsyncInfo") && can_write(w, iface.second.type))
            {
                s();
                write_swift_type_identifier(w, *iface.second.type);
            }}, interfaces));


        std::vector<std::string> eventSourceInvokeLines;
        w.write("public protocol % : %% {\n", type, implements,
            implements.empty() ? "WinRTInterface" : "");
        {
            auto body_indent = w.push_indent();
            for (auto& method : type.functions)
            {
                if (!can_write(w, method)) continue;

                auto full_type_name = w.push_full_type_names(true);
                auto maybe_throws = is_noexcept(method.def) ? "" : " throws";
                w.write("func %(%)%%\n",
                    get_swift_name(method),
                    bind<write_function_params>(method, write_type_params::swift_allow_implicit_unwrap),
                    maybe_throws,
                    bind<write_return_type_declaration>(method, write_type_params::swift_allow_implicit_unwrap));
            }

            for (auto& prop : type.properties)
            {
                if (!can_write(w, prop)) continue;
                auto full_type_name = w.push_full_type_names(true);
                auto&& return_type = *prop.getter->return_type->type;
                w.write("var %: % { get% }\n",
                    get_swift_name(prop),
                    bind<write_type>(return_type, write_type_params::swift_allow_implicit_unwrap),
                    prop.setter ? " set" : "");
            }

            for (auto& event : type.events)
            {
                w.write("var %: Event<%> { get }\n",
                    get_swift_name(event.def),
                    event.type);
                // only write the eventsource extension for interfaces which could be implemented by a swift object
                // not only does this result in less code generated, it also helps alleviate the issue where different 
                // interfaces define an event with the same type. For that scenario, we cache the event type on the
                // writer
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
        w.write("}\n\n");

        for (const auto& line : eventSourceInvokeLines)
        {
            w.write(line);
        }
        if (type.swift_full_name() != "Windows.Foundation.IPropertyValue")
        {
            // write default queryInterface implementation for this interface. don't do
            // it for IPropertyValue since this has a custom wrapper implementation
            w.write(R"(extension % {
    public func queryInterface(_ riid: REFIID, _ ppvObj: UnsafeMutablePointer<LPVOID?>?) -> HRESULT {
        guard let wrapper = %(self) else { fatalError("created abi was null")  }
        return wrapper.queryInterface(riid, ppvObj)
    }
}
)", typeName, bind_wrapper_fullname(type));
        }

        // Declare a short form for the existential version of the type, e.g. AnyClosable for "any IClosable"
        w.write("public typealias Any% = any %\n\n", typeName, typeName);
    }

    static void write_ireference(writer& w)
    {
        w.write(R"(public protocol IReference : IPropertyValue {
    var value: Any { get }
}
)");
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
            write_type(w, *param.type, write_type_params::swift);
        }
    }

    static void write_delegate(writer& w, delegate_type const& type)
    {
        function_def delegate_method = type.functions[0];
        w.write("public typealias % = (%) -> %\n",
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

    % static func from(abi: UnsafeMutablePointer<CABI>?) -> SwiftProjection? {
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
        auto handlerType = w.write_temp("%", type);
        auto abi_guard = w.push_abi_types(is_generic);
        w.write(format,
            access_level,
            bind_impl_name(type),
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
                write_class_func_body(w, invoke_method, delegate, true);
                }));
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

        auto&& generic_params = type.generic_params();
        if (type.swift_type_name().starts_with("IVector")) // IVector and IVectorView
        {
            w.write("typealias Element = %\n", bind<write_type>(*generic_params[0], write_type_params::swift));
        }
        else if (type.swift_type_name().starts_with("IMap")) // IMap and IMapView
        {
            w.write("typealias Key = %\n", bind<write_type>(*generic_params[0], write_type_params::swift));
            w.write("typealias Value = %\n", bind<write_type>(*generic_params[1], write_type_params::swift));
        }
        else
        {
            assert(!"Unexpected collection type");
        }

        w.write("typealias SwiftProjection = %\n",
            bind<write_swift_interface_existential_identifier>(type)); // Do not include outer Optional<>

        w.write("typealias CABI = %\n", bind_type_mangled(type));
        w.write("typealias SwiftABI = %\n", bind_type_abi(type));
        w.write("\n");
        w.write("private (set) public var _default: SwiftABI\n");
        w.write("\n");

        w.write("static func from(abi: UnsafeMutablePointer<CABI>?) -> SwiftProjection? {\n");
        w.write("    guard let abi = abi else { return nil }\n");
        w.write("    return %(abi)\n", bind_impl_name(type));
        w.write("}\n\n");

        w.write("internal init(_ fromAbi: UnsafeMutablePointer<CABI>) {\n");
        w.write("    _default = SwiftABI(fromAbi)\n");
        w.write("}\n\n");

        w.write("static func makeAbi() -> CABI {\n");
        w.write("    let vtblPtr = withUnsafeMutablePointer(to: &%VTable) { $0 }\n",
            bind_type_mangled(type));
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
        for (auto&& event : type.events)
        {
            write_class_impl_event(w, event, info);
        }

        w.write(R"(public func queryInterface(_ riid: REFIID, _ ppvObj: UnsafeMutablePointer<LPVOID?>?) -> HRESULT {
    return E_NOINTERFACE
} 
)");
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
        else if (is_winrt_generic_collection(type))
        {
            write_collection_implementation(w, type);
        }
    }

    // When converting from Swift <-> C we put some local variables on the stack in order to help facilitate
    // converting between the two worlds. This method will returns a scope guard which will write any necessary
    // code for after the ABI function is called (such as cleaning up references).
    static write_scope_guard<writer> write_local_param_wrappers(writer& w, function_def const& signature)
    {
        write_scope_guard guard{ w, w.swift_module };

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
                else if (is_reference_type(param.type) && !is_class(param.type))
                {
                    w.write("let %Wrapper = %(%)\n",
                        get_swift_name(param),
                        bind_wrapper_fullname(param.type),
                        get_swift_name(param));
                    w.write("let _% = try! %Wrapper?.toABI { $0 }\n",
                        get_swift_name(param),
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
                    auto varName = w.write_temp("_%", get_swift_name(param));
                    w.write("var %: %\n",
                        varName,
                        bind<write_type>(*param.type, write_type_params::c_abi));

                    guard.push("% = %\n", get_swift_name(param),
                        bind<write_consume_type>(param.type, varName));
                }
                else if (category == param_category::generic_type)
                {
                    w.write("var _%: %\n",
                        get_swift_name(param),
                        bind<write_type>(*param.type, write_type_params::c_abi));

                    guard.push("% = %.from(abi: _%)\n",
                        get_swift_name(param),
                        bind_impl_fullname(param.type),
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
        w.write("_default = %.%(consuming: %!)\n", abi_namespace(type), default_interface, return_name);
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

                w.write("public init(%) {\n", bind<write_function_params>(method, write_type_params::swift_allow_implicit_unwrap));
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
    self._default = try! MakeComposed(factory, &_inner, self as! Factory.Composable.Default.SwiftProjection).QueryInterface()
    _ = self._default.Release() // release to reset reference count since QI caused an AddRef on ourselves
}
)";
                w.write(base_composable_init, factory);
            }
            else
            {
                auto override_composable_init = R"(override public init() {
    super.init(Self._%) 
    let parentDefault: UnsafeMutablePointer<%> = super._getABI()!
    self._default = try! IInspectable(parentDefault).QueryInterface()
    _ = self._default.Release() // release to reset reference count since QI caused an AddRef on ourselves
}

override public init<Factory: ComposableActivationFactory>(_ factory: Factory) {
    super.init(factory)
    let parentDefault: UnsafeMutablePointer<%> = super._getABI()!
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
        w.write("public static func from(abi: UnsafeMutablePointer<%>?) -> %? {\n",
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
                w.write("return .init(fromAbi: .init(abi))\n");
            }
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

    static void write_class_func_body(writer& w, function_def const& function, interface_info const& iface, bool is_noexcept)
    {
        std::string_view func_name = get_abi_name(function);
        {
            auto guard = write_local_param_wrappers(w, function);

            auto impl = get_swift_name(iface);

            auto try_flavor = is_noexcept ? "try!" : "try";
            if (function.return_type)
            {
                w.write("let % = % %.%Impl(%)\n",
                    function.return_type.value().name,
                    try_flavor,
                    impl,
                    func_name,
                    bind<write_implementation_args>(function));
            }
            else
            {
                w.write("% %.%Impl(%)\n",
                    try_flavor,
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
                bind<write_type>(*prop.getter->return_type->type, write_type_params::swift_allow_implicit_unwrap));
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
            auto propn = get_swift_name(prop);
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
            else if (is_reference_type(prop.type) && !is_class(prop.type))
            {
                w.write("let wrapper = %(newValue)\n", bind_wrapper_fullname(prop.type));
                w.write("let _newValue = try! wrapper?.toABI { $0 }\n");
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

        // When implementing a generic collection interface,
        // we cannot use implicit unwrapping, because of Swift limitations:
        // typename Element must be Base? and not Base!,
        // and declaring GetAt(_: UInt32) -> Base! would not bind to GetAt(_: UInt32) -> Element.
        auto is_winrt_collection = is_winrt_generic_collection(iface.type);
        auto&& type_params = is_winrt_collection
            ? write_type_params::swift : write_type_params::swift_allow_implicit_unwrap;
        auto maybe_throws = is_winrt_collection ? "" : " throws";
        w.write("% func %(%)%% {\n",
            iface.overridable ? "open" : "public",
            get_swift_name(function),
            bind<write_function_params>(function, type_params),
            maybe_throws,
            bind<write_return_type_declaration>(function, type_params));
        {
            auto indent = w.push_indent();
            write_class_func_body(w, function, iface, is_winrt_collection);
        }
        w.write("}\n\n");
    }
  
    static void write_class_impl_event(writer& w, event_def const& def, interface_info const& iface)
    {
        auto event = def.def;
        auto format = R"(public % var % : Event<%> = {
  .init(
    add: { [weak this = %] in
      guard let this else { return .init() }
      let wrapper = %($0)
      let abi = try! wrapper?.toABI { $0 }
      return try! this.add_%Impl(abi)
    },
    remove: { [weak this = %] in
     try? this?.remove_%Impl($0) 
   }
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
        
        assert(delegate_method.def);
        w.write(format,
            iface.attributed ? "static" : "lazy", // public %
            get_swift_name(event), // var %
            def.type, // Event<%>
            get_swift_name(iface), // weak this = %
            bind_wrapper_fullname(def.type), // let wrapper = %
            def.def.Name(), // add_&Impl
            get_swift_name(iface), // weak this = %
            def.def.Name() // remove_&Impl
        );
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

                w.write("public static func %(%)% {\n",
                    get_swift_name(method),
                    bind<write_function_params>(method, write_type_params::swift_allow_implicit_unwrap),
                    bind<write_return_type_declaration>(method, write_type_params::swift_allow_implicit_unwrap));
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
    internal typealias CABI = %
    internal typealias SwiftABI = %
    internal class Default : MakeComposedAbi {
        internal typealias SwiftProjection = %
        internal typealias CABI = %
        internal typealias SwiftABI = %.%
        internal static func from(abi: UnsafeMutableRawPointer?) -> SwiftProjection? {
            guard let abi = abi else { return nil }
            return .init(fromAbi: .init(abi))
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
                        w.write("%.IInspectable", w.support);
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

        std::vector<std::pair<std::string, const metadata_type*>> collection_type_aliases;
        bool needsCustomQueryInterfaceConformance = composable;
        bool baseHasCustomQueryInterfaceConformance = base_class ? base_class->is_composable() : false;
        std::vector<std::pair<std::string, const metadata_type*>> overridable_interfaces;
        for (auto&& [interface_name, info] : type.required_interfaces)
        {
            if (info.base && !info.exclusive && !interface_name.empty())
            {
                baseHasCustomQueryInterfaceConformance = true;
            }
            if (info.overridable && !info.base)
            {
                // overridable interfaces are still considered exclusive, so check here before
                // we skip this interface
                overridable_interfaces.push_back({ interface_name, info.type });
                // Don't need to set needsCustomQueryInterfaceConformance here, but doing so for
                // posterity
                needsCustomQueryInterfaceConformance = true;
            }
            // Filter out which interfaces we actually want to declare on the class.
            // We don't want to specify interfaces which come from the base class or which ones are exclusive
            if (info.base || info.exclusive || interface_name.empty()) continue;

            // TODO: WIN-274 Code generation for IIterable/IIterator
            // TODO: WIN-124 Code generation for IObservableVector and IObservableMap
            auto name = info.type->swift_full_name();
            if (name.starts_with("Windows.Foundation.Collections.IIterable")
                || name.starts_with("Windows.Foundation.Collections.IIterator")
                || name.starts_with("Windows.Foundation.Collections.IObservableVector")
                || name.starts_with("Windows.Foundation.Collections.IObservableMap"))
            {
                continue;
            }

            s();
            // if the class also implements an interface, then it will need to conform to the protocol.
            needsCustomQueryInterfaceConformance = true;

            // when deriving from collections we want to just derive from `IVector` and will use a typealias to set the Element (this is required by Swift)
            auto name_to_write = interface_name;
            if (is_winrt_generic_collection(info.type))
            {
                if (interface_name.starts_with("IVector"))
                {
                    collection_type_aliases.emplace_back(
                        std::make_pair("Element", info.generic_params[0]));
                }
                else if (interface_name.starts_with("IMap"))
                {
                    collection_type_aliases.emplace_back(
                        std::make_pair("Key", info.generic_params[0]));
                    collection_type_aliases.emplace_back(
                        std::make_pair("Value", info.generic_params[1]));
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

        for (auto&& collection_type_alias : collection_type_aliases)
        {
            w.write("public typealias % = %\n",
                collection_type_alias.first,
                bind<write_type>(*collection_type_alias.second, write_type_params::swift));
        }

        writer::generic_param_guard guard;

        if (default_interface)
        {
            auto [ns, name] = get_type_namespace_and_name(*default_interface);
            auto swiftAbi = w.write_temp("%.%", abi_namespace(ns), name);
            std::string defaultVal = "";
            if (is_winrt_generic_collection(default_interface))
            {
                auto generic_type = dynamic_cast<const generic_inst*>(default_interface);
                guard = w.push_generic_params(*generic_type);
                swiftAbi = w.write_temp("%", bind_type_abi(generic_type));
            }

            w.write(R"(private typealias SwiftABI = %
private typealias CABI = %
private var _default: SwiftABI!
% func _getABI<T>() -> UnsafeMutablePointer<T>? {
    if T.self == CABI.self {
        return RawPointer(_default)
    }   
    if T.self == %.IInspectable.self {
        return RawPointer(_default)
    }
    return %
}

% var thisPtr: %.IInspectable { _default }

)",
swiftAbi,
bind_type_mangled(default_interface),
base_class ?
composable ? "override open" :
    "override public" :
                composable ? "open" :
                "public",
                w.c_mod,
                base_class ? "super._getABI()" : "nil",
                base_class ?
                composable ? "override open" :
                "override public" :
                composable ? "open" :
                "public",
                w.support);
            write_default_constructor_declarations(w, type, *default_interface);

            // composable types will always need CustomQueryInterface conformance so that derived types can
            // override the queryInterface call
            if (needsCustomQueryInterfaceConformance)
            {
                auto modifier = composable ? "open" : "public";
                auto override = type.base_class ? "override " : "";
                // A WinRTClass needs CustomQueryInterface conformance when it derives from 1 or more interfaces,
                // otherwise it won't compile. At the end of the day, the winrt object it's holding onto will appropriately
                // respond to QueryInterface calls, so call into the default implementation.
                auto baseComposable = type.base_class && type.base_class->is_composable();
                //auto base_case = "queryInterfaceImpl(riid, ppvObj)";
                auto label = composable || baseComposable ? "unsealed" : "sealed";
                std::string base_case;
                if (base_class)
                {
                    base_case = "super.queryInterface(riid, ppvObj)";
                } 
                else
                {
                    base_case = w.write_temp("%.queryInterface(%: self, riid, ppvObj)", w.support, label);
                }
                std::vector<std::string> query_interface_lines;
                if (overridable_interfaces.empty())
                {
                    query_interface_lines.push_back(w.write_temp("    return %", base_case));
                }
                else
                {
                    query_interface_lines.reserve(overridable_interfaces.size() * 3 + 3);
                    query_interface_lines.push_back(w.write_temp("    switch riid.pointee {\n"));
                    for (auto& [name, info] : overridable_interfaces) {
                        query_interface_lines.push_back(w.write_temp("       case %.IID:\n", bind_wrapper_fullname(info)));
                        query_interface_lines.push_back(w.write_temp("            let wrapper = %(self)\n", bind_wrapper_fullname(info)));
                        query_interface_lines.push_back(w.write_temp("            return wrapper!.queryInterface(riid, ppvObj)\n"));
                    }
                    query_interface_lines.push_back(w.write_temp("        default: return %\n", base_case));
                    query_interface_lines.push_back(w.write_temp("    }"));

                }

                w.write(R"(%% func queryInterface(_ riid: REFIID, _ ppvObj: UnsafeMutablePointer<LPVOID?>?) -> HRESULT { 
%
}
)", override, modifier, query_interface_lines);
            }
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
                write_interface_impl_members(w, info, /* is_class: */ true);
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

    template <typename T>
    static void write_query_interface_case(writer& w, T const& type, metadata_type const& iface, bool cast)
    {
        w.write("if riid.pointee == %.IID {\n", bind_wrapper_fullname(iface));
        {
            auto indent = w.push_indent();

            std::string cast_expr;
            if (cast)
            {
                cast_expr = w.write_temp(" as? any %",
                    bind<write_swift_type_identifier>(iface)); // Do not include outer Optional<>
            }

            w.write("guard let instance = %.tryUnwrapFrom(raw: pUnk)% else { return E_NOINTERFACE }\n",
                bind_wrapper_name(type), cast_expr);
            w.write("guard let inner = %(instance) else { return E_INVALIDARG }\n",
                bind_wrapper_fullname(iface));
            w.write("let pThis = try! inner.toABI { $0 }\n");
            w.write("return pThis.pointee.lpVtbl.pointee.QueryInterface(pThis, riid, ppvObject)\n");
        };
        w.write("}\n");
    }

    template <typename T>
    static void write_iunknown_methods(writer& w, T const& type, std::vector<named_interface_info> const& interfaces, bool composed = false)
    {
        auto wrapper_name = w.write_temp("%", bind_wrapper_name(type));
        w.write(R"(QueryInterface: {
    guard let pUnk = $0, let riid = $1, let ppvObject = $2 else { return E_INVALIDARG }
    if riid.pointee == IUnknown.IID ||
          riid.pointee == IInspectable.IID || 
          riid.pointee == ISwiftImplemented.IID ||
          riid.pointee == IAgileObject.IID ||
          riid.pointee == %.IID { 
        _ = pUnk.pointee.lpVtbl.pointee.AddRef(pUnk)
        ppvObject.pointee = UnsafeMutableRawPointer(pUnk)
        return S_OK
    }

    %
},

)",
    wrapper_name,
    bind([&](writer & w) {
        // Delegates are implemented as simple closures in Swift, which can't implement CustomQueryInterface
        if (is_delegate(type))
        { 
            w.write("return failWith(err: E_NOINTERFACE)");
        }
        else
        {
            std::string cast_expr;
            constexpr bool isGeneric = std::is_same_v<T, generic_inst>;

            w.write(R"(guard let instance = %.tryUnwrapFrom(raw: pUnk)% else { return failWith(err: E_NOINTERFACE )}
    return instance.queryInterface(riid, ppvObject)
)", wrapper_name, cast_expr);
        }
    }));

        w.write(R"(AddRef: {
     guard let wrapper = %.fromRaw($0) else { return 1 }
     _ = wrapper.retain()
     return ULONG(_getRetainCount(wrapper.takeUnretainedValue()))
},

)", wrapper_name);

        w.write(R"(Release: {
    guard let wrapper = %.fromRaw($0) else { return 1 }
    return ULONG(_getRetainCount(wrapper.takeRetainedValue()))
},

)", wrapper_name);
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
                do_write_abi_val_assignment(w, param.type, std::string_view(param_name), return_param_name);
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
            func_call += w.write_temp(".%", get_swift_name(method));
            if (isGeneric)
            {
                auto genericInst = (const generic_inst&)type;
                if (is_winrt_ireference(genericInst.generic_type()))
                {
                    func_call.append(w.write_temp(" as! %", get_full_swift_type_name(w, genericInst.generic_params()[0])));
                }
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

        bool needs_try_catch = !is_get_or_put && !is_winrt_generic_collection(type) && !is_delegate(type);
        w.write("%: {\n", func_name);
        if (needs_try_catch) {
            w.m_indent += 1;
            w.write("do {\n");
        }
        {
            auto indent_guard = w.push_indent();
            w.write("guard let __unwrapped__instance = %.tryUnwrapFrom(raw: $0) else { return E_INVALIDARG }\n",
                bind_wrapper_name(type));
            write_consume_params(w, function);

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
            w.write("} catch { return failWith(err: E_FAIL) } \n");
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
