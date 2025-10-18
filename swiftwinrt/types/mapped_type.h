#pragma once

#include <optional>
#include <string_view>

#include "types/metadata_type.h"

namespace swiftwinrt
{
    struct mapped_type final : metadata_type
    {
        mapped_type(
            winmd::reader::TypeDef const& type,
            std::string_view cpp_name,
            std::string_view mangled_name,
            std::string_view signature);

        static mapped_type const* from_typedef(winmd::reader::TypeDef const& type);

        std::string_view swift_abi_namespace() const override
        {
            return m_type.TypeNamespace();
        }

        std::string_view swift_logical_namespace() const override
        {
            return {};
        }

        std::string_view swift_full_name() const override
        {
            return m_swift_full_name;
        }

        std::string_view swift_type_name() const override
        {
            return m_type.TypeName();
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

        std::optional<std::size_t> contract_index(std::string_view type_name, std::size_t version) const override;
        std::optional<contract_version> contract_from_index(std::size_t index) const override;

        winmd::reader::TypeDef type() const
        {
            return m_type;
        }

    private:
        winmd::reader::TypeDef m_type;
        std::string m_swift_full_name;
        std::string_view m_cpp_name;
        std::string_view m_mangled_name;
        std::string_view m_signature;
    };
}
