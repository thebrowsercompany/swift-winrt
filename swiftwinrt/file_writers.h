#pragma once

namespace swiftwinrt
{

    static void write_namespace_abi(std::string_view const& ns, cache::namespace_members const& members, settings_type const& settings)
    {
        writer w;
        w.type_namespace = ns;
        w.ns_prefix_state = settings.nsprefix;
        write_preamble(w, settings.c_import);

        w.write_each<write_guid>(members.interfaces);
        w.write("public struct ABI {\n");
        {
            auto indent = w.push_indent({ 1 });
            w.write_each<write_interface_abi>(members.interfaces);
            w.write_each<write_delegate_abi>(members.delegates);
            w.write_each<write_struct_abi>(members.structs);
        }
        w.write("}\n\n");
        w.write_each<write_enum_abi>(members.enums);
        w.save_file("ABI");
    }

    static void write_namespace_wrapper(std::string_view const& ns, cache::namespace_members const& members, settings_type const& settings)
    {
        writer w;
        w.type_namespace = ns;
        write_preamble(w, settings.c_import);


        w.write_each<write_enum>(members.enums);
        w.write_each<write_class>(members.classes);
        w.write_each<write_delegate>(members.delegates);
        w.write_each<write_struct>(members.structs);
        w.write("public struct Impl {\n");
        {
            auto indent = w.push_indent({ 1 });
            w.write_each<write_interface_impl>(members.interfaces);
        }
        w.write("}\n\n");
        w.write_each<write_interface>(members.interfaces);

        w.save_file();
    }
}
