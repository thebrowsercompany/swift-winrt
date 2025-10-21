#include "writer_helpers.h"
#include "delegate_writers.h"
#include "interface_writers.h"
#include "utility/swift_codegen_utils.h"

namespace swiftwinrt
{
    void write_delegate(writer& w, delegate_type const& type)
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

    void write_delegate_implementation(writer& w, delegate_type const& type)
    {
        if (can_write(w, type) && !type.is_generic())
        {
            auto delegate_method = type.functions[0];
            do_write_delegate_implementation(w, type, delegate_method);
        }
    }

    void write_delegate_abi(writer& w, delegate_type const& type)
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

    void do_write_delegate_implementation(writer& w, metadata_type const& type, function_def const& invoke_method)
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
        const bool is_generic = is_generic_inst(type);
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

    void write_delegate_extension(writer& w, metadata_type const& inst, function_def const& invoke_method)
    {
        auto delegate_abi_name = w.write_temp("%", bind_type_mangled(inst));

        const bool is_generic = is_generic_inst(inst);

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

    void write_delegate_wrapper(writer& w, metadata_type const& type)
    {
        auto impl_name = w.write_temp("%", bind_bridge_fullname(type));
        auto wrapper_name = w.write_temp("%", bind_wrapper_name(type));
        auto format = R"(
typealias % = InterfaceWrapperBase<%>
)";
        w.write(format, wrapper_name, impl_name);
    }

    void write_vtable(writer& w, delegate_type const& type)
    {
        do_write_vtable(w, type, {});
    }

    void write_delegate_return_type(writer& w, function_def const& sig)
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
}
