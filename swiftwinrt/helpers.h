#pragma once
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

    inline auto get_start_time()
    {
        return std::chrono::high_resolution_clock::now();
    }

    inline auto get_elapsed_time(decltype(get_start_time()) const& start)
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
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

    inline bool operator==(type_name const& left, type_name const& right)
    {
        return left.name == right.name && left.name_space == right.name_space;
    }

    inline bool operator==(type_name const& left, std::string_view const& right)
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

    std::tuple<MethodDef, MethodDef> get_property_methods(Property const& prop);

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

    inline auto get_abi_name(MethodDef const& method)
    {
        if (auto overload = get_attribute(method, metadata_namespace, "OverloadAttribute"))
        {
            return get_attribute_value<std::string_view>(overload, 0);
        }
        else
        {
            return method.Name();
        }
    }

    inline auto get_abi_name(function_def const& method)
    {
        return get_abi_name(method.def);
    }


    inline auto get_name(MethodDef const& method)
    {
        auto name = method.Name();

        if (method.SpecialName())
        {
            return name.substr(name.find('_') + 1);
        }

        return name;
    }

    inline auto get_name(function_def const& method)
    {
        return get_name(method.def);
    }

    inline bool is_remove_overload(MethodDef const& method)
    {
        return method.SpecialName() && method.Name().starts_with("remove_");
    }

    inline bool is_add_overload(MethodDef const& method)
    {
        return method.SpecialName() && method.Name().starts_with("add_");
    }

    inline bool is_get_overload(MethodDef const& method)
    {
        return method.SpecialName() && method.Name().starts_with("get_");
    }

    inline bool is_put_overload(MethodDef const& method)
    {
        return method.SpecialName() && method.Name().starts_with("put_");
    }

    inline bool is_noexcept(MethodDef const& method)
    {
        return is_remove_overload(method) || has_attribute(method, metadata_namespace, "NoExceptionAttribute");
    }

    inline bool is_noexcept(metadata_type const& type, function_def const& method)
    {
        if (is_winrt_generic_collection(type)) return true;
        return is_noexcept(method.def);
    }

    inline bool has_fastabi(TypeDef const& type)
    {
        return settings.fastabi && has_attribute(type, metadata_namespace, "FastAbiAttribute");
    }

    inline bool is_always_disabled(TypeDef const& type)
    {
        if (settings.component_ignore_velocity)
        {
            return false;
        }

        auto feature = get_attribute(type, "Windows.Foundation.Metadata", "FeatureAttribute");

        if (!feature)
        {
            return false;
        }

        auto stage = get_attribute_value<ElemSig::EnumValue>(feature, 0);
        return stage.equals_enumerator("AlwaysDisabled");
    }

    inline bool is_always_enabled(TypeDef const& type)
    {
        auto feature = get_attribute(type, "Windows.Foundation.Metadata", "FeatureAttribute");

        if (!feature)
        {
            return true;
        }

        auto stage = get_attribute_value<ElemSig::EnumValue>(feature, 0);
        return stage.equals_enumerator("AlwaysEnabled");
    }

    inline TypeDef get_base_class(TypeDef const& derived)
    {
        auto extends = derived.Extends();
        if (!extends)
        {
            return{};
        }

        auto const& [extends_namespace, extends_name] = get_type_namespace_and_name(extends);

        // the following base types are for objects and delegates, but they don't actually
        // have metadata in the cache. We return an empty type instead of crashing.
        if (extends_name == "Object" && extends_namespace == "System")
        {
            return {};
        }
        if (extends_name == "MulticastDelegate" && extends_namespace == "System")
        {
            return {};
        }
        return find_required(extends);
    };


    inline auto get_bases(TypeDef const& type)
    {
        std::vector<TypeDef> bases;

        for (auto base = get_base_class(type); base; base = get_base_class(base))
        {
            bases.push_back(base);
        }

        return bases;
    }

    template <typename T>
    inline bool is_composable(T const& type)
    {
        return has_attribute(type, "Windows.Foundation.Metadata", "ComposableAttribute");
    }

    inline bool is_exclusive(interface_type const& type)
    {
        return has_attribute(type.type(), "Windows.Foundation.Metadata", "ExclusiveToAttribute");
    }

    inline auto find_type(coded_index<winmd::reader::TypeDefOrRef> type)
    {
        switch (type.type())
        {
        case TypeDefOrRef::TypeDef:
            return type.TypeDef();
        case TypeDefOrRef::TypeRef:
            return find_required(type.TypeRef());
        case TypeDefOrRef::TypeSpec:
        {
            auto type_signature = type.TypeSpec().Signature();
            auto generic_inst = type_signature.GenericTypeInst();
            return find_required(generic_inst.GenericType());
        }
        default:
            assert(false);
            return TypeDef{};
        }
    }

    inline auto get_delegate_method(TypeDef const& type)
    {
        auto methods = type.MethodList();

        auto method = std::find_if(begin(methods), end(methods), [](auto&& method)
            {
                return method.Name() == "Invoke";
            });

        if (method == end(methods))
        {
            throw_invalid("Delegate's Invoke method not found");
        }

        return method;
    }

    inline std::string get_component_filename(TypeDef const& type)
    {
        std::string result{ type.TypeNamespace() };
        result += '.';
        result += type.TypeName();

        if (!settings.component_name.empty() && starts_with(result, settings.component_name))
        {
            result = result.substr(settings.component_name.size());

            if (starts_with(result, "."))
            {
                result.erase(result.begin());
            }
        }

        return result;
    }

    inline std::string get_generated_component_filename(TypeDef const& type)
    {
        auto result = get_component_filename(type);

        if (!settings.component_prefix)
        {
            std::replace(result.begin(), result.end(), '.', '/');
        }

        return result;
    }

    inline bool is_overridable(InterfaceImpl const& iface)
    {
        return has_attribute(iface, "Windows.Foundation.Metadata", "OverridableAttribute");
    }

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

    inline bool has_projected_types(cache::namespace_members const& members)
    {
        return
            !members.interfaces.empty() ||
            !members.classes.empty() ||
            !members.enums.empty() ||
            !members.structs.empty() ||
            !members.delegates.empty();
    }

    inline TypeDef get_exclusive_to(TypeDef const& type)
    {
        auto attribute = get_attribute(type, metadata_namespace, "ExclusiveToAttribute");
        assert(attribute);

        auto class_name = get_attribute_value<ElemSig::SystemType>(attribute, 0).name;
        return type.get_cache().find_required(class_name);
    }

    inline TypeDef get_exclusive_to(typedef_base const& type)
    {
        return get_exclusive_to(type.type());
    }

    inline bool is_exclusive(typedef_base const& type)
    {
        return has_attribute(type.type(), "Windows.Foundation.Metadata", "ExclusiveToAttribute");
    }

    inline const class_type* try_get_exclusive_to(writer& w, typedef_base const& type)
    {
        auto attribute = get_attribute(type.type(), metadata_namespace, "ExclusiveToAttribute");

        if (!attribute)
        {
            return {};
        }
        auto class_name = get_attribute_value<ElemSig::SystemType>(attribute, 0).name;
        auto last_ns_index = class_name.find_last_of('.');
        assert(last_ns_index != class_name.npos);
        auto ns = class_name.substr(0, last_ns_index);
        auto name = class_name.substr(last_ns_index + 1);

        return dynamic_cast<const class_type*>(&w.cache->find(ns, name));
    }

    inline std::optional<attributed_type> try_get_factory_info(writer& w, typedef_base const& type)
    {
        auto attribute = get_attribute(type.type(), metadata_namespace, "ExclusiveToAttribute");

        if (auto classType = try_get_exclusive_to(w, type))
        {
            auto search = classType->factories.find(std::string(type.swift_type_name()));
            if (search != classType->factories.end())
            {
                return search->second;
            }
        }

        return {};
    }

    inline std::tuple<MethodDef, MethodDef> get_property_methods(Property const& prop)
    {
        MethodDef get_method{}, set_method{};

        for (auto&& method_semantic : prop.MethodSemantic())
        {
            auto semantic = method_semantic.Semantic();

            if (semantic.Getter())
            {
                get_method = method_semantic.Method();
            }
            else if (semantic.Setter())
            {
                set_method = method_semantic.Method();
            }
            else
            {
                throw_invalid("Properties can only have get and set methods");
            }
        }

        XLANG_ASSERT(get_method || set_method);

        if (get_method && set_method)
        {
            XLANG_ASSERT(get_method.Flags().Static() == set_method.Flags().Static());
        }

        return std::make_tuple(get_method, set_method);
    }

    inline std::string get_swift_name(interface_info const& iface)
    {
        if (iface.is_default && !iface.base)
        {
            return "_default";
        }
        else
        {
            auto name = std::string("_").append(iface.type->swift_type_name());
            if (iface.generic_params.size() > 0)
            {
                name.erase(name.find_first_of('`'));
            }
            return name;
        }
    }

    inline std::string put_in_backticks_if_needed(std::string name) {
        // any lowercase swift keywords neet to be put in backticks
        static auto keyWords = std::set<std::string>{
            "as",
            "break",
            "case",
            "catch",
            "class",
            "continue",
            "default",
            "defer",
            "do",
            "else",
            "enum",
            "extension",
            "fallthrough",
            "false",
            "for",
            "func",
            "if",
            "import",
            "in",
            "internal",
            "is",
            "let",
            "nil",
            "private",
            "protocol",
            "public",
            "repeat",
            "rethrows",
            "return",
            "self",
            "static",
            "struct",
            "subscript",
            "super",
            "switch",
            "throw",
            "throws",
            "true",
            "try",
            "var",
            "where",
            "while",
        };
        if (keyWords.contains(name))
        {
            return "`" + name + "`";
        }
        return name;
    }

    inline std::string to_camel_case(std::string_view const& name)
    {
        std::string result = std::string(name);
        result[0] = tolower(result[0]);

        // One or two leading capitals: GetFoo -> getFoo / UInt32 -> uint32
        // 3+ leading capitals or mixed digits, keep the last one:
        //    UIElement -> uiElement / HELLOWorld -> helloWorld / R8G8B8Alpha -> r8g8b8Alpha
        if (result.size() > 1 && isupper(result[1]) || isdigit(result[1])){
            result[1] = tolower(result[1]);
            int next = 2;
            while (next < result.size() && (isupper(result[next]) || isdigit(result[next])))
            {
                result[next - 1] = tolower(result[next - 1]);
                next++;
            }

            // If we got to the end of the string, this means everything was uppercase,
            // so we need to lowercase the last character
            if (next == result.size() && isupper(result[next -1])) {
                result[next - 1] = tolower(result[next - 1]);
            }
        }

        return put_in_backticks_if_needed(result);
    }

    inline std::string get_swift_name(MethodDef const& method)
    {
        // the swift name for the Invoke method of a delegate is the `handler` property
        if (get_category(method.Parent()) == category::delegate_type && method.Name() != ".ctor")
        {
            return "";
        }
        else if (is_get_overload(method) || is_put_overload(method) || is_add_overload(method))
        {
            return to_camel_case(method.Name().substr(sizeof("get")));
        }
        else if (is_remove_overload(method))
        {
            return to_camel_case(method.Name().substr(sizeof("remove")));
        }
        else
        {
            return to_camel_case(method.Name());
        }
    }

    inline std::string get_swift_member_name(std::string_view const& name)
    {
        return to_camel_case(name);
    }

    inline std::string get_swift_name(Property const& property)
    {
        return get_swift_member_name(property.Name());
    }

    inline std::string get_swift_name(Event const& event)
    {
        return get_swift_member_name(event.Name());
    }

    inline std::string get_swift_name(Field const& field)
    {
        return get_swift_member_name(field.Name());
    }

    inline std::string get_swift_name(Param const& param)
    {
        return put_in_backticks_if_needed(std::string(param.Name()));
    }

    inline std::string local_swift_param_name(std::string const& param_name)
    {
        std::string local_name = "_";
        // if the param name starts with backticks
        if (param_name.starts_with('`'))
        {
            local_name.append(param_name.substr(1, param_name.size() - 2));
        }
        else {
            local_name.append(param_name);
        }
        return local_name;
    }

    inline std::string local_swift_param_name(std::string_view const& param_name)
    {
        return local_swift_param_name(std::string(param_name));
    }

    inline std::string local_swift_param_name(function_param const& param)
    {
        return local_swift_param_name(param.def.Name());
    }

    inline std::string get_swift_name(function_param const& param)
    {
        return get_swift_name(param.def);
    }

    inline std::string get_swift_name(function_return_type const& return_type)
    {
        return get_swift_member_name(return_type.name);
    }

    inline std::string get_swift_name(property_def const& property)
    {
        return get_swift_member_name(property.def.Name());
    }

    inline std::string get_swift_name(function_def const& function)
    {
        // the swift name for the Invoke method of a delegate is the `handler` property
        if (get_category(function.def.Parent()) == category::delegate_type && function.def.Name() != ".ctor")
        {
            return "handler";
        }
        return get_swift_member_name(function.def.Name());
    }

    inline std::string get_swift_name(struct_member const& member)
    {
        return get_swift_member_name(member.field.Name());
    }

    inline std::string_view get_abi_name(struct_member const& member)
    {
        return member.field.Name();
    }
    inline std::string_view remove_backtick(std::string_view const& name)
    {
        auto back_tick_i = name.find_first_of('`');
        if (back_tick_i != name.npos)
        {
            return name.substr(0, back_tick_i);
        }
        return name;
    }


    inline std::string internal_namepace(std::string prefix, std::string_view const& ns)
    {
        std::string internal_namespace;
        internal_namespace.reserve(prefix.size() + ns.size());
        internal_namespace += prefix;
        internal_namespace += ns;
        std::replace(internal_namespace.begin(), internal_namespace.end(), '.', '_');
        return internal_namespace;
    }

    inline std::string abi_namespace(std::string_view const& ns)
    {
        return internal_namepace("__ABI_", ns);
    }

    inline std::string impl_namespace(std::string_view const& ns)
    {
        return internal_namepace("__IMPL_", ns);
    }

    inline std::string abi_namespace(TypeDef const& type)
    {
        return abi_namespace(type.TypeNamespace());
    }

    inline std::string abi_namespace(metadata_type const& type)
    {
        return abi_namespace(type.swift_logical_namespace());
    }

    inline std::string abi_namespace(const metadata_type * type)
    {
        return abi_namespace(*type);
    }

    inline winmd::reader::ElementType underlying_enum_type(winmd::reader::TypeDef const& type)
    {
        return std::get<winmd::reader::ElementType>(type.FieldList().first.Signature().Type().Type());
    }

    // NOTE: 37 characters for the null terminator; the actual string is 36 characters
    inline std::array<char, 37> type_iid(winmd::reader::TypeDef const& type)
    {
        using namespace std::literals;

        std::array<char, 37> result;

        auto attr = get_attribute(type, metadata_namespace, "GuidAttribute"sv);
        if (!attr)
        {
            swiftwinrt::throw_invalid("'Windows.Foundation.Metadata.GuidAttribute' attribute for type '", type.TypeNamespace(),
                ".", type.TypeName(), "' not found");
        }

        auto value = attr.Value();
        auto const& args = value.FixedArgs();
        // 966BE0A7-B765-451B-AAAB-C9C498ED2594
        std::snprintf(result.data(), result.size(), "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            std::get<uint32_t>(std::get<ElemSig>(args[0].value).value),
            std::get<uint16_t>(std::get<ElemSig>(args[1].value).value),
            std::get<uint16_t>(std::get<ElemSig>(args[2].value).value),
            std::get<uint8_t>(std::get<ElemSig>(args[3].value).value),
            std::get<uint8_t>(std::get<ElemSig>(args[4].value).value),
            std::get<uint8_t>(std::get<ElemSig>(args[5].value).value),
            std::get<uint8_t>(std::get<ElemSig>(args[6].value).value),
            std::get<uint8_t>(std::get<ElemSig>(args[7].value).value),
            std::get<uint8_t>(std::get<ElemSig>(args[8].value).value),
            std::get<uint8_t>(std::get<ElemSig>(args[9].value).value),
            std::get<uint8_t>(std::get<ElemSig>(args[10].value).value));

        return result;
    }

    inline constexpr std::pair<std::string_view, std::string_view> decompose_type(std::string_view typeName) noexcept
    {
        auto pos = typeName.find('<');
        pos = typeName.rfind('.', pos);
        if (pos == std::string_view::npos)
        {
            // No namespace
            XLANG_ASSERT(false);
            return { std::string_view{}, typeName };
        }

        return { typeName.substr(0, pos), typeName.substr(pos + 1) };
    }

    inline param_category get_category(const metadata_type* type, TypeDef* signature_type = nullptr)
    {
        if (signature_type)
        {
            if (auto typedefBase = dynamic_cast<const typedef_base*>(type))
            {
                *signature_type = typedefBase->type();
            }
        }

        if (auto enumType = dynamic_cast<const enum_type*>(type))
        {
            return param_category::enum_type;
        }
        if (auto structType = dynamic_cast<const struct_type*>(type))
        {
            return param_category::struct_type;
        }
        if (auto elementType = dynamic_cast<const element_type*>(type))
        {
            if (elementType->type() == ElementType::String) return param_category::string_type;
            if (elementType->type() == ElementType::Object) return param_category::object_type;
            if (elementType->type() == ElementType::Boolean) return param_category::boolean_type;
            if (elementType->type() == ElementType::Char) return param_category::character_type;
            return param_category::fundamental_type;
        }
        if (auto sysType = dynamic_cast<const system_type*>(type))
        {
            return sysType->category();
        }
        if (auto mapped = dynamic_cast<const mapped_type*>(type))
        {
            if (signature_type)
            {
                *signature_type = mapped->type();
            }
            if (mapped->swift_type_name() == "EventRegistrationToken") return param_category::struct_type;
            if (mapped->swift_type_name() == "IAsyncInfo") return param_category::object_type;
            if (mapped->swift_type_name() == "HResult") return param_category::fundamental_type;
            assert(false); // unexpected mapped type
        }
        if (is_generic_inst(type))
        {
            return param_category::generic_type;
        }

        // delegates, interfaces, and classes are all object type
        return param_category::object_type;
    }

    template<typename T>
    inline std::string get_full_type_name(T const& type)
    {
        type_name name(type);
        std::string result;
        result.reserve(name.name_space.length() + name.name.length() + 1);
        result += name.name_space;
        result += '.';
        result += name.name;
        return result;
    }

    inline std::string get_full_type_name(TypeRef const& type)
    {
        return get_full_type_name<TypeRef>(type);
    }

    inline std::string get_full_type_name(TypeDef const& type)
    {
        return get_full_type_name<TypeDef>(type);
    }

    inline std::string_view get_full_type_name(generic_inst const& type)
    {
        return type.swift_full_name();
    }

    template<typename T>
    inline TypeDef find_required(T const& type, std::string_view typeName)
    {
        return type.get_cache().find_required(typeName);
    }

    template<>
    inline TypeDef find_required(coded_index<TypeDefOrRef> const& type, std::string_view typeName)
    {
        if (type.type() == TypeDefOrRef::TypeRef)
        {
            return find_required(type.TypeRef(), typeName);
        }
        else if (type.type() == TypeDefOrRef::TypeDef)
        {
            return find_required(type.TypeDef(), typeName);
        }
        else
        {
            XLANG_ASSERT(false);
            return {};
        }
    }
    template<>
    inline TypeDef find_required(generic_inst const& type, std::string_view typeName)
    {
        return find_required(type.generic_type()->type(), typeName);
    }

    template<>
    inline TypeDef find_required(metadata_type const& type, std::string_view typeName)
    {
        if (auto typedefBase = dynamic_cast<const typedef_base*>(&type))
        {
            return find_required(typedefBase->type(), typeName);
        }
        else if (auto genericInst = dynamic_cast<const generic_inst*>(&type))
        {
            return find_required(genericInst->generic_type()->type(), typeName);
        }
        else
        {
            assert(false);
            return {};
        }
    }

    inline std::pair<std::string_view, std::string_view> get_type_namespace_and_name(metadata_type const& type)
    {
        return std::make_pair(type.swift_logical_namespace(), type.swift_type_name());
    }

    inline bool is_struct_blittable(struct_type const& type)
    {
        for (auto&& member : type.members)
        {
            if (!is_type_blittable(member.field.Signature().Type()))
            {
                return false;
            }
        }

        return true;
    }

    inline bool is_struct(metadata_type const& type)
    {
        return dynamic_cast<const struct_type*>(&type) != nullptr;
    }

    inline bool needs_wrapper(param_category category)
    {
        return category == param_category::object_type || category == param_category::generic_type;
    }

    inline bool is_overridable(metadata_type const& type)
    {
        if (auto typedefBase = dynamic_cast<const typedef_base*>(&type))
        {
            return has_attribute(typedefBase->type(), "Windows.Foundation.Metadata", "OverridableAttribute");
        }
        return false;
    }

}
