#include "writer_helpers.h"
#include "interface_writers.h"

namespace swiftwinrt
{
void write_guid(writer& w, typedef_base const& type)
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

void write_interface_abi(writer& w, interface_type const& type)
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

void write_make_from_abi(writer& w, metadata_type const& type)
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

void write_interface_bridge(writer& w, metadata_type const& type)
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

void write_interface_impl(writer& w, interface_type const& type)
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

void write_eventsource_invoke_extension(writer& w, metadata_type const* event_type)
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

void write_interface_proto(writer& w, interface_type const& type)
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

void write_interface_generic(writer& w, generic_inst const& type)
    {
        type.write_swift_declaration(w);

        if (!is_winrt_ireference(type))
        {
            auto generic_params = w.push_generic_params(type);
            do_write_interface_abi(w, *type.generic_type(), type.functions);
        }
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

void write_generic_implementation(writer& w, generic_inst const& type)
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
}
