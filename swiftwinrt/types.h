#pragma once

#include "winmd_reader.h"
#include "sha1.h"
#include "versioning.h"

namespace swiftwinrt
{
    struct writer;

    std::string get_full_type_name(TypeDef const& type);
    std::string_view remove_tick(std::string_view const& name);
    struct deprecation_info
    {
        std::string_view contract_type;
        std::uint32_t version;

        std::string_view message;
    };

    struct metadata_type
    {
        virtual std::string_view swift_full_name() const = 0;
        virtual std::string_view swift_type_name() const = 0;
        virtual std::string_view swift_abi_namespace() const = 0;
        virtual std::string_view swift_logical_namespace() const = 0;

        virtual std::string_view cpp_abi_name() const = 0;
        virtual std::string_view cpp_logical_name() const = 0;

        virtual std::string_view mangled_name() const = 0;
        virtual std::string_view generic_param_mangled_name() const = 0;

        virtual void append_signature(sha1& hash) const = 0;

        virtual void write_c_forward_declaration(writer& w) const = 0;
        virtual void write_c_abi_param(writer& w) const = 0;

        virtual void write_swift_declaration(writer& w) const = 0;
        /*
        virtual void write_swift_abi_param(writer& w) const = 0;
        virtual void write_swift_param(writer& w) const = 0;
        */

        virtual bool is_experimental() const = 0;

        virtual std::optional<std::size_t> contract_index(std::string_view /*typeName*/, std::size_t /*version*/) const
        {
            return std::nullopt;
        }

        virtual std::optional<contract_version> contract_from_index(std::size_t /*index*/) const
        {
            return std::nullopt;
        }
    };

    inline bool operator<(metadata_type const& lhs, metadata_type const& rhs) noexcept
    {
        return lhs.swift_full_name() < rhs.swift_full_name();
    }

    struct typename_comparator
    {
        bool operator()(std::string_view lhs, metadata_type const& rhs)
        {
            return lhs < rhs.swift_full_name();
        }

        bool operator()(metadata_type const& lhs, std::string_view rhs)
        {
            return lhs.swift_full_name() < rhs;
        }
    };

    struct element_type final : metadata_type
    {
        element_type(
            ElementType elementType,
            std::string_view swiftName,
            std::string_view logicalName,
            std::string_view abiName,
            std::string_view cppName,
            std::string_view mangledName,
            std::string_view signature) :
            m_type(elementType),
            m_swiftName(swiftName),
            m_logicalName(logicalName),
            m_abiName(abiName),
            m_cppName(cppName),
            m_mangledName(mangledName),
            m_signature(signature)
        {
        }

        static element_type const& from_type(winmd::reader::ElementType type);

        virtual std::string_view swift_abi_namespace() const override
        {
            return {};
        }

        virtual std::string_view swift_logical_namespace() const override
        {
            return {};
        }

        virtual std::string_view swift_full_name() const override
        {
            return m_swiftName;
        }

        virtual std::string_view swift_type_name() const override
        {
            return m_swiftName;
        }

        virtual std::string_view cpp_abi_name() const override
        {
            return m_cppName;
        }

        virtual std::string_view cpp_logical_name() const override
        {
            return m_logicalName;
        }

        virtual std::string_view mangled_name() const override
        {
            return m_mangledName;
        }

        virtual std::string_view generic_param_mangled_name() const override
        {
            return m_mangledName;
        }

        virtual void append_signature(sha1& hash) const override
        {
            hash.append(m_signature);
        }

        virtual void write_c_forward_declaration(writer&) const override
        {
            // No forward declaration necessary
        }

        virtual void write_c_abi_param(writer& w) const override;

        virtual bool is_experimental() const override
        {
            // Element types are never experimental
            return false;
        }

        virtual void write_swift_declaration(writer&) const override
        {
            // no special declaration necessary
        }

        bool is_blittable() const;

        ElementType type() const { return m_type;  }
    private:

        std::string_view m_swiftName;
        std::string_view m_logicalName;
        std::string_view m_abiName;
        std::string_view m_cppName;
        std::string_view m_mangledName;
        std::string_view m_signature;
        bool m_blittable{ true };
        ElementType m_type;
    };

