#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "types/event_def.h"
#include "types/function_def.h"
#include "types/interface_info.h"
#include "types/property_def.h"
#include "types/typedef_base.h"

namespace swiftwinrt
{
    struct generic_inst final : metadata_type
    {
        generic_inst(typedef_base const* generic_type, std::vector<metadata_type const*> generic_params);

        std::string_view swift_abi_namespace() const override
        {
            return m_generic_type->swift_abi_namespace();
        }

        std::string_view swift_logical_namespace() const override
        {
            return m_generic_type->swift_logical_namespace();
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
            return m_mangled_name;
        }

        std::string_view cpp_logical_name() const override
        {
            return m_mangled_name;
        }

        std::string_view mangled_name() const override
        {
            return m_mangled_name;
        }

        std::string_view generic_param_mangled_name() const override
        {
            return m_mangled_name;
        }

        void append_signature(sha1& hash) const override;
        void write_c_forward_declaration(writer& w) const override;
        void write_c_abi_param(writer& w) const override;
        bool is_experimental() const override;

        typedef_base const* generic_type() const noexcept
        {
            return m_generic_type;
        }

        winmd::reader::category category() const noexcept
        {
            return m_generic_type->category();
        }

        std::optional<deprecation_info> is_deprecated() const noexcept
        {
            return m_generic_type->is_deprecated();
        }

        std::string_view generic_type_abi_name() const noexcept;

        std::vector<metadata_type const*> const& generic_params() const noexcept
        {
            return m_generic_params;
        }

        std::vector<generic_inst const*> dependencies;
        std::vector<function_def> functions;
        std::vector<property_def> properties;
        std::vector<event_def> events;
        std::vector<named_interface_info> required_interfaces;

    private:
        typedef_base const* m_generic_type;
        std::vector<metadata_type const*> m_generic_params;
        std::string m_swift_full_name;
        std::string m_swift_type_name;
        std::string m_mangled_name;
    };
}
