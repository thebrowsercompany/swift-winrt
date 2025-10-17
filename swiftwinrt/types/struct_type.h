#pragma once

#include <vector>

#include "types/struct_member.h"
#include "types/typedef_base.h"

namespace swiftwinrt
{
    struct struct_type final : typedef_base
    {
        explicit struct_type(winmd::reader::TypeDef const& type) :
            typedef_base(type)
        {
        }

        void append_signature(sha1& hash) const override
        {
            using namespace std::literals;
            XLANG_ASSERT(members.size() == static_cast<std::size_t>(distance(m_type.FieldList())));
            hash.append("struct("sv);
            hash.append(m_swift_full_name);
            for (auto const& member : members)
            {
                hash.append(";");
                member.type->append_signature(hash);
            }
            hash.append(")"sv);
        }

        void write_c_forward_declaration(writer& w) const override;
        void write_c_abi_param(writer& w) const override;
        void write_c_definition(writer& w) const;

        std::vector<struct_member> members;
    };
}
