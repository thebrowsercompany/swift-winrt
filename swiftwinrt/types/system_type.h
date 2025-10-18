#pragma once

#include <string_view>

#include "types/metadata_type.h"
#include "utility/type_helpers.h"

namespace swiftwinrt
{
    struct system_type final : metadata_type
    {
        system_type(
            std::string_view swift_module,
            std::string_view swift_type_name,
            std::string_view swift_full_name,
            std::string_view cpp_name,
            std::string_view signature,
            param_category category);

        static system_type const& from_name(std::string_view type_name);

        std::string_view swift_abi_namespace() const override
        {
            return {};
        }

        std::string_view swift_logical_namespace() const override
        {
            return m_swift_module_name;
        }

        std::string_view swift_full_name() const override
        {
            return m_swift_full_name;
        }

        std::string_view swift_type_name() const override
        {
            return m_swift_type_name;
        }

        std::string_view cpp_abi_name() const override
        {
            return m_cpp_name;
        }

        std::string_view cpp_logical_name() const override
        {
            return m_cpp_name;
        }

        std::string_view mangled_name() const override
        {
            return m_cpp_name;
        }

        std::string_view generic_param_mangled_name() const override
        {
            return m_cpp_name;
        }

        void append_signature(sha1& hash) const override
        {
            hash.append(m_signature);
        }

        void write_c_forward_declaration(writer&) const override
        {
        }

        void write_c_abi_param(writer& w) const override;

        void write_swift_declaration(writer&) const override
        {
        }

        bool is_experimental() const override
        {
            return false;
        }

        param_category category() const
        {
            return m_category;
        }

    private:
        std::string_view m_swift_module_name;
        std::string_view m_swift_type_name;
        std::string_view m_swift_full_name;
        std::string_view m_cpp_name;
        std::string_view m_signature;
        param_category m_category;
    };
}
