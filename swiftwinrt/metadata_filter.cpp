#include "pch.h"
#include <queue>

#include "types.h"
#include "common.h"
#include "type_helpers.h"
#include "types.h"
#include "metadata_cache.h"
#include "metadata_filter.h"
#include "attributes.h"
#include "helpers.h"
namespace swiftwinrt
{
    using processing_queue = std::queue<metadata_type const*>;
    inline void add_struct_dependencies_to_queue(processing_queue& to_process, struct_type const& type)
    {
        for (auto& member : type.members)
        {
            to_process.emplace(member.type);
        }
    }

    inline void add_class_dependencies_to_queue(processing_queue& to_process, class_type const& type)
    {
        if (type.base_class)
        {
            to_process.emplace(type.base_class);
        }
        for (auto& required : type.required_interfaces)
        {
            to_process.emplace(required.second.type);
        }
    }

    inline void add_interface_dependencies_to_queue(processing_queue& to_process, interface_type const& type)
    {
        for (auto& member : type.functions)
        {
            if (member.return_type)
            {
                to_process.emplace(member.return_type.value().type);
            }
            for (auto&& param : member.params)
            {
                to_process.emplace(param.type);
            }
        }

        for (auto& [_, required] : type.required_interfaces)
        {
            to_process.emplace(required.type);
        }
    }

    inline void add_delegate_dependencies_to_queue(processing_queue& to_process, delegate_type const& type)
    {
        auto invoke_func = type.functions[0];
        if (invoke_func.return_type)
        {
            to_process.emplace(invoke_func.return_type.value().type);
        }
        for (auto&& param : invoke_func.params)
        {
            to_process.emplace(param.type);
        }
    }

    inline auto add_generic_dependencies_to_queue(processing_queue& to_process, generic_inst const& generic)
    {
        for (auto& param : generic.generic_params())
        {
            to_process.emplace(param);
        }

        for (auto& dependency : generic.dependencies)
        {
            to_process.emplace(dependency);
        }

        for (auto& func : generic.functions)
        {
            if (func.return_type)
            {
                to_process.emplace(func.return_type.value().type);
            }
            for (auto&& param : func.params)
            {
                to_process.emplace(param.type);
            }
        }

        if (auto iface = dynamic_cast<interface_type const*>(generic.generic_type()))
        {
            for (auto& [_, required]: iface->required_interfaces)
            {
                to_process.emplace(required.type);
            }
        }
    }

    template<typename T>
    void add_ns_types_to_queue(processing_queue& to_process, std::vector<T> const& types)
    {
        for (auto& type : types)
        {
            to_process.emplace(&type);
        }
    }

    include_only_used_filter::include_only_used_filter(metadata_cache const& cache, std::vector<std::string> const& includes)
    {
        processing_queue to_process;
        for (auto& include : includes)
        {
            auto last_ns_index = include.find_last_of('.');
            if (last_ns_index == include.npos) {
                // If this is a namespace, then grab all types and add to queue for processing
                auto nsIter = cache.namespaces.find(include);
                if (nsIter != cache.namespaces.end())
                {
                    add_ns_types_to_queue(to_process, nsIter->second.classes);
                    add_ns_types_to_queue(to_process, nsIter->second.interfaces);
                    add_ns_types_to_queue(to_process, nsIter->second.delegates);
                    add_ns_types_to_queue(to_process, nsIter->second.enums);
                    add_ns_types_to_queue(to_process, nsIter->second.structs);
                    for (const auto [name, inst] : nsIter->second.generic_instantiations)
                    {
                        to_process.push(&inst);
                    }
                }
            }
            else {
                auto ns = include.substr(0, last_ns_index);
                auto name = include.substr(last_ns_index + 1);
                auto type = &cache.find(ns, name);
                to_process.push(type);
            }
        }
        namespaces.emplace("Windows.Foundation");
        while (!to_process.empty())
        {
            auto processing = to_process.front();
            
            type_name processing_name(processing);
            auto [iter, added] = types.emplace(std::piecewise_construct,
                std::forward_as_tuple(processing_name.name_space),
                std::forward_as_tuple());
            auto& full_type_names = iter->second;
            to_process.pop();
            auto processing_full_name = processing_name.name;
            if (full_type_names.find(processing_full_name) == full_type_names.end())
            {
                if (auto s = dynamic_cast<const struct_type*>(processing))
                {
                    add_struct_dependencies_to_queue(to_process, *s);
                }
                else if (auto c = dynamic_cast<const class_type*>(processing))
                {
                    add_class_dependencies_to_queue(to_process, *c);
                    for (auto& [name, attributed] : c->factories)
                    {
                        if (attributed.type)
                        {
                            to_process.emplace(attributed.type);
                        }
                    }

                    // This dependency isn't specified in metadata, but we need to specify it now because it's a known
                    // special case that Xaml Applications derive from IXamlMetadataProvider
                    if (c->swift_full_name() == "Microsoft.UI.Xaml.Application")
                    {
                        auto metadata_provider = &cache.find("Microsoft.UI.Xaml.Markup", "IXamlMetadataProvider");
                        to_process.emplace(metadata_provider);
                    }
                }
                else if (auto i = dynamic_cast<const interface_type*>(processing))
                {
                    add_interface_dependencies_to_queue(to_process, *i);
                }
                else if (auto d = dynamic_cast<const delegate_type*>(processing))
                {
                    add_delegate_dependencies_to_queue(to_process, *d);
                }
                else if (auto g = dynamic_cast<const generic_inst*>(processing))
                {
                    add_generic_dependencies_to_queue(to_process, *g);
                    generics.emplace(g->swift_full_name());
                }

                // enums and contracts will be added to processed queue, but they don't have any dependencies
                full_type_names.insert(processing_full_name);
                auto ns = processing->swift_logical_namespace();
                if (!ns.empty())
                {
                    namespaces.emplace(ns);
                }
            }
        }
    }

    bool include_only_used_filter::includes(winmd::reader::TypeDef const& type) const
    {
        auto ns = types.find(type.TypeNamespace());
        if (ns != types.end())
        {
            return ns->second.find(type.TypeName()) != ns->second.end();
        }

        return false;
    }

    bool include_all_filter::includes_ns(std::string_view const& ns) const
    {
        auto members = m_cache.namespaces();
        return has_projected_types(members[ns]);
    }
}