#pragma once
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "winmd_reader.h"
#include "attributes.h"

namespace swiftwinrt
{
    constexpr std::string_view metadata_namespace = "Windows.Foundation.Metadata";

    using namespace std::filesystem;
    using namespace winmd::reader;
    using namespace std::literals;

    std::string get_full_type_name(TypeDef const& type);
    std::string get_full_type_name(TypeRef const& type);
    bool is_generic(TypeDef const& type) noexcept;
    bool starts_with(std::string_view value, std::string_view match) noexcept;

    template <typename...T> struct visit_overload : T... { using T::operator()...; };

    template <typename V, typename...C>
    inline auto call(V&& variant, C&&...call)
    {
        return std::visit(visit_overload<C...>{ std::forward<C>(call)... }, std::forward<V>(variant));
    }

    template <typename T>
    inline bool has_attribute(T const& row, std::string_view const& type_namespace, std::string_view const& type_name)
    {
        return static_cast<bool>(get_attribute(row, type_namespace, type_name));
    }

    coded_index<TypeDefOrRef> get_default_interface(TypeDef const& type);
    bool is_exclusive(TypeDef const& type);
    bool is_default(InterfaceImpl const& ifaceImpl);
    bool is_default(TypeDef const& type);
    bool can_mark_internal(TypeDef const& type);
    coded_index<TypeDefOrRef> get_default_interface(TypeSig const& type);


    enum class param_category
    {
        generic_type,
        generic_type_index,
        object_type,
        string_type,
        character_type,
        boolean_type,
        enum_type,
        struct_type,
        array_type,
        fundamental_type,
        guid_type
    };

    param_category get_category(TypeSig const& signature, TypeDef* signature_type = nullptr);

    bool is_generic(TypeRef const& ref);
    bool is_generic(TypeSig const& sig);
    bool is_generic(coded_index<TypeDefOrRef> const& type);

    bool is_floating_point(TypeSig const& signature);
    bool is_boolean(TypeSig const& signature);
    bool is_object(TypeSig const& signature);

    bool is_guid(param_category category);
    bool is_category_type(TypeSig const& signature, category category);
    bool is_interface(TypeSig const& signature);
    bool is_delegate(TypeSig const& signature);
    bool is_class(TypeSig const& signature);

    bool is_interface(TypeDef const& type);
    bool is_delegate(TypeDef const& type);
    bool is_class(TypeDef const& type);

    bool is_struct(TypeSig const& signature);
    bool is_type_blittable(param_category category);
    bool is_type_blittable(TypeSig const& signature, bool for_array = false);
    bool is_type_blittable(TypeDef const& type);
    bool is_struct_blittable(TypeDef const& type);

    bool is_static(TypeDef const& type);
    bool is_static(MethodDef const& method);
}
