#pragma once
#include <chrono>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>
#include <algorithm>
#include <cctype>

#include "winmd_reader.h"
#include "attributes.h"
#include "versioning.h"
#include "types.h"
#include "metadata_cache.h"
#include "type_writers.h"
#include "settings.h"
namespace swiftwinrt
{
    template <typename T>
    bool has_attribute(T const& row, std::string_view const& type_namespace, std::string_view const& type_name);

    std::chrono::high_resolution_clock::time_point get_start_time();
    std::chrono::milliseconds get_elapsed_time(std::chrono::high_resolution_clock::time_point const& start);

    struct type_name
    {
        std::string_view name;
        std::string_view name_space;

        explicit type_name(TypeDef const& type) :
            name(type.TypeName()),
            name_space(type.TypeNamespace())
        {
        }

        explicit type_name(TypeRef const& type) :
            name(type.TypeName()),
            name_space(type.TypeNamespace())
        {
        }

        explicit type_name(coded_index<TypeDefOrRef> const& type)
        {
            auto const& [type_namespace, type_name] = get_namespace_and_name(type);
            name_space = type_namespace;
            name = type_name;
        }

        explicit type_name(metadata_type const& type)
        {
            construct(&type);
        }

        explicit type_name(metadata_type const* type)
        {
            construct(type);
        }

        explicit type_name(typedef_base const& type)
        {
            construct(&type);
        }

        explicit type_name(typedef_base const* type)
        {
            construct(type);
        }

        // Same as winmd::reader::get_type_namespace_and_name, but also handles TypeSpecs
        static inline std::pair<std::string_view, std::string_view> get_namespace_and_name(coded_index<TypeDefOrRef> const& type)
        {
            if (type.type() == TypeDefOrRef::TypeDef)
            {
                auto const def = type.TypeDef();
                return { def.TypeNamespace(), def.TypeName() };
            }
            else if (type.type() == TypeDefOrRef::TypeRef)
            {
                auto const ref = type.TypeRef();
                return { ref.TypeNamespace(), ref.TypeName() };
            }
            else
            {
                XLANG_ASSERT(type.type() == TypeDefOrRef::TypeSpec);
                auto generic_type = type.TypeSpec().Signature().GenericTypeInst().GenericType();
                return get_namespace_and_name(generic_type);
            }
        }

    private:
        void construct(metadata_type const* type)
        {
            // Check if this is a typedef_base, otherwise we could fail trying to get the swift_abi_namespace
            // for static classes where there is no default interface
            if (auto typedefbase = dynamic_cast<const typedef_base*>(type))
            {
                construct(typedefbase);
            }
            else
            {
                name_space = type->swift_abi_namespace();
                name = type->swift_type_name();
            }
        }

        void construct(typedef_base const* type)
        {
            name_space = type->type().TypeNamespace();
            name = type->type().TypeName();
        }
    };

    bool operator==(type_name const& left, type_name const& right);
    bool operator==(type_name const& left, std::string_view right);

