#include "pch.h"

#include "types/generic_inst.h"

#include "file_writers/abi_writer.h"
#include "code_writers/common_writers.h"
#include "code_writers/writer_helpers.h"
#include "utility/metadata_helpers.h"
#include "types/class_type.h"
#include "types/delegate_type.h"
#include "types/interface_type.h"
#include "types/interface_writer_helpers.h"
#include "types/type_constants.h"
#include "types/type_traits.h"

using namespace std::literals;

namespace swiftwinrt
{
    generic_inst::generic_inst(typedef_base const* generic_type, std::vector<metadata_type const*> generic_params) :
        m_generic_type(generic_type),
        m_generic_params(std::move(generic_params))
    {
        m_swift_full_name = generic_type->swift_full_name();
        m_swift_full_name.push_back('<');

        m_mangled_name = generic_type->mangled_name();
        m_swift_type_name = generic_type->swift_type_name();
        m_swift_type_name.push_back('<');

        std::string_view prefix;
        for (auto param : m_generic_params)
        {
            m_swift_full_name += prefix;
            m_swift_full_name += param->swift_full_name();

            m_swift_type_name += prefix;
            m_swift_type_name += param->swift_full_name();

            m_mangled_name.push_back('_');
            m_mangled_name += param->generic_param_mangled_name();

            prefix = ", ";
        }

        m_swift_full_name.push_back('>');
        m_swift_type_name.push_back('>');
    }

    bool generic_inst::is_experimental() const
    {
        for (auto ptr : m_generic_params)
        {
            if (ptr->is_experimental())
            {
                return true;
            }
        }

        return false;
    }

    void generic_inst::append_signature(sha1& hash) const
    {
        hash.append("pinterface({"sv);
        auto iid = type_iid(m_generic_type->type());
        hash.append(std::string_view{ iid.data(), iid.size() - 1 });
        hash.append("}"sv);
        for (auto param : m_generic_params)
        {
            hash.append(";"sv);
            param->append_signature(hash);
        }
        hash.append(")"sv);
    }

    void generic_inst::write_c_forward_declaration(writer& w) const
    {
        if (!w.begin_declaration(m_mangled_name))
        {
            if (w.should_forward_declare(m_mangled_name))
            {
                w.write("typedef interface % %;\n\n", m_mangled_name, m_mangled_name);
            }

            return;
        }

        for (auto param : m_generic_params)
        {
            param->write_c_forward_declaration(w);
        }

        for (auto dep : dependencies)
        {
            dep->write_c_forward_declaration(w);
        }

        auto is_experimental_instance = is_experimental();
        if (is_experimental_instance)
        {
            w.write("#if defined(ENABLE_WINRT_EXPERIMENTAL_TYPES)\n");
        }

        w.write(R"^-^(#if !defined(__%_INTERFACE_DEFINED__)
    #define __%_INTERFACE_DEFINED__

    typedef interface % %;

    //  Declare the parameterized interface IID.
    EXTERN_C const IID IID_%;

    )^-^", m_mangled_name, m_mangled_name, m_mangled_name, m_mangled_name, m_mangled_name);

        write_c_interface_definition(w, *this);

        w.write(R"^-^(
    #endif // __%_INTERFACE_DEFINED__
    )^-^", m_mangled_name);

        if (is_experimental_instance)
        {
            w.write("#endif // defined(ENABLE_WINRT_EXPERIMENTAL_TYPES)\n");
        }

        w.write('\n');
        w.end_declaration(m_mangled_name);
    }

    void generic_inst::write_c_abi_param(writer& w) const
    {
        w.write("%*", m_mangled_name);
    }

    std::string_view generic_inst::generic_type_abi_name() const noexcept
    {
        auto result = m_generic_type->cpp_abi_name();
        auto tick_pos = result.rfind('`');
        XLANG_ASSERT(tick_pos != std::string_view::npos);
        return result.substr(0, tick_pos);
    }
}
