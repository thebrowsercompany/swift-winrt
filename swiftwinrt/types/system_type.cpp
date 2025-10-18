#include "pch.h"

#include "types/system_type.h"

#include "abi_writer.h"
#include "helpers.h"

using namespace std::literals;

namespace swiftwinrt
{
    system_type::system_type(
        std::string_view swift_module,
        std::string_view swift_type_name,
        std::string_view swift_full_name,
        std::string_view cpp_name,
        std::string_view signature,
        param_category category) :
        m_swift_module_name(swift_module),
        m_swift_type_name(swift_type_name),
        m_swift_full_name(swift_full_name),
        m_cpp_name(cpp_name),
        m_signature(signature),
        m_category(category)
    {
    }

    system_type const& system_type::from_name(std::string_view type_name)
    {
        if (type_name == "Guid"sv)
        {
            static system_type const guid_type{ "Foundation"sv, "UUID"sv, "Foundation.UUID"sv, "GUID"sv, "g16"sv, param_category::guid_type };
            return guid_type;
        }
        else if (type_name == "IBufferByteAccess")
        {
            static system_type const ibufferbyteaccess_type{ ""sv, type_name, type_name, "C_IBufferByteAccess"sv, "{905a0fef-bc53-11df-8c49-001e4fc686da}"sv, param_category::object_type };
            return ibufferbyteaccess_type;
        }
        else if (type_name == "IMemoryBufferByteAccess")
        {
            static system_type const imemorybufferbyte_type{ ""sv, type_name, type_name, "IMemoryBufferByteAccess"sv, "{5b0d3235-4dba-4d44-865e-8f1d0e4fd04d}"sv, param_category::object_type };
            return imemorybufferbyte_type;
        }

        XLANG_ASSERT(false);
        swiftwinrt::throw_invalid("Unknown type '", type_name, "' in System namespace");
    }

    void system_type::write_c_abi_param(writer& w) const
    {
        w.write(m_cpp_name);
    }
}
