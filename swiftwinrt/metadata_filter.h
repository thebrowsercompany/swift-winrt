#pragma once

#include <map>
#include <string>
#include <unordered_map>
#include <vector>
#include <queue>

#include "winmd_reader.h"
#include "types.h"
#include "metadata_cache.h"

namespace swiftwinrt
{
    inline void add_struct_dependencies_to_queue(std::queue<std::string>& to_process, struct_type const& type)
    {
        for (auto& member : type.members)
        {
            to_process.emplace(member.type->swift_full_name());
        }
    }

    inline void add_class_dependencies_to_queue(std::queue<std::string>& to_process, class_type const& type)
    {
        if (type.base_class)
        {
            to_process.emplace(type.base_class->swift_full_name());
        }
        for (auto& required : type.required_interfaces)
        {
            to_process.emplace(required->swift_full_name());
        }

        for (auto& [name, attributed] : get_attributed_types(type.type()))
        {
            if (attributed.type)
            {
                auto full_name = get_full_type_name(attributed.type);
                to_process.emplace(full_name);
            }
        }
    }

    inline void add_interface_dependencies_to_queue(std::queue<std::string>& to_process, interface_type const& type)
    {
        for (auto& member : type.functions)
        {
            if (member.return_type)
            {
                to_process.emplace(member.return_type.value().type->swift_full_name());
            }
            for (auto&& param : member.params)
            {
                to_process.emplace(param.type->swift_full_name());
            }
        }
    }

    inline void add_delegate_dependencies_to_queue(std::queue<std::string>& to_process, delegate_type const& type)
    {
        auto invoke_func = type.functions[0];
        if (invoke_func.return_type)
        {
            to_process.emplace(invoke_func.return_type.value().type->swift_full_name());
        }
        for (auto&& param : invoke_func.params)
        {
            to_process.emplace(param.type->swift_full_name());
        }
    }

    inline void add_generic_params_to_queue(std::queue<std::string>& to_process, std::string generic_name)
    {
        // for generic types we should grab the types, there can only be 1 or two.
        // but for now just only include the whole type
        auto first_bracket = generic_name.find_first_of('<');
        auto last_bracket = generic_name.find_last_of('>');
        auto generic_arg_count = atoi(&generic_name[first_bracket - 1]);
        if (generic_arg_count == 1)
        {
            auto param = generic_name.substr(first_bracket + 1, last_bracket - first_bracket - 1);
            to_process.emplace(param);
        }
        else
        {
            auto comma = generic_name.find_first_of(',');
            auto first = generic_name.substr(first_bracket + 1, comma - first_bracket -1);
            auto second = generic_name.substr(comma + 2, last_bracket - comma - 2);
            to_process.emplace(first);
            to_process.emplace(second);
        }
    }
    
    struct metadata_filter
    {
        metadata_filter(metadata_cache const& cache, std::vector<std::string> const& includes)
        {
            std::set<std::string> processed;
            std::queue<std::string> to_process;
            for (auto& include : includes)
            {
                to_process.push(include);
            }
            namespaces.emplace("Windows.Foundation");
            while (!to_process.empty())
            {
                std::string processing = to_process.front();
                to_process.pop();
                if (processed.find(processing) == processed.end())
                {
                    auto last_ns_index = processing.find_last_of('.');
                    // assume primitive type if no namespace
                    if (last_ns_index == processing.npos) {
                        processed.insert(processing);
                        continue;
                    }
                    auto type_ns = processing.substr(0, last_ns_index);
                    auto type_name = processing.substr(last_ns_index + 1);
                    
                    auto back_tick_index = processing.find_first_of('`');
                    if (back_tick_index != processing.npos)
                    {
                        if (processing.starts_with("Windows.Foundation.Collections"))
                        {
                            namespaces.emplace("Windows.Foundation.Collections");
                        }
                        add_generic_params_to_queue(to_process, processing);
                        processed.insert(processing);
                        generics.insert(processing);
                        continue;
                    }
                    auto type = &cache.find(type_ns, type_name);
                    if (auto s = dynamic_cast<const struct_type*>(type))
                    {
                        add_struct_dependencies_to_queue(to_process, *s);
                    }
                    else if (auto c = dynamic_cast<const class_type*>(type))
                    {
                        add_class_dependencies_to_queue(to_process, *c);
                    }
                    else if (auto i = dynamic_cast<const interface_type*>(type))
                    {
                        add_interface_dependencies_to_queue(to_process, *i);
                    }
                    else if (auto d = dynamic_cast<const delegate_type*>(type))
                    {
                        add_delegate_dependencies_to_queue(to_process, *d);
                    }

                    // enums and contracts will be added to processed queue, but they don't have any dependencies
                    processed.insert(processing);
                    namespaces.emplace(type_ns);
                }
            }
            
            f = winmd::reader::filter{ processed, {}, true };
        }

        bool includes(winmd::reader::TypeDef const& type) const { return f.includes(type); }
        bool includes_ns(std::string_view const& ns) const { return namespaces.find(std::string(ns)) != namespaces.end(); }
        bool includes_generic(std::string_view const& generic) const { return generics.find(std::string(generic)) != generics.end(); }

        winmd::reader::filter filter() { return f; }
    private:
        winmd::reader::filter f{};
        std::set<std::string> namespaces;
        std::set<std::string> generics;
    };
}