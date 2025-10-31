#include "pch.h"

#include "utility/metadata_helpers.h"
#include "utility/type_helpers.h"
#include "utility/swift_codegen_utils.h"
#include "utility/type_writers.h"
#include "utility/settings.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstdio>
#include <set>

#include <cassert>

using namespace winmd::reader;
using namespace std::literals;

namespace swiftwinrt
{

    std::string_view get_name(MethodDef const& method)
    {
        auto name = method.Name();
        if (method.SpecialName())
        {
            return name.substr(name.find('_') + 1);
        }

        return name;
    }

    std::string_view get_name(function_def const& method)
    {
        return get_name(method.def);
    }

    bool is_remove_overload(MethodDef const& method)
    {
        return method.SpecialName() && method.Name().starts_with("remove_");
    }

    bool is_add_overload(MethodDef const& method)
    {
        return method.SpecialName() && method.Name().starts_with("add_");
    }

    bool is_get_overload(MethodDef const& method)
    {
        return method.SpecialName() && method.Name().starts_with("get_");
    }

    bool is_put_overload(MethodDef const& method)
    {
        return method.SpecialName() && method.Name().starts_with("put_");
    }

    bool is_noexcept(MethodDef const& method)
    {
        return is_remove_overload(method) || has_attribute(method, metadata_namespace, "NoExceptionAttribute");
    }

    bool is_noexcept(metadata_type const& type, function_def const& method)
    {
        if (is_winrt_generic_collection(type))
        {
            return true;
        }

        return is_noexcept(method.def);
    }

    bool has_fastabi(TypeDef const& type)
    {
        return settings.fastabi && has_attribute(type, metadata_namespace, "FastAbiAttribute");
    }

    bool is_always_disabled(TypeDef const& type)
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

    bool is_always_enabled(TypeDef const& type)
    {
        auto feature = get_attribute(type, "Windows.Foundation.Metadata", "FeatureAttribute");
        if (!feature)
        {
            return true;
        }

        auto stage = get_attribute_value<ElemSig::EnumValue>(feature, 0);
        return stage.equals_enumerator("AlwaysEnabled");
    }

    TypeDef get_base_class(TypeDef const& derived)
    {
        auto extends = derived.Extends();
        if (!extends)
        {
            return {};
        }

        auto const& [extends_namespace, extends_name] = get_type_namespace_and_name(extends);

        if (extends_name == "Object" && extends_namespace == "System")
        {
            return {};
        }

        if (extends_name == "MulticastDelegate" && extends_namespace == "System")
        {
            return {};
        }

        return find_required(extends);
    }

    std::vector<TypeDef> get_bases(TypeDef const& type)
    {
        std::vector<TypeDef> bases;
        for (auto base = get_base_class(type); base; base = get_base_class(base))
        {
            bases.push_back(base);
        }

        return bases;
    }

    std::chrono::high_resolution_clock::time_point get_start_time()
    {
        return std::chrono::high_resolution_clock::now();
    }

