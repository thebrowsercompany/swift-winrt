#pragma once
#include <variant>

namespace swiftwinrt
{
    constexpr std::string_view metadata_namespace = "Windows.Foundation.Metadata";

    using namespace std::filesystem;
    using namespace winmd::reader;
    using namespace std::literals;

    std::string get_full_type_name(TypeDef const& type);
    std::string get_full_type_name(TypeRef const& type);
    bool is_generic(TypeDef const& type) noexcept;
    inline bool starts_with(std::string_view const& value, std::string_view const& match) noexcept
    {
        return 0 == value.compare(0, match.size(), match);
    }

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

    inline coded_index<TypeDefOrRef> get_default_interface(TypeDef const& type)
    {
        auto impls = type.InterfaceImpl();

        for (auto&& impl : impls)
        {
            if (has_attribute(impl, "Windows.Foundation.Metadata", "DefaultAttribute"))
            {
                return impl.Interface();
            }
        }

        if (!empty(impls))
        {
            throw_invalid("Type '", type.TypeNamespace(), ".", type.TypeName(), "' does not have a default interface");
        }

        return {};
    }

    inline bool is_exclusive(TypeDef const& type)
    {
        return has_attribute(type, "Windows.Foundation.Metadata", "ExclusiveToAttribute");
    }

    inline bool is_default(InterfaceImpl const& ifaceImpl)
    {
        return has_attribute(ifaceImpl, "Windows.Foundation.Metadata", "DefaultAttribute");
    }

    inline bool is_default(TypeDef const& type)
    {
        return has_attribute(type, "Windows.Foundation.Metadata", "DefaultAttribute");
    }

    inline bool can_mark_internal(TypeDef const& type)
    {
        if (has_attribute(type, metadata_namespace, "ActivatableAttribute"))
        {
            return true;
        }
        if (has_attribute(type, metadata_namespace, "StaticAttribute"))
        {
            return true;
        }
        // generic types are always internal since they are duplicated at each use
        if (is_generic(type))
        {
            return true;
        }
        return false;
    }

    inline coded_index<TypeDefOrRef> get_default_interface(TypeSig const& type)
    {
        coded_index<TypeDefOrRef> result = {};
        call(type.Type(),
            [&](coded_index<TypeDefOrRef> const& type)
            {
                TypeDef type_def;

                if (type.type() == TypeDefOrRef::TypeDef)
                {
                    type_def = type.TypeDef();
                }
                else
                {
                    auto type_ref = type.TypeRef();
                    type_def = find_required(type_ref);
                }

                result = get_default_interface(type_def);
            }, [](auto&&) {});

        return result;
    }


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

    inline param_category get_category(TypeSig const& signature, TypeDef* signature_type = nullptr)
    {
        if (signature.is_szarray())
        {
            return param_category::array_type;
        }

        param_category result{};

        call(signature.Type(),
            [&](ElementType type)
            {
                if (type == ElementType::String)
                {
                    result = param_category::string_type;
                }
                else if (type == ElementType::Object)
                {
                    result = param_category::object_type;
                }
                else if (type == ElementType::Char)
                {
                    result = param_category::character_type;
                }
                else if (type == ElementType::Boolean)
                {
                    result = param_category::boolean_type;
                }
                else
                {
                    result = param_category::fundamental_type;
                }
            },
            [&](coded_index<TypeDefOrRef> const& type)
            {
                TypeDef type_def;

                if (type.type() == TypeDefOrRef::TypeDef)
                {
                    type_def = type.TypeDef();
                }
                else
                {
                    auto type_ref = type.TypeRef();

                    if (get_full_type_name(type_ref) == "System.Guid")
                    {
                        result = param_category::guid_type;
                        return;
                    }

                    type_def = find_required(type_ref);
                }

                if (signature_type)
                {
                    *signature_type = type_def;
                }

                switch (get_category(type_def))
                {
                case category::interface_type:
                case category::class_type:
                case category::delegate_type:
                    result = param_category::object_type;
                    return;
                case category::struct_type:
                    // HRESULT maps to Int32 due to C import
                    if (get_full_type_name(type_def) == "Windows.Foundation.HResult")
                    {
                        result = param_category::fundamental_type;
                    }
                    else
                    {
                        result = param_category::struct_type;
                    }
                    return;
                case category::enum_type:
                    result = param_category::enum_type;
                    return;
                }
            },
                [&](GenericTypeInstSig const&)
            {
                result = param_category::generic_type;
            },
            [&](GenericTypeIndex const&)
            {
                result = param_category::generic_type_index;
            },
                [&](auto&&)
            {
                result = param_category::object_type;
            });

        return result;
    }

    inline bool is_generic(TypeDef const& type) noexcept
    {
        return distance(type.GenericParam()) != 0;
    }

    inline bool is_generic(TypeRef const& ref)
    {
        return is_generic(find_required(ref));
    }

    inline bool is_generic(TypeSig const& sig)
    {
        return get_category(sig) == param_category::generic_type;
    }

    inline bool is_generic(coded_index<TypeDefOrRef> const& type)
    {
        switch (type.type())
        {
        case TypeDefOrRef::TypeSpec:
            return true;
        case TypeDefOrRef::TypeRef:
            return is_generic(type.TypeRef());
        case TypeDefOrRef::TypeDef:
            return is_generic(type.TypeDef());
        default:
            return false;
        }
    }

    inline bool is_floating_point(TypeSig const& signature)
    {
        bool object{};

        call(signature.Type(),
            [&](ElementType type)
            {
                if (type == ElementType::R4 || type == ElementType::R8)
                {
                    object = true;
                }
            },
            [](auto&&) {});

        return object;
    }

