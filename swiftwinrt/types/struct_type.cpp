#include "pch.h"

#include "types/struct_type.h"

#include "file_writers/abi_writer.h"
#include "types/type_definition_helpers.h"

namespace swiftwinrt
{
    void struct_type::write_c_forward_declaration(writer& w) const
    {
        if (!w.should_forward_declare(m_mangled_name))
        {
            return;
        }

        w.write("typedef struct % %;\n\n", bind_c_type_name(*this), bind_c_type_name(*this));
    }

    void struct_type::write_c_abi_param(writer& w) const
    {
        w.write("struct %", bind_c_type_name(*this));
    }

    void struct_type::write_c_definition(writer& w) const
    {
        begin_type_definition(w, *this);

        w.write(R"^-^(struct %
    {
)^-^", bind_c_type_name(*this));

        for (auto const& member : members)
        {
            w.write("    % %;\n", [&](writer& inner) { member.type->write_c_abi_param(inner); }, member.field.Name());
        }

        w.write("};\n");

        end_type_definition(w, *this);
    }
}