    std::chrono::milliseconds get_elapsed_time(std::chrono::high_resolution_clock::time_point const& start)
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
    }

    bool operator==(type_name const& left, type_name const& right)
    {
        return left.name == right.name && left.name_space == right.name_space;
    }

    bool operator==(type_name const& left, std::string_view right)
    {
        if (left.name.size() + 1 + left.name_space.size() != right.size())
        {
            return false;
        }

        if (right[left.name_space.size()] != '.')
        {
            return false;
        }

        if (right.compare(left.name_space.size() + 1, left.name.size(), left.name) != 0)
        {
            return false;
        }

        return right.compare(0, left.name_space.size(), left.name_space) == 0;
    }

    bool is_exclusive(interface_type const& type)
    {
        return has_attribute(type.type(), "Windows.Foundation.Metadata", "ExclusiveToAttribute");
    }

    TypeDef find_type(coded_index<TypeDefOrRef> type)
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

    MethodDef get_delegate_method(TypeDef const& type)
    {
        auto methods = type.MethodList();

        auto method = std::find_if(begin(methods), end(methods), [](auto&& candidate)
            {
                return candidate.Name() == "Invoke";
            });

        if (method == end(methods))
        {
            throw_invalid("Delegate's Invoke method not found");
        }

        return *method;
    }

    std::string get_component_filename(TypeDef const& type)
    {
        std::string result{ type.TypeNamespace() };
        result += '.';
        result += type.TypeName();

        if (!settings.component_name.empty() && starts_with(result, settings.component_name))
        {
            result = result.substr(settings.component_name.size());

            if (starts_with(result, "."sv))
            {
                result.erase(result.begin());
            }
        }

        return result;
    }

    std::string get_generated_component_filename(TypeDef const& type)
    {
        auto result = get_component_filename(type);

        if (!settings.component_prefix)
        {
            std::replace(result.begin(), result.end(), '.', '/');
        }

        return result;
    }

    bool is_overridable(InterfaceImpl const& iface)
    {
        return has_attribute(iface, "Windows.Foundation.Metadata", "OverridableAttribute");
    }

    bool has_projected_types(cache::namespace_members const& members)
    {
        return
            !members.interfaces.empty() ||
            !members.classes.empty() ||
            !members.enums.empty() ||
            !members.structs.empty() ||
            !members.delegates.empty();
    }

    bool has_projected_types(namespace_cache const& members)
    {
        return
            !members.interfaces.empty() ||
            !members.classes.empty() ||
            !members.enums.empty() ||
            !members.structs.empty() ||
            !members.delegates.empty();
    }

    TypeDef get_exclusive_to(TypeDef const& type)
    {
        auto attribute = get_attribute(type, metadata_namespace, "ExclusiveToAttribute");
        assert(attribute);

        auto class_name = get_attribute_value<ElemSig::SystemType>(attribute, 0).name;
        return type.get_cache().find_required(class_name);
    }

    TypeDef get_exclusive_to(typedef_base const& type)
    {
        return get_exclusive_to(type.type());
    }

    bool is_exclusive(typedef_base const& type)
    {
        return has_attribute(type.type(), "Windows.Foundation.Metadata", "ExclusiveToAttribute");
    }

    const class_type* try_get_exclusive_to(writer& w, typedef_base const& type)
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

    std::optional<attributed_type> try_get_factory_info(writer& w, typedef_base const& type)
    {
        auto attribute = get_attribute(type.type(), metadata_namespace, "ExclusiveToAttribute");
        (void)attribute;

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

    std::tuple<MethodDef, MethodDef> get_property_methods(Property const& prop)
    {
        MethodDef get_method{};
        MethodDef set_method{};

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
        }

        XLANG_ASSERT(get_method || set_method);

        if (get_method && set_method)
        {
            XLANG_ASSERT(get_method.Flags().Static() == set_method.Flags().Static());
        }

        return std::make_tuple(get_method, set_method);
    }

    winmd::reader::ElementType underlying_enum_type(winmd::reader::TypeDef const& type)
    {
        return std::get<winmd::reader::ElementType>(type.FieldList().first.Signature().Type().Type());
    }

    std::array<char, 37> type_iid(winmd::reader::TypeDef const& type)
    {
        std::array<char, 37> result{};

        auto attr = get_attribute(type, metadata_namespace, "GuidAttribute"sv);
        if (!attr)
        {
            swiftwinrt::throw_invalid("'Windows.Foundation.Metadata.GuidAttribute' attribute for type '",
                type.TypeNamespace(), ".", type.TypeName(), "' not found");
        }

        auto value = attr.Value();
        auto const& args = value.FixedArgs();

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

    param_category get_category(metadata_type const* type, TypeDef* signature_type)
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
            assert(false);
        }
        if (is_generic_inst(type))
        {
            return param_category::generic_type;
        }

        return param_category::object_type;
    }

    std::string_view get_full_type_name(generic_inst const& type)
    {
        return type.swift_full_name();
    }

    std::pair<std::string_view, std::string_view> get_type_namespace_and_name(metadata_type const& type)
    {
        return std::make_pair(type.swift_logical_namespace(), type.swift_type_name());
    }

    bool is_struct_blittable(struct_type const& type)
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

    bool is_struct(metadata_type const& type)
    {
        return dynamic_cast<const struct_type*>(&type) != nullptr;
    }


    bool is_overridable(metadata_type const& type)
    {
        if (auto typedefBase = dynamic_cast<const typedef_base*>(&type))
        {
            return has_attribute(typedefBase->type(), "Windows.Foundation.Metadata", "OverridableAttribute");
        }
        return false;
    }

    std::map<std::string, std::vector<std::string_view>> get_swift_modules(winmd::reader::cache const& cache, metadata_filter const& filter)
    {
        std::map<std::string, std::vector<std::string_view>> module_map;
        for (auto&&[ns, members] : cache.namespaces())
        {
            if (!has_projected_types(members))
            {
                continue;
            }

            if (!filter.includes_any(members))
            {
                continue;
            }
            auto module_name = get_swift_module(ns);

            auto [moduleMapItr, moduleAdded] = module_map.emplace(std::piecewise_construct,
                std::forward_as_tuple(module_name),
                std::forward_as_tuple());
            if (moduleAdded)
            {
                create_directories(writer::root_directory() / module_name);
                auto cmod = std::string("C").append(module_name);
                create_directories(writer::root_directory() / cmod);
                create_directories(writer::root_directory() / cmod / "include");
            }
            moduleMapItr->second.push_back(ns);
        }
        return module_map;
    }

    std::set<std::string_view> get_module_dependencies(
        std::string_view const& module,
        std::vector<std::string_view> const& namespaces,
        metadata_cache const& cache,
        metadata_filter const& filter)
    {
        std::set<std::string_view> moduleDependencies;
        if (module != settings.support)
        {
            moduleDependencies.emplace(settings.support);
        }
        auto dependentNamespaces = cache.get_dependent_namespaces(namespaces, filter);
        
        for (auto&& dependent_ns : dependentNamespaces)
        {
            auto dependent_module = get_swift_module(dependent_ns);
            if (dependent_module != module)
            {
                moduleDependencies.emplace(dependent_module);
            }
        }
        return moduleDependencies;
    }
}