    struct system_type final : metadata_type
    {
        system_type(
            std::string_view swiftModule,
            std::string_view swiftTypeName,
            std::string_view swiftFullName,
            std::string_view cppName,
            std::string_view signature,
            param_category category) :
            m_swiftModuleName(swiftModule),
            m_swiftTypeName(swiftTypeName),
            m_swiftFullName(swiftFullName),
            m_cppName(cppName),
            m_signature(signature),
            m_category(category)
        {
        }

        static system_type const& from_name(std::string_view typeName);

        virtual std::string_view swift_abi_namespace() const override
        {
            // Currently all mapped types from the System namespace have no namespace
            return {};
        }


        virtual std::string_view swift_logical_namespace() const override
        {
            return m_swiftModuleName;
        }

        virtual std::string_view swift_full_name() const override
        {
            return m_swiftFullName;
        }

        virtual std::string_view swift_type_name() const override
        {
            return m_swiftTypeName;
        }

        virtual std::string_view cpp_abi_name() const override
        {
            // Currently all mapped types from the System namespace do not have differing ABI/logical names
            return m_cppName;
        }

        virtual std::string_view cpp_logical_name() const override
        {
            return m_cppName;
        }

        virtual std::string_view mangled_name() const override
        {
            return m_cppName;
        }

        virtual std::string_view generic_param_mangled_name() const override
        {
            // Currently all mangled names from the System namespace match their C++ name (i.e. no '_' etc. in the name)
            return m_cppName;
        }

        virtual void append_signature(sha1& hash) const override
        {
            hash.append(m_signature);
        }

        virtual void write_c_forward_declaration(writer&) const override
        {
            // No forward declaration necessary
        }

        virtual void write_c_abi_param(writer& w) const override;

        virtual bool is_experimental() const override
        {
            // System types are never experimental
            return false;
        }

        virtual void write_swift_declaration(writer&) const override
        {
            // no special declaration necessary
        }

        param_category category() const { return m_category; }
    private:

        std::string_view m_swiftModuleName;
        std::string_view m_swiftTypeName;
        std::string_view m_swiftFullName;
        param_category m_category;
        std::string_view m_cppName;
        std::string_view m_signature;
    };

    struct mapped_type final : metadata_type
    {
        mapped_type(
            winmd::reader::TypeDef const& type,
            std::string_view cppName,
            std::string_view mangledName,
            std::string_view signature) :
            m_type(type),
            m_swiftFullName(swiftwinrt::get_full_type_name(type)),
            m_cppName(cppName),
            m_mangledName(mangledName),
            m_signature(signature)
        {
        }

        static mapped_type const* from_typedef(winmd::reader::TypeDef const& type);

        virtual std::string_view swift_abi_namespace() const override
        {
            return m_type.TypeNamespace();
        }

        virtual std::string_view swift_logical_namespace() const override
        {
            // Currently all mapped types are in the global namespace
            return {};
        }

        virtual std::string_view swift_full_name() const override
        {
            return m_swiftFullName;
        }

        virtual std::string_view swift_type_name() const override
        {
            return m_type.TypeName();
        }

        virtual std::string_view cpp_abi_name() const override
        {
            // Currently no mapped types have differing ABI/logical names
            return m_cppName;
        }

        virtual std::string_view cpp_logical_name() const override
        {
            return m_cppName;
        }

        virtual std::string_view mangled_name() const override
        {
            return m_mangledName;
        }

        virtual std::string_view generic_param_mangled_name() const override
        {
            // Currently no mapped type names have any characters that would make it so that the mangled name would be
            // different than the generic param mangled name (i.e. no underscore, etc.)
            return m_mangledName;
        }

        virtual void append_signature(sha1& hash) const override
        {
            hash.append(m_signature);
        }

        virtual void write_c_forward_declaration(writer&) const override
        {
            // No forward declaration necessary
        }

        virtual void write_c_abi_param(writer& w) const override;

        virtual bool is_experimental() const override
        {
            // Mapped types are never experimental
            return false;
        }

        virtual std::optional<std::size_t> contract_index(std::string_view typeName, std::size_t /*version*/) const override
        {
            // All mapped types are introduced in the UniversalApiContract version 1
            using namespace std::literals;
            if (typeName != "Windows.Foundation.UniversalApiContract"sv)
            {
                return std::nullopt;
            }

            return 0;
        }

