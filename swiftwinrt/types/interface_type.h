#pragma once

#include <vector>

#include "types/event_def.h"
#include "types/function_def.h"
#include "types/interface_info.h"
#include "types/property_def.h"
#include "types/typedef_base.h"

namespace swiftwinrt
{
    struct class_type;

    struct interface_type : typedef_base
    {
        explicit interface_type(winmd::reader::TypeDef const& type) :
            typedef_base(type)
        {
        }

        void append_signature(sha1& hash) const override;
        void write_c_forward_declaration(writer& w) const override;
        void write_c_abi_param(writer& w) const override;
        void write_c_definition(writer& w) const;

        std::vector<named_interface_info> required_interfaces;
        std::vector<function_def> functions;
        std::vector<property_def> properties;
        std::vector<event_def> events;
        class_type const* fast_class{ nullptr };
    };
}
