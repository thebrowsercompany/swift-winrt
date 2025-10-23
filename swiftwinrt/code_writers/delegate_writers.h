#pragma once
#include "types.h"
#include "../type_writers.h"
namespace swiftwinrt
{
    void write_delegate(writer& w, delegate_type const& type);
    void write_delegate_abi(writer& w, delegate_type const& type);
    void write_delegate_implementation(writer& w, delegate_type const& type);

    void write_vtable(writer& w, delegate_type const& type);
    void do_write_delegate_implementation(writer& w, metadata_type const& type, function_def const& invoke_method);
    void write_delegate_extension(writer& w, metadata_type const& inst, function_def const& invoke_method);
    void write_delegate_wrapper(writer& w, metadata_type const& type);
    void write_delegate_return_type(writer& w, function_def const& sig);
}