        virtual std::optional<contract_version> contract_from_index([[maybe_unused]] std::size_t index) const override
        {
            using namespace std::literals;
            XLANG_ASSERT(index == 0);
            return contract_version{ "Windows.Foundation.UniversalApiContract"sv, 1 };
        }

        virtual void write_swift_declaration(writer&) const override
        {
            // no special declaration necessary
        }

        TypeDef type() const { return m_type; }
    private:

        winmd::reader::TypeDef m_type;
        std::string m_swiftFullName;
        std::string_view m_cppName;
        std::string_view m_mangledName;
        std::string_view m_signature;
    };
    struct generic_type_parameter;

    struct typedef_base : metadata_type
    {
        typedef_base(winmd::reader::TypeDef const& type);

        virtual std::string_view swift_abi_namespace() const override
        {
            // Most types don't have a different abi and logical type, so use the logical namespace as an easy default
            return m_type.TypeNamespace();
        }

        virtual std::string_view swift_logical_namespace() const override
        {
            return m_type.TypeNamespace();
        }

        virtual std::string_view swift_full_name() const override
        {
            return m_swiftFullName;
        }

        virtual std::string_view cpp_abi_name() const override
        {
            // Most types just use the swift type name, so use that as the easy default
            return m_type.TypeName();
        }

        virtual std::string_view swift_type_name() const override
        {
            return m_type.TypeName();
        }

        virtual std::string_view cpp_logical_name() const override
        {
            // Most types just use the swift type name, so use that as the easy default
            return m_type.TypeName();
        }

        virtual std::string_view mangled_name() const override
        {
            return m_mangledName;
        }

        virtual std::string_view generic_param_mangled_name() const override
        {
            // Only generic instantiations should be used as generic params
            XLANG_ASSERT(!is_generic());
            return m_genericParamMangledName;
        }

        virtual bool is_experimental() const override;

        virtual std::optional<std::size_t> contract_index(std::string_view typeName, std::size_t version) const override
        {
            if (!m_contractHistory)
            {
                return std::nullopt;
            }

            // Start with previous contracts
            std::size_t result = 0;
            for (auto& prev : m_contractHistory->previous_contracts)
            {
                if ((prev.contract_from == typeName) && (prev.version_low <= version) && (prev.version_high > version))
                {
                    return result;
                }

                ++result;
            }

            // Now the current contract
            if ((m_contractHistory->current_contract.name == typeName) && (m_contractHistory->current_contract.version <= version))
            {
                return result;
            }

            return std::nullopt;
        }

        virtual std::optional<contract_version> contract_from_index(std::size_t index) const override
        {
            if (!m_contractHistory)
            {
                return std::nullopt;
            }

            // Start with previous contracts
            for (auto& prev : m_contractHistory->previous_contracts)
            {
                if (index-- == 0)
                {
                    return contract_version{ prev.contract_from, prev.version_low };
                }
            }

            if (index == 0)
            {
                return m_contractHistory->current_contract;
            }

            XLANG_ASSERT(false);
            return std::nullopt;
        }

        winmd::reader::TypeDef const& type() const noexcept
        {
            return m_type;
        }

        bool is_generic() const noexcept;

        std::optional<deprecation_info> is_deprecated() const noexcept;

        winmd::reader::category category() const noexcept
        {
            return get_category(m_type);
        }

        virtual void write_swift_declaration(writer&) const override
        {
            // not implemented by everyone yet
        }

        std::vector<generic_type_parameter> generic_params;

    protected:

        winmd::reader::TypeDef m_type;

        // These strings are initialized by the base class
        std::string m_swiftFullName;
        std::string m_mangledName;
        std::string m_genericParamMangledName;

        // Versioning information filled in by the base class constructor
        std::vector<platform_version> m_platformVersions;
        std::optional<contract_history> m_contractHistory;
    };

    struct enum_type final : typedef_base
    {
        enum_type(winmd::reader::TypeDef const& type) :
            typedef_base(type)
        {
        }

