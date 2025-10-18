#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "utility/sha1.h"
#include "utility/versioning.h"
#include "winmd_reader.h"

namespace swiftwinrt
{
    struct writer;

    std::string get_full_type_name(winmd::reader::TypeDef const& type);
    std::string_view remove_tick(std::string_view const& name);

    struct deprecation_info
    {
        std::string_view contract_type;
        std::uint32_t version;
        std::string_view message;
    };

    struct metadata_type
    {
        virtual ~metadata_type() = default;

        virtual std::string_view swift_full_name() const = 0;
        virtual std::string_view swift_type_name() const = 0;
        virtual std::string_view swift_abi_namespace() const = 0;
        virtual std::string_view swift_logical_namespace() const = 0;

        virtual std::string_view cpp_abi_name() const = 0;
        virtual std::string_view cpp_logical_name() const = 0;

        virtual std::string_view mangled_name() const = 0;
        virtual std::string_view generic_param_mangled_name() const = 0;

        virtual void append_signature(sha1& hash) const = 0;

        virtual void write_c_forward_declaration(writer& w) const = 0;
        virtual void write_c_abi_param(writer& w) const = 0;

        virtual void write_swift_declaration(writer& w) const = 0;

        virtual bool is_experimental() const = 0;

        virtual std::optional<std::size_t> contract_index(std::string_view /*typeName*/, std::size_t /*version*/) const
        {
            return std::nullopt;
        }

        virtual std::optional<contract_version> contract_from_index(std::size_t /*index*/) const
        {
            return std::nullopt;
        }
    };

    inline bool operator<(metadata_type const& lhs, metadata_type const& rhs) noexcept
    {
        return lhs.swift_full_name() < rhs.swift_full_name();
    }

    struct typename_comparator
    {
        bool operator()(std::string_view lhs, metadata_type const& rhs) const
        {
            return lhs < rhs.swift_full_name();
        }

        bool operator()(metadata_type const& lhs, std::string_view rhs) const
        {
            return lhs.swift_full_name() < rhs;
        }
    };

    using generic_param_type = metadata_type const*;
    using generic_param_vector = std::vector<generic_param_type>;
}
