#pragma once

#include <string>
#include <string_view>

#include "winmd_reader.h"
#include "types/metadata_type.h"

namespace swiftwinrt
{
    struct writer;

    void write_preamble(writer& w, bool swift_code);

    std::string get_swift_module(std::string_view ns);
    std::string get_swift_module(winmd::reader::TypeDef const& type);

    std::string get_swift_namespace(writer const& w, std::string_view ns);
    std::string get_swift_namespace(writer const& w, winmd::reader::TypeDef const& type);

    std::string get_full_swift_type_name(writer const& w, winmd::reader::TypeDef const& type);
    std::string get_full_swift_type_name(writer const& w, metadata_type const* type);
    std::string get_full_swift_type_name(writer const& w, metadata_type const& type);
}
