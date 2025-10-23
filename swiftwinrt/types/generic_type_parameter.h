#pragma once

#include <string_view>

#include "types/metadata_type.h"

namespace swiftwinrt
{
    struct generic_type_parameter final : metadata_type
    {
        explicit generic_type_parameter(std::string_view name) :
            m_param_name(name)
        {
        }

        std::string_view swift_full_name() const override
        {
            return m_param_name;
        }

        std::string_view swift_type_name() const override
        {
            return m_param_name;
        }

        std::string_view swift_abi_namespace() const override
        {
            return {};
        }

        std::string_view swift_logical_namespace() const override
        {
            return {};
        }

        std::string_view cpp_abi_name() const override
        {
            return m_param_name;
        }

        std::string_view cpp_logical_name() const override
        {
            return m_param_name;
        }

        std::string_view mangled_name() const override
        {
            return m_param_name;
        }

        std::string_view generic_param_mangled_name() const override
        {
            return m_param_name;
        }

        void append_signature(sha1&) const override
        {
        }

        void write_c_forward_declaration(writer&) const override
        {
        }

        void write_c_abi_param(writer&) const override
        {
        }

        bool is_experimental() const override
        {
            return false;
        }

        void write_swift_declaration(writer&) const override
        {
        }

    private:
        std::string_view m_param_name;
    };
}
