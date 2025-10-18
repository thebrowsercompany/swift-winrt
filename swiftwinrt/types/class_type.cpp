#include "pch.h"

#include "types/class_type.h"

#include "abi_writer.h"
#include "utility/metadata_helpers.h"
#include "utility/swift_codegen_utils.h"
#include "types/interface_type.h"

namespace swiftwinrt
{
    class_type::class_type(winmd::reader::TypeDef const& type) :
        typedef_base(type)
    {
        using namespace winmd::reader;
        if (auto default_iface = get_default_interface(type))
        {
            auto [ns, _] = type_name::get_namespace_and_name(default_iface);
            m_abi_namespace = ns;
        }
    }

    std::string_view class_type::swift_abi_namespace() const
    {
        if (m_abi_namespace.empty())
        {
            swiftwinrt::throw_invalid("Class type '", swift_full_name(), "' does not have a default interface and therefore "
                "does not have an ABI type namespace");
        }

        return m_abi_namespace;
    }

    std::string_view class_type::cpp_abi_name() const
    {
        if (!default_interface)
        {
            swiftwinrt::throw_invalid("Class type '", swift_full_name(), "' does not have a default interface and therefore "
                "does not have an ABI type name");
        }

        return default_interface->cpp_abi_name();
    }

    void class_type::append_signature(sha1& hash) const
    {
        using namespace std::literals;
        if (!default_interface)
        {
            swiftwinrt::throw_invalid("Class type '", swift_full_name(), "' does not have a default interface and therefore "
                "does not have a signature");
        }

        hash.append("rc("sv);
        hash.append(m_swift_full_name);
        hash.append(";"sv);
        default_interface->append_signature(hash);
        hash.append(")"sv);
    }

    void class_type::write_c_forward_declaration(writer& w) const
    {
        if (!default_interface)
        {
            swiftwinrt::throw_invalid("Cannot forward declare class '", m_swift_full_name, "' since it has no default interface");
        }

        default_interface->write_c_forward_declaration(w);
    }

    void class_type::write_c_abi_param(writer& w) const
    {
        if (!default_interface)
        {
            swiftwinrt::throw_invalid("Class '", m_swift_full_name, "' cannot be used as a function argument since it has no "
                "default interface");
        }

        default_interface->write_c_abi_param(w);
    }

    bool class_type::is_composable() const
    {
        return swiftwinrt::is_composable(type());
    }
}
