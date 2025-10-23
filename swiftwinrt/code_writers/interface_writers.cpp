#include "writer_helpers.h"
#include "interface_writers.h"
#include "utility/swift_codegen_utils.h"

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

    void write_interface_bridge(writer& w, metadata_type const& type) {
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

    void do_write_interface_abi(writer& w, typedef_base const& type, std::vector<function_def> const& methods)
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

    void write_implementable_interface(writer& w, interface_type const& type)
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

    void write_vtable(writer& w, interface_type const& type)
    {
        do_write_vtable(w, type, type.required_interfaces);
    }

    void write_class_impl_property(writer& w, property_def const& prop, interface_info const& iface, typedef_base const& type_definition)
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

    void write_class_impl_func(writer& w, function_def const& function, interface_info const& iface, typedef_base const& type_definition)
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

    void write_class_impl_event(writer& w, event_def const& def, interface_info const& iface, typedef_base const& type_definition)
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

    void write_interface_impl_members(writer& w, interface_info const& info, typedef_base const& type_definition)
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
    
    void write_class_func_body(writer& w, function_def const& function, interface_info const& iface, bool is_noexcept)
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
}
