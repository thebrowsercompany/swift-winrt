#include "interface_writers.h"
#include "writer_helpers.h"
#include "delegate_writers.h"
#include "utility/swift_codegen_utils.h"

namespace swiftwinrt
{
    void write_delegate(writer& w, delegate_type const& type)
    {
        // Delegates require tuples because of the way that the bridges are implemented.
        // The bridge classes have a typealias for the parameters, and we use those
        // parameters for the delegate signature to create the bridge. The swift compiler
        // complains if the typealias isn't placed in a tuple
        function_def delegate_method = type.functions[0];
        w.write("public typealias % = (%) throws -> %\n",
            type,
            bind<write_comma_param_types>(delegate_method.params),
            bind<write_delegate_return_type>(delegate_method));
    }

    void write_delegate_implementation(writer& w, delegate_type const& type)
    {
        if (can_write(w, type) && !type.is_generic())
        {
            auto delegate_method = type.functions[0];
            do_write_delegate_implementation(w, type, delegate_method);
        }
    }

    void write_delegate_abi(writer& w, delegate_type const& type)
    {
        if (type.is_generic()) return;
        w.write("@_spi(WinRTInternal)\n");
        w.write("extension % {\n", abi_namespace(w.type_namespace));
        {
            auto guard(w.push_indent());
            write_guid(w, type);
            do_write_interface_abi(w, type, type.functions);

            write_delegate_wrapper(w, type);
            write_vtable(w, type);
        }
        w.write("}\n");
        write_delegate_extension(w, type, type.functions[0]);
    }
}