        virtual void append_signature(sha1& hash) const override
        {
            using namespace std::literals;
            hash.append("enum("sv);
            hash.append(m_swiftFullName);
            hash.append(";"sv);
            element_type::from_type(underlying_type()).append_signature(hash);
            hash.append(")"sv);
        }

        virtual void write_c_forward_declaration(writer& w) const override;
        virtual void write_c_abi_param(writer& w) const override;

        void write_c_definition(writer& w) const;

        winmd::reader::ElementType underlying_type() const;
    };

    struct struct_member
    {
        winmd::reader::Field field;
        metadata_type const* type;
    };

    struct struct_type final : typedef_base
    {
        struct_type(winmd::reader::TypeDef const& type) :
            typedef_base(type)
        {
        }

        virtual void append_signature(sha1& hash) const override
        {
            using namespace std::literals;
            XLANG_ASSERT(members.size() == static_cast<std::size_t>(distance(m_type.FieldList())));
            hash.append("struct("sv);
            hash.append(m_swiftFullName);
            for (auto const& member : members)
            {
                hash.append(";");
                member.type->append_signature(hash);
            }
            hash.append(")"sv);
        }

        virtual void write_c_forward_declaration(writer& w) const override;
        virtual void write_c_abi_param(writer& w) const override;

        void write_c_definition(writer& w) const;

        std::vector<struct_member> members;
    };

    struct function_return_type
    {
        winmd::reader::RetTypeSig signature;
        std::string_view name;
        metadata_type const* type;

        bool in() const { return false; }
        bool out() const { return true; }
        bool is_array() const { return signature.Type().is_szarray() || signature.Type().is_array(); }
    };

    struct function_param
    {
        winmd::reader::Param def;
        winmd::reader::ParamSig signature;
        std::string_view name;
        metadata_type const* type;

        bool in() const { return def.Flags().In(); }
        bool out() const { return def.Flags().Out(); }
        bool is_array() const { return signature.Type().is_szarray() || signature.Type().is_array(); }
    };

    struct function_def
    {
        winmd::reader::MethodDef def;
        std::optional<function_return_type> return_type;
        std::vector<function_param> params;
        bool is_async() const;
    };

    struct property_def
    {
        winmd::reader::Property def;
        metadata_type const* type;
        std::optional<function_def> getter;
        std::optional<function_def> setter;
        bool is_array() const { return def.Type().Type().is_szarray() || def.Type().Type().is_array(); }
    };

    struct event_def
    {
        winmd::reader::Event def;
        metadata_type const* type;
    };


    using generic_param_type = const metadata_type*;
    using generic_param_vector = std::vector<generic_param_type>;
    struct interface_info
    {
        const metadata_type* type{};
        bool is_default{};
        bool defaulted{};
        bool overridable{};
        bool base{};
        bool exclusive{};
        bool fastabi{};
        bool attributed{};

        // A pair of (relativeContract, version) where 'relativeContract' is the contract the interface was introduced
        // in relative to the contract history of the class. E.g. if a class goes from contract 'A' to 'B' to 'C',
        // 'relativeContract' would be '0' for an interface introduced in contract 'A', '1' for an interface introduced
        // in contract 'B', etc. This is only set/valid for 'fastabi' interfaces
        std::pair<uint32_t, uint32_t> relative_version{};
        generic_param_vector generic_params{};
    };

    using named_interface_info = std::pair<std::string, interface_info>;

    struct attributed_type
    {
        const metadata_type* type;
        bool activatable{};
        bool statics{};
        bool composable{};
        bool visible{};
        bool defaultComposable{};
    };

    struct delegate_type final : typedef_base
    {
        delegate_type(winmd::reader::TypeDef const& type);

        virtual std::string_view cpp_logical_name() const override
        {
            // Even though the ABI name of delegates is different than their swift name, the logical name is still the same as
            // the ABI name
            return m_abiName;
        }

        virtual void append_signature(sha1& hash) const override;

        virtual void write_c_forward_declaration(writer& w) const override;
        virtual void write_c_abi_param(writer& w) const override;

        void write_c_definition(writer& w) const;

        std::vector<function_def> functions;

    private:

        std::string m_abiName;
    };

    struct class_type;

