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
        virtual bool includes(winmd::reader::TypeDef const& type) const = 0;
        virtual bool includes_ns(std::string_view const& ns) const = 0;
        virtual bool includes_generic(std::string_view const& generic) const = 0;
        
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

    };

    struct include_only_used_filter : metadata_filter {
        include_only_used_filter(metadata_cache const& cache, std::vector<std::string> const& includes);
        include_only_used_filter() = default;
        include_only_used_filter(const include_only_used_filter&) = default;

        virtual bool includes(winmd::reader::TypeDef const& type) const;
        virtual bool includes_ns(std::string_view const& ns) const { return namespaces.find(std::string(ns)) != namespaces.end(); }
        virtual bool includes_generic(std::string_view const& generic) const { return generics.find(std::string(generic)) != generics.end(); }

    private:
        std::map<std::string_view, std::set<std::string_view>> types;
        std::set<std::string> namespaces;
        std::set<std::string> generics;
    };

    struct include_all_filter : metadata_filter {
        include_all_filter(winmd::reader::cache& c) : m_cache(c) {};
        bool includes(winmd::reader::TypeDef const& type) const override { return true; }
        bool includes_ns(std::string_view const& ns) const override;
        bool includes_generic(std::string_view const& generic) const override { return true; }

    private:
        winmd::reader::cache& m_cache;
    };
}