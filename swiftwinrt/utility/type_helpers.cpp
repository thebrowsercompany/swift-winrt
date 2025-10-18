#include "pch.h"

#include "utility/type_helpers.h"

#include <cassert>

#include "utility/metadata_helpers.h"

using namespace winmd::reader;

namespace swiftwinrt
{
    std::string get_full_type_name(TypeDef const& type)
    {
        type_name name(type);
        std::string result;
        result.reserve(name.name_space.length() + name.name.length() + 1);
        result += name.name_space;
        result += '.';
        result += name.name;
        return result;
    }

    std::string get_full_type_name(TypeRef const& type)
    {
        type_name name(type);
        std::string result;
        result.reserve(name.name_space.length() + name.name.length() + 1);
        result += name.name_space;
        result += '.';
        result += name.name;
        return result;
    }

    bool is_generic(TypeDef const& type) noexcept
    {
        return type.GenericParam().First() != type.GenericParam().Last();
    }

    bool starts_with(std::string_view value, std::string_view match) noexcept
    {
        return value.compare(0, match.size(), match) == 0;
    }

    coded_index<TypeDefOrRef> get_default_interface(TypeDef const& type)
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

    bool is_exclusive(TypeDef const& type)
    {
        return has_attribute(type, "Windows.Foundation.Metadata", "ExclusiveToAttribute");
    }

    bool is_default(InterfaceImpl const& ifaceImpl)
    {
        return has_attribute(ifaceImpl, "Windows.Foundation.Metadata", "DefaultAttribute");
    }

    bool is_default(TypeDef const& type)
    {
        return has_attribute(type, "Windows.Foundation.Metadata", "DefaultAttribute");
    }

    bool can_mark_internal(TypeDef const& type)
    {
        if (has_attribute(type, metadata_namespace, "ActivatableAttribute"))
        {
            return true;
        }
        if (has_attribute(type, metadata_namespace, "StaticAttribute"))
        {
            return true;
        }
        if (is_generic(type))
        {
            return true;
        }
        return false;
    }

    coded_index<TypeDefOrRef> get_default_interface(TypeSig const& type)
    {
        coded_index<TypeDefOrRef> result{};
        call(type.Type(),
            [&](coded_index<TypeDefOrRef> const& type_arg)
            {
                TypeDef type_def;

                if (type_arg.type() == TypeDefOrRef::TypeDef)
                {
                    type_def = type_arg.TypeDef();
                }
                else
                {
                    auto type_ref = type_arg.TypeRef();
                    type_def = find_required(type_ref);
                }

                result = get_default_interface(type_def);
            }, [](auto&&) {});

        return result;
    }

    param_category get_category(TypeSig const& signature, TypeDef* signature_type)
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
            [&](coded_index<TypeDefOrRef> const& type_arg)
            {
                TypeDef type_def;

                if (type_arg.type() == TypeDefOrRef::TypeDef)
                {
                    type_def = type_arg.TypeDef();
                }
                else
                {
                    auto type_ref = type_arg.TypeRef();

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

    bool is_generic(TypeRef const& ref)
    {
        return is_generic(find_required(ref));
    }

    bool is_generic(TypeSig const& sig)
    {
        return get_category(sig) == param_category::generic_type;
    }

    bool is_generic(coded_index<TypeDefOrRef> const& type)
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

    bool is_floating_point(TypeSig const& signature)
    {
        bool result{};

        call(signature.Type(),
            [&](ElementType type)
            {
                if (type == ElementType::R4 || type == ElementType::R8)
                {
                    result = true;
                }
            },
            [](auto&&) {});

        return result;
    }

    bool is_boolean(TypeSig const& signature)
    {
        return get_category(signature) == param_category::boolean_type;
    }

    bool is_object(TypeSig const& signature)
    {
        bool result{};

        call(signature.Type(),
            [&](ElementType type)
            {
                if (type == ElementType::Object)
                {
                    result = true;
                }
            },
            [](auto&&) {});

        return result;
    }

    bool is_guid(param_category category_value)
    {
        return category_value == param_category::guid_type;
    }

    bool is_category_type(TypeSig const& signature, category category_value)
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
                        is_match = category_value == category::struct_type;
                        return;
                    }

                    type_def = find_required(type_ref);
                }

                if (get_category(type_def) == category_value)
                {
                    is_match = true;
                }
            },
            [](auto&&) {});

        return is_match;
    }

    bool is_interface(TypeSig const& signature)
    {
        return is_category_type(signature, category::interface_type);
    }

    bool is_delegate(TypeSig const& signature)
    {
        return is_category_type(signature, category::delegate_type);
    }

    bool is_class(TypeSig const& signature)
    {
        return is_category_type(signature, category::class_type);
    }

    bool is_interface(TypeDef const& type)
    {
        if (type == TypeDef{})
        {
            return false;
        }

        return get_category(type) == category::interface_type;
    }

    bool is_delegate(TypeDef const& type)
    {
        if (type == TypeDef{})
        {
            return false;
        }

        return get_category(type) == category::delegate_type;
    }

    bool is_class(TypeDef const& type)
    {
        if (type == TypeDef{})
        {
            return false;
        }

        return get_category(type) == category::class_type;
    }

    bool is_struct(TypeSig const& signature)
    {
        return get_category(signature) == param_category::struct_type;
    }

    bool is_type_blittable(param_category category_value)
    {
        assert(category_value != param_category::generic_type_index);

        switch (category_value)
        {
        case param_category::enum_type:
        case param_category::fundamental_type:
            return true;
        default:
            return false;
        }
    }

    bool is_type_blittable(TypeSig const& signature, bool for_array)
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
                default:
                    break;
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
                        result = false;
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
                default:
                    break;
                }
            },
            [&](GenericTypeInstSig const&)
            {
                result = false;
            },
            [&](auto&&)
            {
                result = true;
            });

        return result;
    }

    bool is_type_blittable(TypeDef const& type)
    {
        if (type == TypeDef{})
        {
            return false;
        }

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
        default:
            return false;
        }
    }

    bool is_struct_blittable(TypeDef const& type)
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

    bool is_static(TypeDef const& type)
    {
        return get_category(type) == category::class_type && type.Flags().Abstract();
    }

    bool is_static(MethodDef const& method)
    {
        return method.Flags().Static();
    }
}