    struct interface_type : typedef_base
    {
        interface_type(winmd::reader::TypeDef const& type) :
            typedef_base(type)
        {
        }

        virtual void append_signature(sha1& hash) const override;

        virtual void write_c_forward_declaration(writer& w) const override;
        virtual void write_c_abi_param(writer& w) const override;

        void write_c_definition(writer& w) const;

        std::vector<named_interface_info> required_interfaces;
        std::vector<function_def> functions;
        std::vector<property_def> properties;
        std::vector<event_def> events;

        // When non-null, this interface gets extended with functions from other exclusiveto interfaces on the class
        class_type const* fast_class = nullptr;
    };

    struct generic_type_parameter final : metadata_type {
        generic_type_parameter(std::string_view name) : param_name(name)
        {
        }

        virtual std::string_view swift_full_name() const override
        {
            return param_name;
        }


        virtual std::string_view swift_type_name() const override
        {
            return param_name;
        }

        virtual std::string_view swift_abi_namespace() const override
        {
            return {};
        }

        virtual std::string_view swift_logical_namespace() const override
        {
            return {};
        }

        virtual std::string_view cpp_abi_name() const override
        {
            return param_name;
        }

        virtual std::string_view cpp_logical_name() const override
        {
            return param_name;
        }

        virtual std::string_view mangled_name() const override
        {
            return param_name;
        }

        virtual std::string_view generic_param_mangled_name() const override
        {
            return param_name;
        }

        virtual void append_signature(sha1& hash) const override
        {
        }

        virtual void write_c_forward_declaration(writer&) const override
        {
            // No forward declaration necessary
        }

        virtual void write_c_abi_param(writer& w) const override {}

        virtual bool is_experimental() const override
        {
            return false;
        }

        virtual void write_swift_declaration(writer&) const override
        {
            // no special declaration necessary
        }

    private:
        std::string_view param_name;
    };

    struct class_type final : typedef_base
    {
        class_type(winmd::reader::TypeDef const& type);

        virtual std::string_view swift_abi_namespace() const override
        {
            if (m_abiNamespace.empty())
            {
                swiftwinrt::throw_invalid("Class type '", swift_full_name(), "' does not have a default interface and therefore "
                    "does not have an ABI type namespace");
            }

            return m_abiNamespace;
        }

        virtual std::string_view cpp_abi_name() const override
        {
            if (!default_interface)
            {
                swiftwinrt::throw_invalid("Class type '", swift_full_name(), "' does not have a default interface and therefore "
                    "does not have an ABI type name");
            }

            return default_interface->cpp_abi_name();
        }

        virtual void append_signature(sha1& hash) const override
        {
            using namespace std::literals;
            if (!default_interface)
            {
                swiftwinrt::throw_invalid("Class type '", swift_full_name(), "' does not have a default interface and therefore "
                    "does not have a signature");
            }

            hash.append("rc("sv);
            hash.append(m_swiftFullName);
            hash.append(";"sv);
            default_interface->append_signature(hash);
            hash.append(")"sv);
        }

        virtual void write_c_forward_declaration(writer& w) const override;
        virtual void write_c_abi_param(writer& w) const override;

        void write_c_definition(writer& w) const;

        std::vector<named_interface_info> required_interfaces;
        std::vector<std::pair<interface_type const*, version>> supplemental_fast_interfaces;
        class_type const* base_class = nullptr;
        metadata_type const* default_interface = nullptr;
        std::map<std::string, attributed_type> factories;

        bool is_composable() const;

    private:
        std::string_view m_abiNamespace;
    };

    struct generic_inst final : metadata_type
    {
        generic_inst(typedef_base const* genericType, std::vector<metadata_type const*> genericParams) :
            m_genericType(genericType),
            m_genericParams(std::move(genericParams))
        {
            m_swiftFullName = genericType->swift_full_name();
            m_swiftFullName.push_back('<');

            m_mangledName = genericType->mangled_name();
            m_swiftTypeName = genericType->swift_type_name();
            m_swiftTypeName.push_back('<');

            std::string_view prefix;
            for (auto param : m_genericParams)
            {
                m_swiftFullName += prefix;
                m_swiftFullName += param->swift_full_name();

                m_swiftTypeName += prefix;
                // we use the param full name because short type names could conflict (i.e. Microsoft.UI.Input.PointerPoint/Windows.UI.Input.PointerPoint)
                m_swiftTypeName += param->swift_full_name();

                m_mangledName.push_back('_');
                m_mangledName += param->generic_param_mangled_name();

                prefix = ", ";
            }

            m_swiftFullName.push_back('>');
            m_swiftTypeName.push_back('>');
        }