    inline bool is_boolean(TypeSig const& signature)
    {
        return get_category(signature) == param_category::boolean_type;
    }

    inline bool is_object(TypeSig const& signature)
    {
        bool object{};

        call(signature.Type(),
            [&](ElementType type)
            {
                if (type == ElementType::Object)
                {
                    object = true;
                }
            },
            [](auto&&) {});

        return object;
    }

    inline bool is_guid(param_category category)
    {
        return category == param_category::guid_type;
    }

    inline bool is_category_type(TypeSig const& signature, category category)
    {
        bool is_match{};

        call(signature.Type(),
            [&](coded_index<TypeDefOrRef> const& type)
            {
                TypeDef type_def;

                if (type.type() == TypeDefOrRef::TypeDef)
                {
                    type_def = type.TypeDef();
                }
                else
                {
                    auto type_ref = type.TypeRef();

                    if (get_full_type_name(type_ref) == "System.Guid")
                    {
                        is_match = category == category::struct_type;
                        return;
                    }

                    type_def = find_required(type_ref);
                }

                if (get_category(type_def) == category)
                {
                    is_match = true;
                }
            },
            [](auto&&) {});

        return is_match;
    }

    inline bool is_interface(TypeSig const& signature)
    {
        return is_category_type(signature, category::interface_type);
    }

    inline bool is_delegate(TypeSig const& signature)
    {
        return is_category_type(signature, category::delegate_type);
    }

    inline bool is_class(TypeSig const& signature)
    {
        return is_category_type(signature, category::class_type);
    }

    inline bool is_interface(TypeDef const& type)
    {
        if (type == TypeDef{}) return false;
        return get_category(type) == category::interface_type;
    }

    inline bool is_delegate(TypeDef const& type)
    {
        if (type == TypeDef{}) return false;
        return get_category(type) == category::delegate_type;
    }

    inline bool is_class(TypeDef const& type)
    {
        if (type == TypeDef{}) return false;
        return get_category(type) == category::class_type;
    }

    inline bool is_struct(TypeSig const& signature)
    {
        // use get_category bc this does a more detailed check of what is actually a struct type.
        // the winmd reader seems to imply that any value type is a struct
        return get_category(signature) == param_category::struct_type;
    }

    inline bool is_type_blittable(param_category category)
    {
        // generic types should be unwrapped first before calling this API
        assert(category != param_category::generic_type_index);

        switch (category) {
        case param_category::enum_type:
        case param_category::fundamental_type:
        case param_category::guid_type:
            return true;
        default:
            return false;
        }
    }
    inline bool is_type_blittable(TypeSig const& signature, bool for_array = false)
    {
        if (signature.is_szarray())
        {
            return false;
        }
        bool result = true;
        call(signature.Type(),
            [&](ElementType type)
            {
                switch (type)
                {
                    case ElementType::I1:
                    case ElementType::I2:
                    case ElementType::I4:
                    case ElementType::I8:
                    case ElementType::U1:
                    case ElementType::U2:
                    case ElementType::U4:
                    case ElementType::U8:
                        result = true;
                        return;
                    case ElementType::Enum:
                        result = !for_array;
                        return;
                    case ElementType::Boolean:
                    case ElementType::String:
                    case ElementType::GenericInst:
                    case ElementType::Char:
                    case ElementType::Object:
                        result = false;
                        return;
                }
            },
            [&](coded_index<TypeDefOrRef> const& type)
            {
                TypeDef type_def;

                if (type.type() == TypeDefOrRef::TypeDef)
                {
                    type_def = type.TypeDef();
                }
                else
                {
                    auto type_ref = type.TypeRef();

                    if (get_full_type_name(type_ref) == "System.Guid")
                    {
                        result = true;
                        return;
                    }
                    type_def = find_required(type.TypeRef());
                }

                switch (get_category(type_def))
                {
                case category::interface_type:
                case category::class_type:
                case category::delegate_type:
                    result = false;
                    return;
                case category::struct_type:
                    for (auto&& field : type_def.FieldList())
                    {
                        if (!is_type_blittable(field.Signature().Type()))
                        {
                            result = false;
                            return;
                        }
                    }
                    result = true;
                    return;
                case category::enum_type:
                    result = !for_array;
                    return;
                }
            },
                [&](GenericTypeInstSig const&)
            {
                result = false;
                return;
            },
                [&](auto&&)
            {
                result = true;
                return;
            });
        return result;
    }

    inline bool is_type_blittable(TypeDef const& type)
    {
        if (type == TypeDef{}) return false;
        switch (get_category(type))
        {
        case category::interface_type:
        case category::class_type:
        case category::delegate_type:
            return false;
        case category::struct_type:
            for (auto&& field : type.FieldList())
            {
                if (!is_type_blittable(field.Signature().Type()))
                {
                    return false;
                }
            }
            return true;
        case category::enum_type:
            return true;
        }
    }

    inline bool is_struct_blittable(TypeDef const& type)
    {
        assert(get_category(type) == category::struct_type);
        for (auto&& field : type.FieldList())
        {
            if (!is_type_blittable(field.Signature().Type()))
            {
                return false;
            }
        }

        return true;
    }

    inline bool is_static(TypeDef const& type)
    {
        return get_category(type) == category::class_type && type.Flags().Abstract();
    }

    inline bool is_static(MethodDef const& method)
    {
        return method.Flags().Static();
    }
}