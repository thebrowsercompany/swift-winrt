#pragma once

#include <string>
#include <vector>

#include "types/function_def.h"
#include "types/typedef_base.h"

namespace swiftwinrt
{
    struct delegate_type final : typedef_base
    {
        explicit delegate_type(winmd::reader::TypeDef const& type);

        std::string_view cpp_logical_name() const override
        {
            return m_abi_name;
        }

        void append_signature(sha1& hash) const override;
        void write_c_forward_declaration(writer& w) const override;
        void write_c_abi_param(writer& w) const override;
        void write_c_definition(writer& w) const;

        std::vector<function_def> functions;

    private:
        std::string m_abi_name;
    };
}