        virtual std::string_view swift_abi_namespace() const override
        {
            return m_genericType->swift_abi_namespace();
        }

        virtual std::string_view swift_logical_namespace() const override
        {
            return m_genericType->swift_logical_namespace();
        }

        virtual std::string_view swift_full_name() const override
        {
            return m_swiftFullName;
        }

        virtual std::string_view swift_type_name() const override
        {
            return m_swiftTypeName;
        }

        virtual std::string_view cpp_abi_name() const override
        {
            return m_mangledName;
        }

        virtual std::string_view cpp_logical_name() const override
        {
            return m_mangledName;
        }

        virtual std::string_view mangled_name() const override
        {
            return m_mangledName;
        }

        virtual std::string_view generic_param_mangled_name() const override
        {
            return m_mangledName;
        }

        virtual void append_signature(sha1& hash) const override;

        virtual void write_c_forward_declaration(writer& w) const override;
        virtual void write_c_abi_param(writer& w) const override;

        virtual bool is_experimental() const override
        {
            // Generic instances are experimental only if their arguments are experimental
            for (auto ptr : m_genericParams)
            {
                if (ptr->is_experimental())
                {
                    return true;
                }
            }

            return false;
        }

        typedef_base const* generic_type() const noexcept
        {
            return m_genericType;
        }

        winmd::reader::category category() const noexcept
        {
            return m_genericType->category();
        }

        std::optional<deprecation_info> is_deprecated() const noexcept
        {
            return m_genericType->is_deprecated();
        }

        std::string_view generic_type_abi_name() const noexcept
        {
            // Generic type swift names end with "`N" where 'N' is the number of generic parameters
            auto result = m_genericType->cpp_abi_name();
            auto tickPos = result.rfind('`');
            XLANG_ASSERT(tickPos != std::string_view::npos);
            return result.substr(0, tickPos);
        }

        std::vector<metadata_type const*> const& generic_params() const noexcept
        {
            return m_genericParams;
        }

        std::vector<generic_inst const*> dependencies;
        std::vector<function_def> functions;
        std::vector<property_def> properties;
        std::vector<event_def> events;

        virtual void write_swift_declaration(writer&) const override;

        std::vector<named_interface_info> required_interfaces;
    private:

        typedef_base const* m_genericType;
        std::vector<metadata_type const*> m_genericParams;
        std::string m_swiftFullName;
        std::string m_swiftTypeName;
        std::string m_mangledName;
    };

    // Helpers
    inline bool is_generic_inst(const metadata_type* type)
    {
        return dynamic_cast<const generic_inst*>(type) != nullptr;
    }

    inline bool is_generic_inst(metadata_type const& type)
    {
        return is_generic_inst(&type);
    }

    inline bool is_generic_def(const metadata_type* type)
    {
        auto typedefBase = dynamic_cast<const typedef_base*>(type);
        return typedefBase != nullptr && typedefBase->is_generic();
    }

    inline bool is_generic_def(metadata_type const& type)
    {
        return is_generic_def(&type);
    }

    inline bool is_delegate(metadata_type const* type, bool allow_generic = true)
    {
        if (allow_generic)
        {
            if (auto genericInst = dynamic_cast<generic_inst const*>(type))
            {
                return is_delegate(genericInst->generic_type());
            }
        }

        return dynamic_cast<delegate_type const*>(type) != nullptr;
    }

    inline bool is_delegate(metadata_type const& type, bool allow_generic = true)
    {
        return is_delegate(&type, allow_generic);
    }

    inline bool is_interface(metadata_type const* type, bool allow_generic = true)
    {
        if (allow_generic)
        {
            if (auto geninst = dynamic_cast<generic_inst const*>(type))
            {
                return is_interface(geninst->generic_type());
            }
        }

        return dynamic_cast<interface_type const*>(type) != nullptr;
    }

