#pragma once

#include "types/generic_inst.h"

namespace swiftwinrt
{
    struct writer;

    void write_guid_generic(writer& w, generic_inst const& type);
    void write_interface_generic(writer& w, generic_inst const& type);
    void write_generic_extension(writer& w, generic_inst const& inst);
    void write_generic_implementation(writer& w, generic_inst const& type);

    void write_ireference_init_extension(writer& w, generic_inst const& type);
    void write_generic_delegate_wrapper(writer& w, generic_inst const& generic);
    void write_generic_interface_implementation(writer& w, generic_inst const& type);
    void write_generic_declaration(writer& w, generic_inst const& type);
}

