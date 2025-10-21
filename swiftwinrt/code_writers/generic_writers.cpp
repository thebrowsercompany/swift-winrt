#include "generic_writers.h"

#include "writer_helpers.h"
#include "delegate_writers.h"
#include "interface_writers.h"
#include "utility/swift_codegen_utils.h"

namespace swiftwinrt
{
    void write_guid_generic(writer& w, generic_inst const& type)
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

    void write_generic_declaration(writer& w, generic_inst const& type)
    {
        auto push_param_guard = w.push_generic_params(type);
        w.write("internal var %VTable: %Vtbl = .init(\n",
            type.mangled_name(),
            type.mangled_name());

        const bool is_delegate_instance = type.generic_type()->category() == category::delegate_type;
        {
            auto indent = w.push_indent();
            write_iunknown_methods(w, type);
            separator s{ w, ",\n\n" };

            if (!is_delegate_instance)
            {
                write_iinspectable_methods(w, type, type.required_interfaces);
                s();
            }

            for (auto&& method : type.functions)
            {
                s();
                write_vtable_method(w, method, type);
            }
        }

        w.write(R"(
)
)");

        if (is_winrt_ireference(type))
        {
            w.write("typealias % = ReferenceWrapperBase<%>\n",
                bind_wrapper_name(type),
                bind_bridge_fullname(type));
        }
        else
        {
            w.write("typealias % = InterfaceWrapperBase<%>\n",
                bind_wrapper_name(type),
                bind_bridge_fullname(type));
        }
    }

    void write_interface_generic(writer& w, generic_inst const& type)
    {
        write_generic_declaration(w, type);

        if (!is_winrt_ireference(type))
        {
            auto generic_params = w.push_generic_params(type);
            write_interface_abi_body(w, *type.generic_type(), type.functions);
        }
    }

    void write_ireference_init_extension(writer& w, generic_inst const& type)
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

    void write_generic_extension(writer& w, generic_inst const& inst)
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

    void write_generic_delegate_wrapper(writer& w, generic_inst const& generic)
    {
        write_delegate_wrapper(w, generic);
    }

    void write_generic_interface_implementation(writer& w, generic_inst const& type)
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

        for (const auto& [interface_name, iface_info] : type.required_interfaces)
        {
            if (!can_write(w, iface_info.type)) { continue; }

            write_interface_impl_members(w, iface_info, *type.generic_type());
        }

        w.write("public func queryInterface(_ iid: %.IID) -> IUnknownRef? { nil }\n", w.support);
        indent_guard.end();
        w.write("}\n\n");
    }

    void write_generic_implementation(writer& w, generic_inst const& type)
    {
        auto generics_guard = w.push_generic_params(type);
        if (is_delegate(type))
        {
            auto delegate_method = type.functions[0];
            write_delegate_implementation_body(w, type, delegate_method);
        }
        else if (!is_winrt_ireference(type))
        {
            write_generic_interface_implementation(w, type);
        }
    }
}