    inline bool is_interface(metadata_type const& type, bool allow_generic = true)
    {
        return is_interface(&type, allow_generic);
    }

    inline bool is_class(metadata_type const* type)
    {
        // Classes cannot be generic in WinRT
        return dynamic_cast<class_type const*>(type) != nullptr;
    }

    inline bool is_reference_type(metadata_type const* type)
    {
        if (is_class(type) || is_interface(type) || is_delegate(type))
        {
            return true;
        }

        if (auto elem = dynamic_cast<element_type const*>(type))
        {
            return elem->type() == ElementType::Object;
        }
        return false;
    }

    inline bool is_element_type(metadata_type const* type, ElementType elementType)
    {
        if (auto elem = dynamic_cast<element_type const*>(type))
        {
            return elem->type() == elementType;
        }
        return false;
    }

    inline bool is_boolean(metadata_type const* signature)
    {
        return is_element_type(signature, ElementType::Boolean);
    }

    inline bool is_floating_point(metadata_type const* signature)
    {
        if (auto elementType = dynamic_cast<element_type const*>(signature))
        {
            return elementType->type() == ElementType::R4 ||
                elementType->type() == ElementType::R8;
        }
        return false;
    }

    inline bool is_string(metadata_type const* signature)
    {
        return is_element_type(signature, ElementType::String);
    }

    // Helpers for WinRT types
    constexpr std::string_view system_namespace = "System";
    constexpr std::string_view winrt_foundation_namespace = "Windows.Foundation";
    constexpr std::string_view winrt_collections_namespace = "Windows.Foundation.Collections";

    inline bool is_winrt_ireference(metadata_type const& type)
    {
        return type.swift_full_name().starts_with("Windows.Foundation.IReference");
    }

    inline bool is_winrt_ireference(const metadata_type* type)
    {
        return is_winrt_ireference(*type);
    }

    inline bool is_winrt_eventhandler(metadata_type const& type)
    {
        return type.swift_full_name().starts_with("Windows.Foundation.EventHandler");
    }

    inline bool is_winrt_eventhandler(const metadata_type* type)
    {
        return is_winrt_eventhandler(*type);
    }

    inline bool is_winrt_typedeventhandler(metadata_type const& type)
    {
        return type.swift_full_name().starts_with("Windows.Foundation.TypedEventHandler");
    }

    inline bool is_winrt_typedeventhandler(const metadata_type* type)
    {
        return is_winrt_typedeventhandler(*type);
    }

    inline bool is_winrt_async_result_type(metadata_type const& type)
    {
        if (type.swift_logical_namespace() == winrt_foundation_namespace)
        {
            auto type_name = type.swift_type_name();

            return type_name == "IAsyncAction" ||
                type_name.starts_with("IAsyncOperation");
        }
        return false;
    }

    inline bool is_winrt_generic_collection(metadata_type const& type)
    {
        if (type.swift_logical_namespace() == winrt_collections_namespace)
        {
            auto type_name = type.swift_type_name();

            // Add backtick to ensure we don't match with interfaces like IVectorChangedEventArgs
            return type_name.starts_with("IVector`") ||
                type_name.starts_with("IVectorView`") ||
                type_name.starts_with("IMap`") ||
                type_name.starts_with("IMapView`") ||
                type_name.starts_with("IIterator`") ||
                type_name.starts_with("IIterable`") ||
                type_name.starts_with("IObservableMap`") ||
                type_name.starts_with("IObservableVector`");
        }
        return false;
    }

    inline bool is_winrt_generic_collection(const metadata_type* type)
    {
        return is_winrt_generic_collection(*type);
    }

    inline bool needs_collection_conformance(metadata_type const& type)
    {
        if (type.swift_logical_namespace() == winrt_collections_namespace)
        {
            auto type_name = type.swift_type_name();

            // Add backtick to ensure we don't match with interfaces like IVectorChangedEventArgs
            return type_name.starts_with("IVector`") ||
                type_name.starts_with("IVectorView`") ||
                type_name.starts_with("IObservableVector`");
        }
        return false;
    }

    inline bool needs_collection_conformance(const metadata_type* type)
    {
        return needs_collection_conformance(*type);
    }
}
