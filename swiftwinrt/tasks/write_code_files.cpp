#include "write_code_files.h"
#include "utility/settings.h"
#include "file_writers/file_writers.h"
#include "file_writers/abi_writer.h"
namespace swiftwinrt
{
    void write_swift_code_files(
        task_group& group,
        std::string_view const& module,
        std::vector<std::string_view> const& namespaces,
        metadata_cache const& mdCache,
        include_only_used_filter const& mf)
    {
         group.add([&,
            module,
            namespaces]
        {
            swiftwinrt::task_group module_group;
            module_group.add([module, &namespaces, &mdCache, &mf]
            {
                // generics are written on a per module basis because this helps us reduce the
                // amount of code that is generated.
                auto types = mdCache.compile_namespaces(namespaces, mf);
                write_module_generics(module, types, mf);
            });

            if (module == settings.support)
            {
                module_group.add([module]
                {
                    write_swift_support_files(module);
                });
            }

            for (auto& ns : namespaces)
            {
                module_group.add([ns, &mdCache, &mf]
                {
                    auto types = mdCache.compile_namespaces({ ns }, mf);
                    write_namespace_impl(ns, types, mf);
                    write_namespace_enums(ns, types, mf);
                    write_namespace_classes(ns, types, mf);
                    write_namespace_structs(ns, types, mf);
                    write_namespace_interfaces(ns, types, mf);
                    write_namespace_delegates(ns, types, mf);
                });
            } 
        
            module_group.get();
        });
        group.add([] { write_cwinrt_build_files(); });

    }

    void write_c_code_files(
        task_group& group,
        std::string_view const& module,
        std::vector<std::string_view> const& namespaces,
        metadata_cache const& mdCache,
        include_all_filter const& mf)
    {
      group.add([module, namespaces, &mdCache, &mf]
        {
            swiftwinrt::task_group module_group;
            for (auto& ns : namespaces)
            {
                module_group.add([ns, &mdCache, &mf]
                {
                    auto types = mdCache.compile_namespaces({ ns }, mf);
                    write_abi_header(ns, types);
                });
            } 
        
            module_group.get();
        });
    }
}