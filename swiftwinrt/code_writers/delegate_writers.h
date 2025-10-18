#pragma once
#include "types.h"
#include "../type_writers.h"
namespace swiftwinrt
{
    void write_delegate(writer& w, delegate_type const& type);
    void write_delegate_abi(writer& w, delegate_type const& type);
    void write_delegate_implementation(writer& w, delegate_type const& type);
}
