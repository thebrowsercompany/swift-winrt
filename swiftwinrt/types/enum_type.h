#pragma once

#include "types/element_type.h"
#include "types/typedef_base.h"

namespace swiftwinrt
{
    struct enum_type final : typedef_base
    {
        explicit enum_type(winmd::reader::TypeDef const& type) :
            typedef_base(type)
        {
        }

        void append_signature(sha1& hash) const override
        {
            using namespace std::literals;
            hash.append("enum("sv);
            hash.append(m_swift_full_name);
            hash.append(";"sv);
            element_type::from_type(underlying_type()).append_signature(hash);
            hash.append(")"sv);
        }

        void write_c_forward_declaration(writer& w) const override;
        void write_c_abi_param(writer& w) const override;
        void write_c_definition(writer& w) const;
        winmd::reader::ElementType underlying_type() const;
    };
}
