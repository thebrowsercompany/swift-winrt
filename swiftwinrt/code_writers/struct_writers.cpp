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

                w.write("%: %", get_swift_name(field), field.type);
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

    void write_struct(writer & w, struct_type const& type)
    {
        w.write("public struct % : Equatable {\n", type);
        {
            auto indent_guard1 = w.push_indent();
            for (auto&& field : type.members)
            {
                auto field_type = field.type;
                // WIN-64 - swiftwinrt: support boxing/unboxing
                // WIN-65 - swiftwinrt: support generic types
                if (!can_write(w, field_type)) continue;

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

            w.write(R"(public static func from(abi: %) -> % {
        .init(%)
    }
    )", bind_type_mangled(type),
    type,
    bind([&](writer& w) {
                    separator s{ w };
            for (auto&& field : type.members)
            {
                if (can_write(w, field.type))
                {
                    s();
                    std::string from = std::string("abi.").append(get_abi_name(field));
                    w.write("%: %",
                        get_swift_name(field),
                        bind<write_consume_type>(field.type, from)
                    );
                }
            }
        }));

        }
        w.write("}\n\n");
    }
}