    bool is_exclusive(interface_type const& type);
    TypeDef find_type(coded_index<winmd::reader::TypeDefOrRef> type);
    MethodDef get_delegate_method(TypeDef const& type);
    std::string get_component_filename(TypeDef const& type);
    std::string get_generated_component_filename(TypeDef const& type);
    bool is_overridable(InterfaceImpl const& iface);
    bool has_projected_types(cache::namespace_members const& members);
    TypeDef get_exclusive_to(TypeDef const& type);
    TypeDef get_exclusive_to(typedef_base const& type);
    bool is_exclusive(typedef_base const& type);
    const class_type* try_get_exclusive_to(writer& w, typedef_base const& type);
    std::optional<attributed_type> try_get_factory_info(writer& w, typedef_base const& type);
    std::tuple<MethodDef, MethodDef> get_property_methods(Property const& prop);
    std::string get_swift_name(interface_info const& iface);
    std::string put_in_backticks_if_needed(std::string name);
    std::string to_camel_case(std::string_view name);
    std::string get_swift_name(MethodDef const& method);
    std::string get_swift_member_name(std::string_view name);
    std::string get_swift_name(Property const& property);
    std::string get_swift_name(Event const& event);
    std::string get_swift_name(Field const& field);
    std::string get_swift_name(Param const& param);
    std::string local_swift_param_name(std::string const& param_name);
    std::string local_swift_param_name(std::string_view param_name);
    std::string local_swift_param_name(function_param const& param);
    std::string get_swift_name(function_param const& param);
    std::string get_swift_name(function_return_type const& return_type);
    std::string get_swift_name(property_def const& property);
    std::string get_swift_name(function_def const& function);
    std::string get_swift_name(struct_member const& member);
    std::string_view get_abi_name(struct_member const& member);
    std::string_view remove_backtick(std::string_view name);
    std::string internal_namepace(std::string prefix, std::string_view ns);
    std::string abi_namespace(std::string_view ns);
    std::string impl_namespace(std::string_view ns);
    std::string abi_namespace(TypeDef const& type);
    std::string abi_namespace(metadata_type const& type);
    std::string abi_namespace(metadata_type const* type);
    winmd::reader::ElementType underlying_enum_type(winmd::reader::TypeDef const& type);
    std::array<char, 37> type_iid(winmd::reader::TypeDef const& type);
    param_category get_category(metadata_type const* type, TypeDef* signature_type = nullptr);
    std::string_view get_full_type_name(generic_inst const& type);
    bool is_struct_blittable(struct_type const& type);
    bool is_struct(metadata_type const& type);
    bool needs_wrapper(param_category category);
    bool is_overridable(metadata_type const& type);

    struct separator
    {
        writer& w;
        std::string s = ", ";
        bool first{ true };

        void operator()()
        {
            if (first)
            {
                first = false;
            }
            else
            {
                w.write(s);
            }
        }
    };

    std::string_view get_abi_name(winmd::reader::MethodDef const& method);
    std::string_view get_abi_name(function_def const& method);

    std::string_view get_name(winmd::reader::MethodDef const& method);
    std::string_view get_name(function_def const& method);

    bool is_remove_overload(winmd::reader::MethodDef const& method);
    bool is_add_overload(winmd::reader::MethodDef const& method);
    bool is_get_overload(winmd::reader::MethodDef const& method);
    bool is_put_overload(winmd::reader::MethodDef const& method);

    bool is_noexcept(winmd::reader::MethodDef const& method);
    bool is_noexcept(metadata_type const& type, function_def const& method);

    bool has_fastabi(winmd::reader::TypeDef const& type);
    bool is_always_disabled(winmd::reader::TypeDef const& type);
    bool is_always_enabled(winmd::reader::TypeDef const& type);

    winmd::reader::TypeDef get_base_class(winmd::reader::TypeDef const& derived);
    std::vector<winmd::reader::TypeDef> get_bases(winmd::reader::TypeDef const& type);

    template <typename T>
    inline bool is_composable(T const& type)
    {
        return has_attribute(type, "Windows.Foundation.Metadata", "ComposableAttribute");
    }

    TypeDef find_type(coded_index<winmd::reader::TypeDefOrRef> type);

    template <typename T>
    inline bool is_experimental(T const& value)
    {
        using namespace std::literals;
        return static_cast<bool>(get_attribute(value, metadata_namespace, "ExperimentalAttribute"sv));
    }

    template <typename T>
    inline std::optional<deprecation_info> is_deprecated(T const& type)
    {
        using namespace std::literals;

        auto attr = get_attribute(type, metadata_namespace, "DeprecatedAttribute"sv);
        if (!attr)
        {
            return std::nullopt;
        }

        auto sig = attr.Value();
        auto const& fixedArgs = sig.FixedArgs();

        // There are three DeprecatedAttribute constructors, two of which deal with version numbers which we don't care
        // about here. The third is relative to a contract version, which we _do_ care about
        if ((fixedArgs.size() != 4))
        {
            return std::nullopt;
        }

        auto const& contractSig = std::get<ElemSig>(fixedArgs[3].value);
        if (!std::holds_alternative<std::string_view>(contractSig.value))
        {
            return std::nullopt;
        }

        return deprecation_info
        {
            std::get<std::string_view>(contractSig.value),
            std::get<std::uint32_t>(std::get<ElemSig>(fixedArgs[2].value).value),
            std::get<std::string_view>(std::get<ElemSig>(fixedArgs[0].value).value)
        };
    }
