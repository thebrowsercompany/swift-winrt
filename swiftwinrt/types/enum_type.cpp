#include "pch.h"

#include "types/enum_type.h"

#include "abi_writer.h"
#include "helpers.h"
#include "types/type_definition_helpers.h"

using namespace winmd::reader;

namespace swiftwinrt
{
    void enum_type::write_c_forward_declaration(writer& w) const
    {
        if (!w.should_forward_declare(m_mangled_name))
        {
            return;
        }

        w.write("typedef enum % %;\n\n", bind_c_type_name(*this), bind_c_type_name(*this));
    }

    void enum_type::write_c_abi_param(writer& w) const
    {
        w.write("enum %", bind_c_type_name(*this));
    }

    void enum_type::write_c_definition(writer& w) const
    {
        begin_type_definition(w, *this);

        w.write(R"^-^(enum %
    {
    )^-^", bind_c_type_name(*this));

        for (auto const& field : m_type.FieldList())
        {
            if (auto value = field.Constant())
            {
                auto is_experimental_field = swiftwinrt::is_experimental(field);
                if (is_experimental_field)
                {
                    w.write("#if defined(ENABLE_WINRT_EXPERIMENTAL_TYPES)\n");
                }

                w.write("    %_%", mangled_name(), field.Name());
                w.write(" = %,\n", value);

                if (is_experimental_field)
                {
                    w.write("#endif // defined(ENABLE_WINRT_EXPERIMENTAL_TYPES)\n");
                }
            }
        }

        w.write("};\n");
        end_type_definition(w, *this);
    }

    ElementType enum_type::underlying_type() const
    {
        return underlying_enum_type(m_type);
    }
}
