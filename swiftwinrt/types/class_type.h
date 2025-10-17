#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "types/attributed_type.h"
#include "types/interface_info.h"
#include "types/typedef_base.h"
#include "versioning.h"

namespace swiftwinrt
{
    struct interface_type;

    struct class_type final : typedef_base
    {
        explicit class_type(winmd::reader::TypeDef const& type);

        std::string_view swift_abi_namespace() const override;
        std::string_view cpp_abi_name() const override;
        void append_signature(sha1& hash) const override;
        void write_c_forward_declaration(writer& w) const override;
        void write_c_abi_param(writer& w) const override;
        void write_c_definition(writer&) const {}

        std::vector<named_interface_info> required_interfaces;
        std::vector<std::pair<interface_type const*, version>> supplemental_fast_interfaces;
        class_type const* base_class{ nullptr };
        metadata_type const* default_interface{ nullptr };
        std::map<std::string, attributed_type> factories;

        bool is_composable() const;

    private:
        std::string_view m_abi_namespace;
    };
}
