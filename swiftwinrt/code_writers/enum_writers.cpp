#include "utility/metadata_cache.h"
#include "common_writers.h"
#include "utility/type_writers.h"
#include "can_write.h"
#include "enum_writers.h"
#include "utility/metadata_helpers.h"
#include "utility/swift_codegen_utils.h"

namespace swiftwinrt
{
    void write_enum_def(writer& w, enum_type const& type)
    {
        if (type.swift_logical_namespace() == "Windows.Foundation.Metadata" || !can_write(w, type))
        {
            return;
        }

        write_documentation_comment(w, type);
        w.write("public typealias % = %\n", type, bind_type_mangled(type));
    }

    void write_enum_extension(writer& w, enum_type const& type)
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
                    if (type.swift_full_name() == "Windows.Foundation.Collections.CollectionChange")
                    {
                        w.write(format, get_swift_name(field), get_full_swift_type_name(w, type), type, field.Name());
                    }
                    else
                    {
                        w.write(format, get_swift_name(field), get_full_swift_type_name(w, type), bind_type_mangled(type), field.Name());
                    }
                }
            }
        }
        w.write("}\n");

        w.write("extension %: ^@retroactive Hashable, ^@retroactive Codable, ^@retroactive ^@unchecked Sendable {}\n\n", get_full_swift_type_name(w, type));
    }
}
