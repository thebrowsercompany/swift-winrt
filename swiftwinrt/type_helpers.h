#pragma once

namespace swiftwinrt
{
    using namespace std::filesystem;
    using namespace winmd::reader;
    using namespace std::literals;

    inline bool starts_with(std::string_view const& value, std::string_view const& match) noexcept
    {
        return 0 == value.compare(0, match.size(), match);
    }

    template <typename...T> struct visit_overload : T... { using T::operator()...; };

    template <typename V, typename...C>
    auto call(V&& variant, C&&...call)
    {
        return std::visit(visit_overload<C...>{ std::forward<C>(call)... }, std::forward<V>(variant));
    }

    template <typename T>
    bool has_attribute(T const& row, std::string_view const& type_namespace, std::string_view const& type_name)
    {
        return static_cast<bool>(get_attribute(row, type_namespace, type_name));
    }

    static coded_index<TypeDefOrRef> get_default_interface(TypeDef const& type)
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

    static coded_index<TypeDefOrRef> get_default_interface(TypeSig const& type)
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
            auto const& [type_namespace, type_name] = get_type_namespace_and_name(type);
            name_space = type_namespace;
            name = type_name;
        }
    };

    bool operator==(type_name const& left, type_name const& right)
    {
        return left.name == right.name && left.name_space == right.name_space;
    }

    bool operator==(type_name const& left, std::string_view const& right)
    {
        if (left.name.size() + 1 + left.name_space.size() != right.size())
        {
            return false;
        }

        if (right[left.name_space.size()] != '.')
        {
            return false;
        }

        if (0 != right.compare(left.name_space.size() + 1, left.name.size(), left.name))
        {
            return false;
        }

        return 0 == right.compare(0, left.name_space.size(), left.name_space);
    }

    enum class param_category
    {
        generic_type,
        object_type,
        string_type,
        enum_type,
        struct_type,
        array_type,
        fundamental_type,
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

                    if (type_name(type_ref) == "System.Guid")
                    {
                        result = param_category::struct_type;
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
                    result = param_category::struct_type;
                    return;
                case category::enum_type:
                    result = param_category::enum_type;
                    return;
                }
            },
                [&](GenericTypeInstSig const&)
            {
                result = param_category::object_type;
            },
                [&](auto&&)
            {
                result = param_category::generic_type;
            });

        return result;
    }

    static bool is_floating_point(TypeSig const& signature)
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

    static bool is_object(TypeSig const& signature)
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

    static bool is_category_type(TypeSig const& signature, category category)
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

                    if (type_name(type_ref) == "System.Guid")
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

    static bool is_interface(TypeSig const& signature)
    {
        return is_category_type(signature, category::interface_type);
    }

    static bool is_delegate(TypeSig const& signature)
    {
        return is_category_type(signature, category::delegate_type);
    }

    static bool is_class(TypeSig const& signature)
    {
        return is_category_type(signature, category::class_type);
    }

    static bool is_struct(TypeSig const& signature)
    {
        return is_category_type(signature, category::struct_type);
    }

    bool is_type_blittable(TypeSig const& signature, bool for_array = false)
    {
        if (signature.is_szarray())
        {
            return false;
        }

        return call(signature.Type(),
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
                        return true;
                    case ElementType::Enum:
                        return !for_array;
                    case ElementType::Boolean:
                    case ElementType::String:
                    case ElementType::GenericInst:
                    case ElementType::Char:
                        return false;
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
                    type_def = find_required(type.TypeRef());
                }

                switch (get_category(type_def))
                {
                case category::interface_type:
                case category::class_type:
                case category::delegate_type:
                    return false;
                case category::struct_type:
                    for (auto&& field : type_def.FieldList())
                    {
                        if (!is_type_blittable(field.Signature().Type()))
                        {
                            return false;
                        }
                    }
                    return true;
                case category::enum_type:
                    return !for_array;
                }
            },
                [&](GenericTypeInstSig const&)
            {
                return false;
            },
                [&](auto&&)
            {
                return true;
            });

    }
}