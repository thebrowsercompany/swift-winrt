#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "attributes.h"
#include "types/generic_type_parameter.h"
#include "types/metadata_type.h"
#include "utility/type_helpers.h"
#include "versioning.h"

namespace swiftwinrt
{
    struct typedef_base : metadata_type
    {
        explicit typedef_base(winmd::reader::TypeDef const& type);

        std::string_view swift_abi_namespace() const override
        {
            return m_type.TypeNamespace();
        }

        std::string_view swift_logical_namespace() const override
        {
            return m_type.TypeNamespace();
        }

        std::string_view swift_full_name() const override
        {
            return m_swift_full_name;
        }

        std::string_view cpp_abi_name() const override
        {
            return m_type.TypeName();
        }

        std::string_view swift_type_name() const override
        {
            return m_type.TypeName();
        }

        std::string_view cpp_logical_name() const override
        {
            return m_type.TypeName();
        }

        std::string_view mangled_name() const override
        {
            return m_mangled_name;
        }

        std::string_view generic_param_mangled_name() const override;

        bool is_experimental() const override;

        std::optional<std::size_t> contract_index(std::string_view type_name, std::size_t version) const override;
        std::optional<contract_version> contract_from_index(std::size_t index) const override;

        winmd::reader::TypeDef const& type() const noexcept
        {
            return m_type;
        }

        bool is_generic() const noexcept;
        std::optional<deprecation_info> is_deprecated() const noexcept;

        winmd::reader::category category() const noexcept
        {
            return get_category(m_type);
        }

        void write_swift_declaration(writer&) const override
        {
        }

        std::vector<generic_type_parameter> generic_params;

    protected:
        winmd::reader::TypeDef m_type;
        std::string m_swift_full_name;
        std::string m_mangled_name;
        std::string m_generic_param_mangled_name;
        std::vector<platform_version> m_platform_versions;
        std::optional<contract_history> m_contract_history;
    };
}
