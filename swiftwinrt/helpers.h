#pragma once
#include "winmd_reader.h"
#include "attributes.h"
#include "versioning.h"
#include "types.h"
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

    inline bool is_put_overload(MethodDef const& method)
    {
        return method.SpecialName() && method.Name().starts_with("put_");
    }

    struct method_signature
    {
        explicit method_signature(MethodDef const& method) :
            m_method(method),
            m_signature(method.Signature())
        {
            auto params = method.ParamList();

            if (m_signature.ReturnType() && params.first != params.second && params.first.Sequence() == 0)
            {
                m_return = params.first.Name();
                ++params.first;
            }

            for (uint32_t i{}; i != size(m_signature.Params()); ++i)
            {
                m_params.emplace_back(params.first + i, &m_signature.Params().first[i]);
            }
        }

        explicit method_signature(function_def const& method) :
            m_method(method.def),
            m_signature(method.def.Signature())
        {
            auto params = method.def.ParamList();

            if (method.return_type)
            {
                auto return_type = method.return_type.value();
                m_return_type = return_type.type;
                m_return = return_type.name;
                ++params.first;
            }

            for (uint32_t i{}; i != method.params.size(); ++i)
            {
                m_params.emplace_back(params.first + i, &method.params[i].signature);
            }
        }

        std::vector<std::pair<Param, ParamSig const*>>& params()
        {
            return m_params;
        }

        std::vector<std::pair<Param, ParamSig const*>> const& params() const
        {
            return m_params;
        }

        auto const& return_signature() const
        {
            return m_signature.ReturnType();
        }

        auto return_type() const
        {
            return m_return_type;
        }

        auto return_param_name() const
        {
            return m_return;
        }

        MethodDef const& method() const
        {
            return m_method;
        }

        bool is_async() const
        {
            if (!m_signature.ReturnType())
            {
                return false;
            }

            bool async{};

            call(m_signature.ReturnType().Type().Type(),
                [&](coded_index<TypeDefOrRef> const& type)
                {
                    auto const& [type_namespace, type_name] = get_type_namespace_and_name(type);
                    async = type_namespace == foundation_namespace && type_name == "IAsyncAction";
                },
                [&](GenericTypeInstSig const& type)
                {
                    auto const& [type_namespace, type_name] = get_type_namespace_and_name(type.GenericType());

                    if (type_namespace == foundation_namespace)
                    {
                        async =
                            type_name == "IAsyncOperation`1" ||
                            type_name == "IAsyncActionWithProgress`1" ||
                            type_name == "IAsyncOperationWithProgress`2";
                    }
                },
                    [](auto&&) {});

            return async;
        }

    private:

        MethodDef m_method;
        MethodDefSig m_signature;
        std::vector<std::pair<Param, ParamSig const*>> m_params;
        std::string_view m_return;
        metadata_type const* m_return_type;
    };

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

    inline auto get_name(MethodDef const& method)
    {
        auto name = method.Name();

        if (method.SpecialName())
        {
            return name.substr(name.find('_') + 1);
        }

        return name;
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

    inline bool is_noexcept(MethodDef const& method)
    {
        return is_remove_overload(method) || has_attribute(method, metadata_namespace, "NoExceptionAttribute");
    }

    inline bool has_fastabi(TypeDef const& type)
    {
        return settings.fastabi&& has_attribute(type, metadata_namespace, "FastAbiAttribute");
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

        auto const&[extends_namespace, extends_name] = get_type_namespace_and_name(extends);

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

    struct interface_info
    {
        TypeDef type;
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
        std::vector<generic_param_vector> generic_param_stack{};
    };

    using get_interfaces_t = std::vector<std::pair<std::string, interface_info>>;

    inline interface_info* find(get_interfaces_t& interfaces, std::string_view const& name)
    {
        auto pair = std::find_if(interfaces.begin(), interfaces.end(), [&](auto&& pair)
        {
            return pair.first == name;
        });

        if (pair == interfaces.end())
        {
            return nullptr;
        }

        return &pair->second;
    }

    inline void insert_or_assign(get_interfaces_t& interfaces, std::string_view const& name, interface_info&& info)
    {
        if (auto existing = find(interfaces, name))
        {
            *existing = std::move(info);
        }
        else
        {
            interfaces.emplace_back(name, std::move(info));
        }
    }

    inline void get_interfaces_impl(writer& w, get_interfaces_t& result, bool defaulted, bool overridable, bool base, std::vector<generic_param_vector> const& generic_param_stack, std::pair<InterfaceImpl, InterfaceImpl>&& children)
    {
        for (auto&& impl : children)
        {
            interface_info info;
            auto type = impl.Interface();
            auto name = w.write_temp("%", type);
            info.is_default = has_attribute(impl, "Windows.Foundation.Metadata", "DefaultAttribute");
            info.defaulted = !base && (defaulted || info.is_default);

            {
                // This is for correctness rather than an optimization (but helps performance as well).
                // If the interface was not previously inserted, carry on and recursively insert it.
                // If a previous insertion was defaulted we're done as it is correctly captured.
                // If a newly discovered instance of a previous insertion is not defaulted, we're also done.
                // If it was previously captured as non-defaulted but now found as defaulted, we carry on and
                // rediscover it as we need it to be defaulted recursively.

                if (auto found = find(result, name))
                {
                    if (found->defaulted || !info.defaulted)
                    {
                        continue;
                    }
                }
            }

            info.overridable = overridable || has_attribute(impl, metadata_namespace, "OverridableAttribute");
            info.base = base;
            info.generic_param_stack = generic_param_stack;
            writer::generic_param_guard guard;

            switch (type.type())
            {
                case TypeDefOrRef::TypeDef:
                {
                    info.type = type.TypeDef();
                    break;
                }
                case TypeDefOrRef::TypeRef:
                {
                    info.type = find_required(type.TypeRef());
                    w.add_depends(info.type);
                    break;
                }
                case TypeDefOrRef::TypeSpec:
                {
                    auto type_signature = type.TypeSpec().Signature();

                    generic_param_vector names;

                    for (auto&& arg : type_signature.GenericTypeInst().GenericArgs())
                    {
                        auto semantics = valuetype_copy_semantics::equal;
                        bool blittable = is_type_blittable(arg);
                        if (is_struct(arg) && blittable)
                        {
                            semantics = valuetype_copy_semantics::blittable;
                        }
                        else if (!blittable)
                        {
                            semantics = valuetype_copy_semantics::nonblittable;
                        }

                        names.emplace_back(arg, semantics);
                    }


                    guard = w.push_generic_params(type_signature.GenericTypeInst());
                    auto signature = type_signature.GenericTypeInst();
                    info.type = find_required(signature.GenericType());

                    break;
                }
            }

            info.exclusive = has_attribute(info.type, "Windows.Foundation.Metadata", "ExclusiveToAttribute");
            get_interfaces_impl(w, result, info.defaulted, info.overridable, base, info.generic_param_stack, info.type.InterfaceImpl());
            insert_or_assign(result, name, std::move(info));
        }
    };

    inline auto get_interfaces(writer& w, TypeDef const& type)
    {
        w.abi_types = false;
        get_interfaces_t result;
        get_interfaces_impl(w, result, false, false, false, {}, type.InterfaceImpl());

        for (auto&& base : get_bases(type))
        {
            get_interfaces_impl(w, result, false, false, true, {}, base.InterfaceImpl());
        }

        if (!has_fastabi(type))
        {
            return result;
        }

        size_t count = 0;

        if (auto history = get_contract_history(type))
        {
            for (auto& pair : result)
            {
                if (pair.second.exclusive && !pair.second.base && !pair.second.overridable)
                {
                    ++count;

                    auto introduced = get_initial_contract_version(pair.second.type);
                    pair.second.relative_version.second = introduced.version;

                    auto itr = std::find_if(history->previous_contracts.begin(), history->previous_contracts.end(), [&](previous_contract const& prev)
                        {
                            return prev.contract_from == introduced.name;
                        });
                    if (itr != history->previous_contracts.end())
                    {
                        pair.second.relative_version.first = static_cast<uint32_t>(itr - history->previous_contracts.begin());
                    }
                    else
                    {
                        assert(history->current_contract.name == introduced.name);
                        pair.second.relative_version.first = static_cast<uint32_t>(history->previous_contracts.size());
                    }
                }
            }
        }

        std::partial_sort(result.begin(), result.begin() + count, result.end(), [](auto&& left_pair, auto&& right_pair)
        {
            auto& left = left_pair.second;
            auto& right = right_pair.second;

            // Sort by base before is_default because each base will have a default.
            if (left.base != right.base)
            {
                return !left.base;
            }

            if (left.is_default != right.is_default)
            {
                return left.is_default;
            }

            if (left.overridable != right.overridable)
            {
                return !left.overridable;
            }

            if (left.exclusive != right.exclusive)
            {
                return left.exclusive;
            }

            auto left_enabled = is_always_enabled(left.type);
            auto right_enabled = is_always_enabled(right.type);

            if (left_enabled != right_enabled)
            {
                return left_enabled;
            }

            if (left.relative_version != right.relative_version)
            {
                return left.relative_version < right.relative_version;
            }

            return left_pair.first < right_pair.first;
        });

        std::for_each_n(result.begin(), count, [](auto && pair)
        {
            pair.second.fastabi = true;
        });

        return result;
    }

    inline bool implements_interface(TypeDef const& type, std::string_view const& name)
    {
        for (auto&& impl : type.InterfaceImpl())
        {
            const auto iface = impl.Interface();
            if (iface.type() != TypeDefOrRef::TypeSpec && type_name(iface) == name)
            {
                return true;
            }
        }

        if (auto base = get_base_class(type))
        {
            return implements_interface(base, name);
        }
        else
        {
            return false;
        }
    }

    inline bool has_fastabi_tearoffs(writer& w, TypeDef const& type)
    {
        for (auto&& [name, info] : get_interfaces(w, type))
        {
            if (info.is_default)
            {
                continue;
            }

            return info.fastabi;
        }

        return false;
    }

    inline std::size_t get_fastabi_size(writer& w, TypeDef const& type)
    {
        if (!has_fastabi(type))
        {
            return 0;
        }

        auto result = 6 + get_bases(type).size();

        for (auto&& [name, info] : get_interfaces(w, type))
        {
            if (!info.fastabi)
            {
                break;
            }

            result += size(info.type.MethodList());
        }

        return result;
    }

    inline auto get_fastabi_size(writer& w, std::vector<TypeDef> const& classes)
    {
        std::size_t result{};

        for (auto&& type : classes)
        {
            result = (std::max)(result, get_fastabi_size(w, type));
        }

        return result;
    }

    struct factory_info
    {
        TypeDef type;
        bool activatable{};
        bool statics{};
        bool composable{};
        bool visible{};
    };

    inline auto get_factories(writer& w, TypeDef const& type)
    {
        auto get_system_type = [&](auto&& signature) -> TypeDef
        {
            for (auto&& arg : signature.FixedArgs())
            {
                if (auto type_param = std::get_if<ElemSig::SystemType>(&std::get<ElemSig>(arg.value).value))
                {
                    return type.get_cache().find_required(type_param->name);
                }
            }

            return {};
        };

        std::map<std::string, factory_info> result;

        for (auto&& attribute : type.CustomAttribute())
        {
            auto attribute_name = attribute.TypeNamespaceAndName();

            if (attribute_name.first != "Windows.Foundation.Metadata")
            {
                continue;
            }

            auto signature = attribute.Value();
            factory_info info;

            if (attribute_name.second == "ActivatableAttribute")
            {
                info.type = get_system_type(signature);
                info.activatable = true;
            }
            else if (attribute_name.second == "StaticAttribute")
            {
                info.type = get_system_type(signature);
                info.statics = true;
            }
            else if (attribute_name.second == "ComposableAttribute")
            {
                info.type = get_system_type(signature);
                info.composable = true;

                for (auto&& arg : signature.FixedArgs())
                {
                    if (auto visibility = std::get_if<ElemSig::EnumValue>(&std::get<ElemSig>(arg.value).value))
                    {
                        info.visible = std::get<int32_t>(visibility->value) == 2;
                        break;
                    }
                }
            }
            else
            {
                continue;
            }

            std::string name;

            if (info.type)
            {
                name = w.write_temp("%", info.type);
            }

            result[name] = std::move(info);
        }

        return result;
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

    inline std::string get_field_abi(writer& w, Field const& field)
    {
        auto signature = field.Signature();
        auto const& type = signature.Type();
        std::string name = w.write_temp("%", type);

        if (starts_with(name, "struct "))
        {
            auto ref = std::get<coded_index<TypeDefOrRef>>(type.Type());

            name = "struct{";

            for (auto&& nested : find_required(ref).FieldList())
            {
                name += " " + get_field_abi(w, nested) + " ";
                name += nested.Name();
                name += ";";
            }

            name += " }";
        }

        return name;
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

    inline bool has_factory_members(writer& w, TypeDef const& type)
    {
        for (auto&&[factory_name, factory] : get_factories(w, type))
        {
            if (!factory.type || !empty(factory.type.MethodList()))
            {
                return true;
            }
        }

        return false;
    }

    inline bool is_composable(writer& w, TypeDef const& type)
    {
        for (auto&&[factory_name, factory] : get_factories(w, type))
        {
            if (factory.composable)
            {
                return true;
            }
        }

        return false;
    }

    inline bool is_overridable(InterfaceImpl const& iface)
    {
        return has_attribute(iface, "Windows.Foundation.Metadata", "OverridableAttribute");
    }

    inline bool is_generic(TypeDef const& type) noexcept
    {
        return distance(type.GenericParam()) != 0;
    }
    
    inline bool is_generic(coded_index<TypeDefOrRef> const& type)
    {
        switch (type.type())
        {
        case TypeDefOrRef::TypeSpec:
            return true;
        case TypeDefOrRef::TypeRef:
        case TypeDefOrRef::TypeDef:
        default:
            return false;
        }
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


    inline bool has_composable_constructors(writer& w, TypeDef const& type)
    {
        for (auto&&[interface_name, factory] : get_factories(w, type))
        {
            if (factory.composable && !empty(factory.type.MethodList()))
            {
                return true;
            }
        }

        return false;
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

    inline bool can_produce(TypeDef const& type, cache const& c)
    {
        auto attribute = get_attribute(type, metadata_namespace, "ExclusiveToAttribute");

        if (!attribute)
        {
            return true;
        }

        auto interface_name = type_name(type);
        auto class_name = get_attribute_value<ElemSig::SystemType>(attribute, 0).name;
        auto class_type = c.find_required(class_name);

        for (auto&& impl : class_type.InterfaceImpl())
        {
            if (has_attribute(impl, metadata_namespace, "OverridableAttribute"))
            {
                if (interface_name == type_name(impl.Interface()))
                {
                    return true;
                }
            }
        }

        if (!settings.component)
        {
            return false;
        }

        return settings.component_filter.includes(class_name);
    }

    inline auto get_property_methods(Property const& prop)
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
        return iface.is_default && !iface.base ? "interface" : std::string("_").append(iface.type.TypeName());
    }

    inline std::string_view put_in_backticks_if_needed(std::string_view const& name)
    {
        if (name == "Protocol")
        {
            return "`Protocol`";
        }
        else if (name == "Type")
        {
            return "`Type`";
        }
        else if (name == "Self")
        {
            return "`Self`";
        }
        return name;
    }
    inline std::string_view get_swift_name(MethodDef const& method)
    {
        return put_in_backticks_if_needed(method.Name());
    }

    inline std::string_view get_swift_name(Property const& property)
    {
        return put_in_backticks_if_needed(property.Name());
    }

    inline std::string_view get_swift_name(Event const& event)
    {
        return put_in_backticks_if_needed(event.Name());
    }

    inline std::string_view get_swift_name(Field const& field)
    {
        return put_in_backticks_if_needed(field.Name());
    }

    inline std::string_view get_swift_name(Param const& param)
    {
        return param.Name();
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

    inline std::string abi_namespace(std::string_view const& ns)
    {
        std::string abi_namespace;
        constexpr auto abi_prefix = "__ABI_";
        abi_namespace.reserve(sizeof(abi_prefix) + ns.size());
        abi_namespace += abi_prefix;
        abi_namespace += ns;
        std::replace(abi_namespace.begin(), abi_namespace.end(), '.', '_');
        return abi_namespace;
    }

    inline std::string abi_namespace(TypeDef const& type)
    {
        return abi_namespace(type.TypeNamespace());
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
}
