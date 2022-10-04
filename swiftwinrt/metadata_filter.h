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
        bool includes(winmd::reader::TypeDef const& type) const { return f.includes(type); }
        bool includes_ns(std::string_view const& ns) const { return namespaces.find(std::string(ns)) != namespaces.end(); }
        bool includes_generic(std::string_view const& generic) const { return generics.find(std::string(generic)) != generics.end(); }

        winmd::reader::filter filter() { return f; }

        template <auto F, typename T>
        auto bind_each(std::vector<std::reference_wrapper<T const>> const& types) const
        {
            return [&](auto& writer)
            {
                for (auto&& type : types)
                {
                    if (includes(type.get().type()))
                    {
                        F(writer, type.get().type());
                    }
                }
            };
        }
    private:
        winmd::reader::filter f{};
        std::set<std::string> namespaces;
        std::set<std::string> generics;
    };
}