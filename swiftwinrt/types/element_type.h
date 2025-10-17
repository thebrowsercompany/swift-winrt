#pragma once

#include <string_view>

#include "types/metadata_type.h"

namespace swiftwinrt
{
    struct element_type final : metadata_type
    {
        element_type(
            winmd::reader::ElementType element_type,
            std::string_view swift_name,
            std::string_view logical_name,
            std::string_view abi_name,
            std::string_view cpp_name,
            std::string_view mangled_name,
            std::string_view signature);

        static element_type const& from_type(winmd::reader::ElementType type);

        std::string_view swift_abi_namespace() const override
        {
            return {};
        }

        std::string_view swift_logical_namespace() const override
        {
            return {};
        }

        std::string_view swift_full_name() const override
        {
            return m_swift_name;
        }

        std::string_view swift_type_name() const override
        {
            return m_swift_name;
        }

        std::string_view cpp_abi_name() const override
        {
            return m_cpp_name;
        }

        std::string_view cpp_logical_name() const override
        {
            return m_logical_name;
        }

        std::string_view mangled_name() const override
        {
            return m_mangled_name;
        }

        std::string_view generic_param_mangled_name() const override
        {
            return m_mangled_name;
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

        bool is_blittable() const;
        winmd::reader::ElementType type() const
        {
            return m_type;
        }

    private:
        std::string_view m_swift_name;
        std::string_view m_logical_name;
        std::string_view m_abi_name;
        std::string_view m_cpp_name;
        std::string_view m_mangled_name;
        std::string_view m_signature;
        winmd::reader::ElementType m_type;
    };
}
