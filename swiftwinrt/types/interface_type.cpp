#include "pch.h"

#include "types/interface_type.h"

#include "abi_writer.h"
#include "utility/metadata_helpers.h"
#include "types/class_type.h"
#include "types/delegate_type.h"
#include "types/generic_inst.h"
#include "types/interface_writer_helpers.h"

using namespace std::literals;

namespace swiftwinrt
{
    void interface_type::append_signature(sha1& hash) const
    {
        hash.append("{"sv);
        auto iid = type_iid(m_type);
        hash.append(std::string_view{ iid.data(), iid.size() - 1 });
        hash.append("}"sv);
    }

    void interface_type::write_c_forward_declaration(writer& w) const
    {
        if (!w.should_forward_declare(m_mangled_name))
        {
            return;
        }

        w.write(R"^-^(#ifndef __%_FWD_DEFINED__
#define __%_FWD_DEFINED__
    typedef interface % %;

#endif // __%_FWD_DEFINED__

)^-^",
            bind_mangled_name_macro(*this),
            bind_mangled_name_macro(*this),
            bind_c_type_name(*this),
            bind_c_type_name(*this),
            bind_mangled_name_macro(*this));
    }

    void interface_type::write_c_abi_param(writer& w) const
    {
        w.write("%*", bind_c_type_name(*this));
    }

    static void write_interface_definition(writer& w, interface_type const& type, void (*func)(writer&, interface_type const&))
    {
        if (type.is_generic())
        {
            return;
        }

        begin_type_definition(w, type);

        w.write(R"^-^(#if !defined(__%_INTERFACE_DEFINED__)
    #define __%_INTERFACE_DEFINED__
    )^-^",
            bind_mangled_name_macro(type),
            bind_mangled_name_macro(type));

        func(w, type);

        w.write(R"^-^(
    EXTERN_C const IID %;
#endif /* !defined(__%_INTERFACE_DEFINED__) */
    )^-^", bind_iid_name(type), bind_mangled_name_macro(type));

        end_type_definition(w, type);
    }

    void interface_type::write_c_definition(writer& w) const
    {
        write_interface_definition(w, *this, &write_c_interface_definition<interface_type>);
    }
}
