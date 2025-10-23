#include "writer_helpers.h"
#include "class_writers.h"
#include "interface_writers.h"
#include "utility/type_helpers.h"
#include "utility/swift_codegen_utils.h"
namespace swiftwinrt
{
    void write_class(writer& w, class_type const& type)
    {
        write_class_impl(w, type);
    }

    void write_class_impl(writer& w, class_type const& type)
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

    void write_class_bridge(writer& w, class_type const& type)
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

    void write_class_abi(writer& w, class_type const& type)
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

    void write_composable_impl_extension(writer& w, class_type const& overridable)
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

    void write_overrides_vtable(writer& w, class_type const& type, interface_info const& overrides, std::vector<named_interface_info> const& other_interfaces)
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

    void write_factory_constructors(writer& w, attributed_type const& factory, class_type const& type, metadata_type const& default_interface)
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

    void write_composable_constructor(writer& w, attributed_type const& factory, class_type const& type)
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

    void write_default_constructor_declarations(writer& w, class_type const& type, metadata_type const& default_interface)
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

    void write_statics_body(writer& w, function_def const& method, metadata_type const& statics)
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

    void write_static_members(writer& w, attributed_type const& statics, class_type const& type)
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

    void write_composable_impl(writer& w, class_type const& parent, metadata_type const& overrides, bool compose)
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
}
