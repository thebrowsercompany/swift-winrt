#include "metadata_cache.h"
#include "common_writers.h"
#include "type_writers.h"
#include "types.h"
#include "common.h"
#include "can_write.h"
#include "struct_writers.h"
namespace swiftwinrt
{
    void write_struct_initializer_params(writer & w, struct_type const& type)
    {
        separator s{ w };

        for (auto&& field : type.members)
        {
            // WIN-64 - swiftwinrt: support boxing/unboxing
            // WIN-65 - swiftwinrt: support generic types
            if (can_write(w, field.type))
            {
                s();

                w.write("%: %", get_swift_name(field), bind<write_type>(*field.type, write_type_params::swift));
            }
        }
    }

    void write_struct_init_extension(writer & w, struct_type const& type)
    {
        bool is_blittable = is_struct_blittable(type);
        if (is_blittable)
        {
            w.write(R"(extension % {
        public static func from(swift: %) -> % {
            .init(%)
        }
    }
    )", bind_type_mangled(type), get_full_swift_type_name(w, type), bind_type_mangled(type), bind([&](writer& w) {
                    separator s{ w };
            for (auto&& field : type.members)
            {
                s();

                if (dynamic_cast<const struct_type*>(field.type))
                {
                    w.write("%: .from(swift: swift.%)",
                        get_abi_name(field),
                        get_swift_name(field)
                    );
                }
                else
                {
                    w.write("%: swift.%",
                        get_abi_name(field),
                        get_swift_name(field)
                    );
                }
            }
        }));
        }
    }

    void write_struct(writer& w, struct_type const& type)
    {
        write_documentation_comment(w, type);
        w.write("public struct %: Hashable, Codable, Sendable {\n", type);
        {
            auto indent_guard1 = w.push_indent();
            for (auto&& field : type.members)
            {
                auto field_type = field.type;
                // WIN-64 - swiftwinrt: support boxing/unboxing
                // WIN-65 - swiftwinrt: support generic types
                if (!can_write(w, field_type)) continue;

                write_documentation_comment(w, type, field.field.Name());
                w.write("public var %: %%\n",
                    get_swift_name(field),
                    bind<write_type>(*field_type, write_type_params::swift_allow_implicit_unwrap),
                    bind<write_default_init_assignment>(*field_type, projection_layer::swift));
            }

            w.write("public init() {}\n");
            w.write("public init(%) {\n", bind<write_struct_initializer_params>(type));
            {
                auto indent_guard2 = w.push_indent();
                for (auto&& field : type.members)
                {
                    // WIN-64 - swiftwinrt: support boxing/unboxing
                    // WIN-65 - swiftwinrt: support generic types
                    if (can_write(w, field.type))
                    {
                        auto field_name = get_swift_name(field);
                        w.write("self.% = %\n", field_name, field_name);
                    }
                }
            }
            w.write("}\n");
        }
        w.write("}\n\n");
    }

    void write_struct_bridgeable(writer& w, struct_type const& type)
    {
        w.write("@_spi(WinRTInternal)\n");
        w.write("extension %: WinRTBridgeable {\n", type);
        {
            auto indent_guard1 = w.push_indent();
            w.write("public typealias ABI = %\n", bind_type_mangled(type));
            w.write("public static func from(abi: ABI) -> Self {\n");
            {
                auto from_body_indent = w.push_indent();

                w.write(".init(");
                separator s{ w };
                for (auto&& field : type.members)
                {
                    if (can_write(w, field.type))
                    {
                        s();
                        std::string from = std::string("abi.").append(get_abi_name(field));
                        w.write("%: %",
                            get_swift_name(field),
                            bind<write_consume_type>(field.type, from, false)
                        );
                    }
                }
                w.write(")\n");
            }
            w.write("}\n");

            w.write("public func toABI() -> ABI {\n");
            {
                auto from_body_indent = w.push_indent();
                if (is_struct_blittable(type))
                {
                    w.write(".from(swift: self)\n");
                }
                else
                {
                     w.write("%._ABI_%(from: self).detach()\n", abi_namespace(type), type.swift_type_name());
                }
            }
            w.write("}\n");
        }
        w.write("}\n\n");
    }

    void write_struct_abi(writer& w, struct_type const& type)
    {
        bool is_blittable = is_struct_blittable(type);
        if (is_blittable)
        {
            return;
        }

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
}