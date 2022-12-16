#pragma once

#include <map>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "winmd_reader.h"
#include "task_group.h"
#include "types.h"

namespace swiftwinrt
{
    struct api_contract
    {
        winmd::reader::TypeDef type;
        std::uint32_t current_version;
    };

    struct category_compare
    {
        bool operator()(typedef_base const& lhs, typedef_base const& rhs) const
        {
            using namespace winmd::reader;
            auto leftCat = get_category(lhs.type());
            auto rightCat = get_category(rhs.type());
            if (leftCat == rightCat)
            {
                return lhs.swift_full_name() < rhs.swift_full_name();
            }

            auto category_power = [](category cat)
            {
                switch (cat)
                {
                case category::enum_type: return 0;
                case category::struct_type: return 1;
                case category::delegate_type: return 2;
                case category::interface_type: return 3;
                case category::class_type: return 4;
                default: return 100;
                }
            };
            return category_power(leftCat) < category_power(rightCat);
        }
    };

    struct metadata_cache;

    struct type_cache
    {
        metadata_cache const* cache;

        // Definitions
        std::vector<std::reference_wrapper<enum_type const>> enums;
        std::vector<std::reference_wrapper<struct_type const>> structs;
        std::vector<std::reference_wrapper<delegate_type const>> delegates;
        std::vector<std::reference_wrapper<interface_type const>> interfaces;
        std::vector<std::reference_wrapper<class_type const>> classes;

        // Dependencies
        std::set<std::string_view> dependent_namespaces;
        std::map<std::string_view, std::reference_wrapper<generic_inst const>> generic_instantiations;
        std::set<std::reference_wrapper<typedef_base const>> external_dependencies;
        std::set<std::reference_wrapper<typedef_base const>, category_compare> internal_dependencies;
    };

    struct namespace_cache
    {
        // Definitions
        std::vector<enum_type> enums;
        std::vector<struct_type> structs;
        std::vector<delegate_type> delegates;
        std::vector<interface_type> interfaces;
        std::vector<class_type> classes;
        std::vector<api_contract> contracts;

        // Dependencies
        std::set<std::string_view> dependent_namespaces;
        std::map<std::string_view, generic_inst> generic_instantiations;
        std::set<std::reference_wrapper<typedef_base const>> type_dependencies;
    };

    struct metadata_filter;
    struct metadata_cache
    {
        std::map<std::string_view, namespace_cache> namespaces;

        metadata_cache(winmd::reader::cache const& c);

        type_cache compile_namespaces(std::vector<std::string_view> const& targetNamespaces, metadata_filter const& f);

        metadata_type const* try_find(std::string_view typeNamespace, std::string_view typeName) const
        {
            if (typeNamespace == system_namespace)
            {
                return &system_type::from_name(typeName);
            }

            auto nsItr = m_typeTable.find(typeNamespace);
            if (nsItr != m_typeTable.end())
            {
                auto nameItr = nsItr->second.find(typeName);
                if (nameItr != nsItr->second.end())
                {
                    return &nameItr->second;
                }
            }

            return nullptr;
        }

        metadata_type const& find(std::string_view typeNamespace, std::string_view typeName) const
        {
            if (auto ptr = try_find(typeNamespace, typeName))
            {
                return *ptr;
            }

            swiftwinrt::throw_invalid("Could not find type '", typeName, "' in namespace '", typeNamespace, "'");
        }

    private:

        void process_namespace_types(
            winmd::reader::cache::namespace_members const& members,
            namespace_cache& target,
            std::map<std::string_view, metadata_type const&>& table);

        struct init_state
        {
            namespace_cache* target;
            generic_inst const* parent_generic_inst = nullptr;
        };

        void process_namespace_dependencies(namespace_cache& target);
        void process_enum_dependencies(init_state& state, enum_type& type);
        void process_struct_dependencies(init_state& state, struct_type& type);
        void process_delegate_dependencies(init_state& state, delegate_type& type);
        void process_interface_dependencies(init_state& state, interface_type& type);
        void process_class_dependencies(init_state& state, class_type& type);

        using relative_version = std::pair<std::size_t, std::uint32_t>;
        using relative_version_map = std::unordered_map<interface_type const*, relative_version>;
        void process_fastabi_required_interfaces(init_state& state, interface_type const* currentInterface, relative_version rank, relative_version_map& interfaceMap);

        function_def process_function(init_state& state, winmd::reader::MethodDef const& def);
        property_def process_property(init_state& state, winmd::reader::Property const& def);
        event_def process_event(init_state& state, Event const& def);

        metadata_type const& find_dependent_type(init_state& state, winmd::reader::TypeSig const& type);
        metadata_type const& find_dependent_type(init_state& state, winmd::reader::coded_index<winmd::reader::TypeDefOrRef> const& type);
        metadata_type const& find_dependent_type(init_state& state, winmd::reader::GenericTypeInstSig const& type);

        using get_interfaces_t = std::vector<named_interface_info>;

        get_interfaces_t get_interfaces(init_state& state, winmd::reader::TypeDef const& type);
        void get_interfaces_impl(init_state& state, writer& w, get_interfaces_t& result, bool defaulted, bool overridable, bool base, std::pair<InterfaceImpl, InterfaceImpl>&& children);
        interface_info* find(get_interfaces_t& interfaces, std::string_view const& name);
        void insert_or_assign(get_interfaces_t& interfaces, std::string_view const& name, interface_info&& info);

        std::map<std::string, attributed_type> get_attributed_types(winmd::reader::TypeDef const& type) const;

        std::map<std::string_view, std::map<std::string_view, metadata_type const&>> m_typeTable;
    };
}
