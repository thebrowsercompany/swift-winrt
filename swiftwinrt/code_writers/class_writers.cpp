#include "writer_helpers.h"
#include "class_writers.h"

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
}
