#pragma once

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "winmd_reader.h"

namespace swiftwinrt
{
    struct metadata_cache;
    struct metadata_filter
    {
        metadata_filter(metadata_cache const& cache, std::vector<std::string> const& includes);
        metadata_filter() = default;
        metadata_filter(const metadata_filter&) = default;
        bool includes(winmd::reader::TypeDef const& type) const;
        bool includes_ns(std::string_view const& ns) const { return namespaces.find(std::string(ns)) != namespaces.end(); }
        bool includes_generic(std::string_view const& generic) const { return generics.find(std::string(generic)) != generics.end(); }
        
        bool includes_any(cache::namespace_members const& members) const
        {
            for (auto&& type : members.types)
            {
                if (includes(type.second))
                {
                    return true;
                }
            }

            return false;
        }

        template <auto F, typename T>
        auto bind_each(std::vector<std::reference_wrapper<T const>> const& types) const
        {
            return [&](auto& writer)
            {
                for (auto&& type : types)
                {
                    if (includes(type.get().type()))
                    {
                        F(writer, type.get());
                    }
                }
            };
        }
    private:
        std::map<std::string_view, std::set<std::string_view>> types;
        std::set<std::string> namespaces;
        std::set<std::string> generics;
    };
}