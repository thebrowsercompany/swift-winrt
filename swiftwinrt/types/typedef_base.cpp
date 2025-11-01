#include "pch.h"

#include "types/typedef_base.h"

#include "utility/metadata_helpers.h"
#include "utility/type_helpers.h"
#include "utility/type_writers.h"
#include "utility/versioning.h"

using namespace std::literals;
using namespace winmd::reader;

namespace swiftwinrt
{
    typedef_base::typedef_base(TypeDef const& type) :
        m_type(type),
        m_swift_full_name(get_full_type_name(type)),
        m_mangled_name(swiftwinrt::mangled_name<false>(type)),
        m_generic_param_mangled_name(swiftwinrt::mangled_name<true>(type)),
        m_contract_history(get_contract_history(type))
    {
        for_each_attribute(type, metadata_namespace, "VersionAttribute"sv, [&](bool, CustomAttribute const& attr)
        {
            m_platform_versions.push_back(decode_platform_version(attr));
        });
    }

    std::string_view typedef_base::generic_param_mangled_name() const
    {
        XLANG_ASSERT(!is_generic());
        return m_generic_param_mangled_name;
    }

    bool typedef_base::is_experimental() const
    {
        return swiftwinrt::is_experimental(m_type);
    }

    std::optional<deprecation_info> typedef_base::is_deprecated() const noexcept
    {
        return swiftwinrt::is_deprecated(m_type);
    }

    bool typedef_base::is_generic() const noexcept
    {
        return swiftwinrt::is_generic(m_type);
    }

    std::optional<std::size_t> typedef_base::contract_index(std::string_view type_name, std::size_t version) const
    {
        if (!m_contract_history)
        {
            return std::nullopt;
        }

        std::size_t result = 0;
        for (auto& prev : m_contract_history->previous_contracts)
        {
            if ((prev.contract_from == type_name) && (prev.version_low <= version) && (prev.version_high > version))
            {
                return result;
            }

            ++result;
        }

        if ((m_contract_history->current_contract.name == type_name) && (m_contract_history->current_contract.version <= version))
        {
            return result;
        }

        return std::nullopt;
    }

    std::optional<contract_version> typedef_base::contract_from_index(std::size_t index) const
    {
        if (!m_contract_history)
        {
            return std::nullopt;
        }

        for (auto& prev : m_contract_history->previous_contracts)
        {
            if (index-- == 0)
            {
                return contract_version{ prev.contract_from, prev.version_low };
            }
        }

        if (index == 0)
        {
            return m_contract_history->current_contract;
        }

        XLANG_ASSERT(false);
        return std::nullopt;
    }
}
