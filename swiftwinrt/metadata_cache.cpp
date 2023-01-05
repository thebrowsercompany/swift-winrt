#include "pch.h"

#include "types.h"
#include "abi_writer.h"
#include "metadata_cache.h"
#include "metadata_filter.h"
#include "task_group.h"
#include "helpers.h"
#include "versioning.h"
using namespace std::literals;
using namespace winmd::reader;
using namespace swiftwinrt;

metadata_cache::metadata_cache(winmd::reader::cache const& c)
{
    // We need to initialize in two phases. The first phase creates the collection of all type defs. The second phase
    // processes dependencies and initializes generic types
    // NOTE: We may only need to do this for a subset of types, but that would introduce a fair amount of complexity and
    //       the runtime cost of processing everything is relatively insignificant
    task_group group;
    for (auto const& [ns, members] : c.namespaces())
    {
        // We don't do any synchronization of access to this type's data structures, so reserve space on the "main"
        // thread. Note that set/map iterators are not invalidated on insert/emplace
        auto [nsItr, nsAdded] = namespaces.emplace(std::piecewise_construct,
            std::forward_as_tuple(ns),
            std::forward_as_tuple());
        XLANG_ASSERT(nsAdded);

        auto [tableItr, tableAdded] = m_typeTable.emplace(std::piecewise_construct,
            std::forward_as_tuple(ns),
            std::forward_as_tuple());
        XLANG_ASSERT(tableAdded);

        group.add([&, &members = members, &nsTypes = nsItr->second, &table = tableItr->second]()
        {
            process_namespace_types(members, nsTypes, table);
        });
    }
    group.get();

    for (auto& [ns, nsCache] : namespaces)
    {
        group.add([&, &nsCache = nsCache]()
        {
            process_namespace_dependencies(nsCache);
        });
    }
    group.get();
}

void metadata_cache::process_namespace_types(
    cache::namespace_members const& members,
    namespace_cache& target,
    std::map<std::string_view, metadata_type const&>& table)
{
    // Mapped types are only in the 'Windows.Foundation' namespace, so pre-compute
    bool isFoundationNamespace = members.types.begin()->second.TypeNamespace() == foundation_namespace;

    target.enums.reserve(members.enums.size());
    for (auto const& e : members.enums)
    {
        // 'AsyncStatus' is an enum
        if (isFoundationNamespace)
        {
            if (auto ptr = mapped_type::from_typedef(e))
            {
                [[maybe_unused]] auto [itr, added] = table.emplace(e.TypeName(), *ptr);
                XLANG_ASSERT(added);
                continue;
            }
        }

        target.enums.emplace_back(e);
        [[maybe_unused]] auto [itr, added] = table.emplace(e.TypeName(), target.enums.back());
        XLANG_ASSERT(added);
    }

    target.structs.reserve(members.structs.size());
    for (auto const& s : members.structs)
    {
        // 'EventRegistrationToken' and 'HResult' are structs
        if (isFoundationNamespace)
        {
            if (auto ptr = mapped_type::from_typedef(s))
            {
                [[maybe_unused]] auto [itr, added] = table.emplace(s.TypeName(), *ptr);
                XLANG_ASSERT(added);
                continue;
            }
        }
        target.structs.emplace_back(s);
        [[maybe_unused]] auto [itr, added] = table.emplace(s.TypeName(), target.structs.back());
        XLANG_ASSERT(added);
    }

    target.delegates.reserve(members.delegates.size());
    for (auto const& d : members.delegates)
    {
        target.delegates.emplace_back(d);
        [[maybe_unused]] auto [itr, added] = table.emplace(d.TypeName(), target.delegates.back());
        XLANG_ASSERT(added);
    }

    target.interfaces.reserve(members.interfaces.size());
    for (auto const& i : members.interfaces)
    {
        // 'IAsyncInfo' is an interface
        if (isFoundationNamespace)
        {
            if (auto ptr = mapped_type::from_typedef(i))
            {
                [[maybe_unused]] auto [itr, added] = table.emplace(i.TypeName(), *ptr);
                XLANG_ASSERT(added);
                continue;
            }
        }
        target.interfaces.emplace_back(i);
        [[maybe_unused]] auto [itr, added] = table.emplace(i.TypeName(), target.interfaces.back());
        XLANG_ASSERT(added);
    }

    target.classes.reserve(members.classes.size());
    for (auto const& c : members.classes)
    {
        target.classes.emplace_back(c);
        [[maybe_unused]] auto [itr, added] = table.emplace(c.TypeName(), target.classes.back());
        XLANG_ASSERT(added);
    }

    for (auto const& contract : members.contracts)
    {
        // Contract versions are attributes on the contract type itself
        auto attr = get_attribute(contract, metadata_namespace, "ContractVersionAttribute"sv);
        XLANG_ASSERT(attr);
        XLANG_ASSERT(attr.Value().FixedArgs().size() == 1);

        target.contracts.push_back(api_contract{
                contract,
                std::get<uint32_t>(std::get<ElemSig>(attr.Value().FixedArgs()[0].value).value)
            });
    }
}

void metadata_cache::process_namespace_dependencies(namespace_cache& target)
{
    init_state state{ &target };

    for (auto& enumType : target.enums)
    {
        process_enum_dependencies(state, enumType);
        XLANG_ASSERT(!state.parent_generic_inst);
    }

    for (auto& structType : target.structs)
    {
        process_struct_dependencies(state, structType);
        XLANG_ASSERT(!state.parent_generic_inst);
    }

    for (auto& delegateType : target.delegates)
    {
        process_delegate_dependencies(state, delegateType);
        XLANG_ASSERT(!state.parent_generic_inst);
    }

    for (auto& interfaceType : target.interfaces)
    {
        process_interface_dependencies(state, interfaceType);
        XLANG_ASSERT(!state.parent_generic_inst);
    }

    for (auto& classType : target.classes)
    {
        process_class_dependencies(state, classType);
        XLANG_ASSERT(!state.parent_generic_inst);
    }
}

template <typename T>
static void process_contract_dependencies(namespace_cache& target, T const& type)
{
    if (auto attr = swiftwinrt::get_contract_history(type))
    {
        target.dependent_namespaces.emplace(decompose_type(attr->current_contract.name).first);
        for (auto const& prevContract : attr->previous_contracts)
        {
            target.dependent_namespaces.emplace(decompose_type(prevContract.contract_from).first);
        }
    }

    if (auto info = is_deprecated(type))
    {
        target.dependent_namespaces.emplace(decompose_type(info->contract_type).first);
    }
}

interface_info* metadata_cache::find(get_interfaces_t& interfaces, std::string_view const& name)
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

void metadata_cache::insert_or_assign(get_interfaces_t& interfaces, std::string_view const& name, interface_info&& info)
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

void metadata_cache::get_interfaces_impl(init_state& state, writer& w, get_interfaces_t& result, bool defaulted, bool overridable, bool base, std::pair<InterfaceImpl, InterfaceImpl>&& children)
{
    for (auto&& impl : children)
    {
        interface_info info;

        auto type = impl.Interface();
        info.type = &find_dependent_type(state, type);

        info.is_default = has_attribute(impl, "Windows.Foundation.Metadata", "DefaultAttribute");
        info.defaulted = !base && (defaulted || info.is_default);
        writer::generic_param_guard guard;
        if (auto genericInst = dynamic_cast<const generic_inst*>(info.type))
        {
            guard = w.push_generic_params(*genericInst);
            info.generic_params = w.generic_param_stack.back();
        }
        auto name = w.write_temp("%", info.type);
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

        if (auto typeBase = dynamic_cast<const interface_type*>(info.type))
        {
            info.exclusive = has_attribute(typeBase->type(), "Windows.Foundation.Metadata", "ExclusiveToAttribute");

            process_contract_dependencies(*state.target, impl);
            get_interfaces_impl(state, w, result, info.defaulted, info.overridable, base, typeBase->type().InterfaceImpl());
        }

        insert_or_assign(result, name, std::move(info));
    }
};

metadata_cache::get_interfaces_t metadata_cache::get_interfaces(init_state& state, TypeDef const& type)
{
    get_interfaces_t result;

    writer w;
    w.type_namespace = type.TypeNamespace();
    get_interfaces_impl(state, w, result, false, false, false, type.InterfaceImpl());

    for (auto&& base : get_bases(type))
    {
        get_interfaces_impl(state,w, result, false, false, true, base.InterfaceImpl());
    }

    if (!has_fastabi(type))
    {
        return result;
    }

    size_t count = 0;

    auto get_interface_type = [](const metadata_type* metadataType) -> TypeDef {
        TypeDef interfaceType{};
        if (auto genericInst = dynamic_cast<const generic_inst*>(metadataType))
        {
            interfaceType = genericInst->generic_type()->type();
        }
        else if (auto iFaceType = dynamic_cast<const interface_type*>(metadataType))
        {
            interfaceType = iFaceType->type();
        }
        else 
        {
            interfaceType = dynamic_cast<const mapped_type*>(metadataType)->type();

        }
        assert(interfaceType);
        return interfaceType;
    };

    if (auto history = get_contract_history(type))
    {
        for (auto& pair : result)
        {
            if (pair.second.exclusive && !pair.second.base && !pair.second.overridable)
            {
                ++count;

                auto interfaceType = get_interface_type(pair.second.type);

                auto introduced = get_initial_contract_version(interfaceType);
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

    std::partial_sort(result.begin(), result.begin() + count, result.end(), [&get_interface_type](auto&& left_pair, auto&& right_pair)
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

            auto left_enabled = is_always_enabled(get_interface_type(left.type));
            auto right_enabled = is_always_enabled(get_interface_type(right.type));

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

    std::for_each_n(result.begin(), count, [](auto&& pair)
        {
            pair.second.fastabi = true;
        });

    return result;
}
void metadata_cache::process_enum_dependencies(init_state& state, enum_type& type)
{
    // There's no pre-processing that we need to do for enums. Just take note of the namespace dependencies that come
    // from contract version(s)/deprecations
    process_contract_dependencies(*state.target, type.type());

    for (auto const& field : type.type().FieldList())
    {
        process_contract_dependencies(*state.target, field);
    }
}

void metadata_cache::process_struct_dependencies(init_state& state, struct_type& type)
{
    process_contract_dependencies(*state.target, type.type());

    for (auto const& field : type.type().FieldList())
    {
        process_contract_dependencies(*state.target, field);
        type.members.push_back(struct_member{ field, &find_dependent_type(state, field.Signature().Type()) });
    }
}

void metadata_cache::process_delegate_dependencies(init_state& state, delegate_type& type)
{
    process_contract_dependencies(*state.target, type.type());

    // We only care about instantiations of generic types, so early exit as we won't be able to resolve references
    if (type.is_generic())
    {
        return;
    }

    // Delegates only have a single function - Invoke - that we care about
    for (auto const& method : type.type().MethodList())
    {
        if (method.Name() != ".ctor"sv)
        {
            XLANG_ASSERT(method.Name() == "Invoke"sv);
            process_contract_dependencies(*state.target, method);
            type.functions.push_back(process_function(state, method));
            break;
        }
    }

    // Should be exactly one function named 'Invoke'
    XLANG_ASSERT(type.functions.size() == 1);
}

void metadata_cache::process_interface_dependencies(init_state& state, interface_type& type)
{
    process_contract_dependencies(*state.target, type.type());

    if (type.is_generic())
    {
        // only add interface info for non-generic interfaces since we won't be able to resolve references otherwise.
        // this is what we use to know that IReference<T> derives from IPropertyValue
        for (auto const& iface : type.type().InterfaceImpl())
        {
            if (!swiftwinrt::is_generic(iface.Interface()))
            {
                interface_info info;
                info.type = &find_dependent_type(state, iface.Interface());
                type_name name{ iface.Interface() };
                type.required_interfaces.push_back(std::make_pair(std::string(name.name), info));
            }
        }
        return;
    }

    for (auto const& interfaces : get_interfaces(state, type.type()))
    {
        // TODO: https://linear.app/the-browser-company/issue/WIN-103/swiftwinrt-write-iasyncinfo
        if (!interfaces.first.ends_with("IAsyncInfo"))
        {
            type.required_interfaces.push_back(interfaces);
        }
    }

    for (auto const& method : type.type().MethodList())
    {
        process_contract_dependencies(*state.target, method);
        type.functions.push_back(process_function(state, method));
    }

    for (auto const& prop : type.type().PropertyList())
    {
        type.properties.push_back(process_property(state, prop));
    }

    for (auto const& event : type.type().EventList())
    {
        type.events.push_back(process_event(state, event));
    }
}

void metadata_cache::process_class_dependencies(init_state& state, class_type& type)
{
    process_contract_dependencies(*state.target, type.type());

    // We only care about instantiations of generic types, so early exit as we won't be able to resolve references
    if (type.is_generic())
    {
        return;
    }

    if (auto base = get_base_class(type.type()))
    {
        type.base_class = &dynamic_cast<class_type const&>(this->find(base.TypeNamespace(), base.TypeName()));
    }

    for (auto const& iface : get_interfaces(state, type.type()))
    {
        type.required_interfaces.push_back(iface);

        // NOTE: Types can have more than one default interface so long as they apply to different platforms. This is
        //       not very useful, as we have to choose one to use for function argument types, but it technically is
        //       allowed... If that's the case, just use the first one we encounter as this has the highest probability
        //       of matching MIDLRT's behavior
        if (iface.second.is_default && !type.default_interface)
        {
            type.default_interface = iface.second.type;
        }
    }

    type.factories = get_attributed_types(type.type());

    if (auto fastAttr = get_attribute(type.type(), metadata_namespace, "FastAbiAttribute"sv))
    {
        auto attrVer = version_from_attribute(fastAttr);
        interface_type* fastInterface = nullptr;
        relative_version_map rankingMap;
        for (auto const& ifaceImpl : type.type().InterfaceImpl())
        {
            // If the interface is not exclusive to this class, ignore
            auto iface = dynamic_cast<interface_type const*>(&find_dependent_type(state, ifaceImpl.Interface()));
            if (!iface || !is_exclusive(iface->type()))
            {
                continue;
            }

            // Make sure that this interface reference applies for the same versioning "scheme" as the attribute
            auto verMatch = match_versioning_scheme(attrVer, ifaceImpl);
            if (!verMatch)
            {
                // No match on the interface reference is okay so long as there is _no_ versioning information on the
                // reference. If there's not, then the requirement applies to all versioning schemes, so we look at the
                // interface for the versioning information
                if (get_attribute(ifaceImpl, metadata_namespace, "ContractVersionAttribute"sv) ||
                    get_attribute(ifaceImpl, metadata_namespace, "VersionAttribute"sv))
                {
                    continue;
                }

                verMatch = match_versioning_scheme(attrVer, iface->type());
                if (!verMatch)
                {
                    XLANG_ASSERT(false);
                    continue;
                }
            }

            // Take note if this is the default interface
            if (is_default(ifaceImpl))
            {
                XLANG_ASSERT(!fastInterface);
                XLANG_ASSERT(!iface->fast_class);

                // NOTE: 'find_dependent_type' returns a const-ref since there are some items that it returns that may
                // actually be const. This is not true for 'interface_type', hence the 'const_cast' here is appropriate
                fastInterface = const_cast<interface_type*>(iface);
                fastInterface->fast_class = &type;
                continue;
            }

            // Ignore this interface if it's overridable, experimental, or in a disabled state
            if (is_overridable(ifaceImpl))
            {
                continue;
            }
            else if (is_experimental(ifaceImpl) || is_experimental(iface->type()))
            {
                continue;
            }

            // Determine how this interface's inclusion in the class relates to the class' version history
            relative_version relVer = {};
            call(*verMatch,
                [&](contract_version const& ver)
                {
                    relVer.first = *type.contract_index(ver.name, ver.version);
                    relVer.second = ver.version;
                },
                [&](platform_version const& ver)
                {
                    // For platform versioning, the "relative contract" (relVer.first) is always zero
                    relVer.second = ver.version;
                });
            process_fastabi_required_interfaces(state, iface, relVer, rankingMap);
        }

        if (fastInterface)
        {
            // The fast default interface may have gotten added to the map as a required interface. If so, remove it
            if (auto itr = rankingMap.find(fastInterface); itr != rankingMap.end())
            {
                rankingMap.erase(itr);
            }

            for (auto& [iface, rank] : rankingMap)
            {
                version ver;
                if (std::holds_alternative<contract_version>(attrVer))
                {
                    ver = contract_version{ type.contract_from_index(rank.first)->name, rank.second };
                }
                else // platform_version
                {
                    ver = platform_version{ std::get<platform_version>(attrVer).platform, rank.second };
                }
                type.supplemental_fast_interfaces.push_back({ iface, ver });
            }

            std::sort(type.supplemental_fast_interfaces.begin(), type.supplemental_fast_interfaces.end(),
                [&](auto const& lhs, auto const& rhs)
            {
                auto& [lhsPtr, lhsRank] = *rankingMap.find(lhs.first);
                auto& [rhsPtr, rhsRank] = *rankingMap.find(rhs.first);
                if (lhsRank == rhsRank)
                {
                    // Same ranking; sort by type name
                    return lhsPtr->swift_full_name() < rhsPtr->swift_full_name();
                }

                return lhsRank < rhsRank;
            });
        }
        else
        {
            XLANG_ASSERT(rankingMap.empty());
        }
    }
}

void metadata_cache::process_fastabi_required_interfaces(
    init_state& state,
    interface_type const* currentInterface,
    relative_version rank,
    relative_version_map& interfaceMap)
{
    if (!is_exclusive(currentInterface->type()))
    {
        return; // Not exclusive-to, so can safely ignore
    }
    // NOTE: We should also ignore if this is the default interface that we are extending, however we may not have
    //       enough information at this point to make that determination, so just allow it and remove later

    if (auto itr = interfaceMap.find(currentInterface); itr != interfaceMap.end())
    {
        if (itr->second <= rank)
        {
            return; // Already processed with at least as good a match
        }
    }

    interfaceMap[currentInterface] = rank;
    for (auto const& ifaceImpl : currentInterface->type().InterfaceImpl())
    {
        auto type = &find_dependent_type(state, ifaceImpl.Interface());
        if (auto iface = dynamic_cast<interface_type const*>(type))
        {
            process_fastabi_required_interfaces(state, iface, rank, interfaceMap);
        }
    }
}

function_def metadata_cache::process_function(init_state& state, MethodDef const& def)
{
    auto paramNames = def.ParamList();
    auto sig = def.Signature();
    XLANG_ASSERT(sig.GenericParamCount() == 0);

    std::optional<function_return_type> return_type;
    if (sig.ReturnType())
    {
        std::string_view name = "result"sv;
        if ((paramNames.first != paramNames.second) && (paramNames.first.Sequence() == 0))
        {
            name = paramNames.first.Name();
            ++paramNames.first;
        }

        return_type = function_return_type{ sig.ReturnType(), name, &find_dependent_type(state, sig.ReturnType().Type()) };
    }

    std::vector<function_param> params;
    for (auto const& param : sig.Params())
    {
        XLANG_ASSERT(paramNames.first != paramNames.second);
        params.push_back(function_param{ paramNames.first, param, paramNames.first.Name(), &find_dependent_type(state, param.Type()) });
        ++paramNames.first;
    }

    return function_def{ def, std::move(return_type), std::move(params) };
}

property_def metadata_cache::process_property(init_state& state, Property const& def)
{
    auto [getter, setter] = get_property_methods(def);
    auto type = &find_dependent_type(state, def.Type().Type());
    if (getter && setter)
    {
        return property_def{ def, type, process_function(state, getter), process_function(state, setter) };
    }
    else if (getter)
    {
        return property_def{ def, type, process_function(state, getter), {} };
    }
    else
    {
        return property_def{ def, type, {}, process_function(state, setter) };
    }
}

event_def metadata_cache::process_event(init_state& state, Event const& def)
{
    return event_def{ def, &find_dependent_type(state, def.EventType()) };
}

metadata_type const& metadata_cache::find_dependent_type(init_state& state, TypeSig const& type)
{
    metadata_type const* result;
    call(type.Type(),
        [&](ElementType t)
        {
            result = &element_type::from_type(t);
        },
        [&](coded_index<TypeDefOrRef> const& t)
        {
            result = &find_dependent_type(state, t);
        },
        [&](GenericTypeIndex t)
        {
            if (state.parent_generic_inst)
            {
                if (t.index < state.parent_generic_inst->generic_params().size())
                {
                    result = state.parent_generic_inst->generic_params()[t.index];
                }
                else
                {
                    XLANG_ASSERT(false);
                    swiftwinrt::throw_invalid("GenericTypeIndex out of range");
                }
            }
            else
            {
                XLANG_ASSERT(false);
                swiftwinrt::throw_invalid("GenericTypeIndex encountered with no generic instantiation to refer to");
            }
        },
        [&](GenericTypeInstSig const& t)
        {
            result = &find_dependent_type(state, t);
        },
        [&](GenericMethodTypeIndex)
        {
            swiftwinrt::throw_invalid("Generic methods not supported");
        }, [](auto&&) {});

    return *result;
}

metadata_type const& metadata_cache::find_dependent_type(init_state& state, coded_index<TypeDefOrRef> const& type)
{
    metadata_type const* result = nullptr;
    switch (type.type())
    {
    case TypeDefOrRef::TypeSpec:
        result = &find_dependent_type(state, type.TypeSpec().Signature().GenericTypeInst());
        break;
    case TypeDefOrRef::TypeDef:
    case TypeDefOrRef::TypeRef:
        auto [ns, name] = get_type_namespace_and_name(type);

        result = &find(ns, name);
        if (auto typeDef = dynamic_cast<typedef_base const*>(result))
        {
            auto swift_namespace = result->swift_abi_namespace();
            state.target->dependent_namespaces.insert(swift_namespace);
            if (!typeDef->is_generic())
            {
                state.target->type_dependencies.emplace(*typeDef);
            }
        }
        break;
    }

    return *result;
}

metadata_type const& metadata_cache::find_dependent_type(init_state& state, GenericTypeInstSig const& type)
{
    auto genericType = dynamic_cast<typedef_base const*>(&find_dependent_type(state, type.GenericType()));
    if (!genericType)
    {
        XLANG_ASSERT(false);
        swiftwinrt::throw_invalid("Generic types must be TypeDefs");
    }
    std::vector<metadata_type const*> genericParams;
    for (auto const& param : type.GenericArgs())
    {
        genericParams.push_back(&find_dependent_type(state, param));
    }

    generic_inst inst{ genericType, std::move(genericParams) };
    auto [itr, added] = state.target->generic_instantiations.emplace(inst.swift_full_name(), std::move(inst));
    if (added)
    {
        auto restore = std::exchange(state.parent_generic_inst, &itr->second);
        auto check_dependency = [&, &itr = itr](auto const& t)
        {
            auto mdType = &find_dependent_type(state, t);
            if (auto genericType = dynamic_cast<generic_inst const*>(mdType))
            {
                itr->second.dependencies.push_back(genericType);
            }
        };

        for (auto const& iface : genericType->type().InterfaceImpl())
        {
            check_dependency(iface.Interface());
        }

        for (auto const& fn : genericType->type().MethodList())
        {
            if (fn.Name() == ".ctor"sv)
            {
                continue;
            }

            // TODO: Duplicated effort!
            itr->second.functions.push_back(process_function(state, fn));

            auto sig = fn.Signature();
            if (sig.ReturnType())
            {
                check_dependency(sig.ReturnType().Type());
            }

            for (auto const& param : sig.Params())
            {
                check_dependency(param.Type());
            }
        }

        for (auto const& prop : genericType->type().PropertyList())
        {
            itr->second.properties.push_back(process_property(state, prop ));
        }

        for (auto const& event : genericType->type().EventList())
        {
            itr->second.events.push_back(process_event(state, event));
        }


        state.parent_generic_inst = restore;
    }

    return itr->second;
}

template <typename T>
static void merge_into(std::vector<T>& from, std::vector<std::reference_wrapper<T const>>& to, metadata_filter const& f)
{
    std::vector<std::reference_wrapper<T const>> result;
    result.reserve(from.size() + to.size());
    for (auto& cp : from)
    {
        if (f.includes(cp.type()))
        {
            result.emplace_back(cp);
        }
    }
    result.shrink_to_fit();
    to.swap(result);
}

std::set<std::string_view> metadata_cache::get_dependent_namespaces(std::vector<std::string_view> const& targetNamespaces, metadata_filter const& f)
{
    std::set<std::string_view> result;
    for (auto ns : targetNamespaces)
    {
        auto itr = namespaces.find(ns);
        if (itr == namespaces.end())
        {
            XLANG_ASSERT(false);
            swiftwinrt::throw_invalid("Namespace '", ns, "' not found");
        }

        for (auto& dependent : itr->second.dependent_namespaces)
        {
            if (f.includes_ns(dependent))
            {
                result.insert(dependent);
            }
        }
    }
    return result;
}

type_cache metadata_cache::compile_namespaces(std::vector<std::string_view> const& targetNamespaces, metadata_filter const& f)
{
    type_cache result{ this };

    auto includes_namespace = [&](std::string_view ns)
    {
        return std::find(targetNamespaces.begin(), targetNamespaces.end(), ns) != targetNamespaces.end();
    };

    for (auto ns : targetNamespaces)
    {
        auto itr = namespaces.find(ns);
        if (itr == namespaces.end())
        {
            XLANG_ASSERT(false);
            swiftwinrt::throw_invalid("Namespace '", ns, "' not found");
        }

        // Merge the type definitions together
        merge_into(itr->second.enums, result.enums, f);
        merge_into(itr->second.structs, result.structs, f);
        merge_into(itr->second.delegates, result.delegates, f);
        merge_into(itr->second.interfaces, result.interfaces, f);
        merge_into(itr->second.classes, result.classes, f);

        // Merge the dependencies together
        for (auto& dependent : itr->second.dependent_namespaces)
        {
            if (f.includes_ns(dependent))
            {
                result.dependent_namespaces.insert(dependent);
            }
        }
     
        for (auto& generic : itr->second.generic_instantiations)
        {
            if (f.includes_generic(generic.first))
            {
                result.generic_instantiations.insert(generic);
            }
        }

        for (auto& depends : itr->second.type_dependencies)
        {
            if (f.includes(depends.get().type()))
            {
                if (includes_namespace(depends.get().swift_logical_namespace()))
                {
                    result.internal_dependencies.insert(depends);
                }
                else
                {
                    result.external_dependencies.insert(depends);
                }
            }
        }
 
        // Remove metadata only types
        auto remove_type = [&](auto& list, std::string_view name)
        {
            auto [lo, hi] = std::equal_range(list.begin(), list.end(), name, typename_comparator{});
            XLANG_ASSERT((lo + 1) >= hi);
            list.erase(lo, hi);
        };

        if (ns == "Windows.Foundation.Metadata"sv)
        {
            remove_type(result.enums, "Windows.Foundation.Metadata.AttributeTargets");
            remove_type(result.enums, "Windows.Foundation.Metadata.CompositionType");
            remove_type(result.enums, "Windows.Foundation.Metadata.DeprecationType");
            remove_type(result.enums, "Windows.Foundation.Metadata.FeatureStage");
            remove_type(result.enums, "Windows.Foundation.Metadata.MarshalingType");
            remove_type(result.enums, "Windows.Foundation.Metadata.Platform");
            remove_type(result.enums, "Windows.Foundation.Metadata.ThreadingModel");
        }
    }

    // Structs need all members to be defined prior to the struct definition
    std::pair range{ result.structs.begin(), result.structs.end() };
    while (range.first != range.second)
    {
        bool shouldAdvance = true;
        for (auto const& member : range.first->get().members)
        {
            if (auto structType = dynamic_cast<struct_type const*>(member.type))
            {
                if (includes_namespace(structType->swift_abi_namespace()))
                {
                    auto itr = std::find_if(range.first + 1, range.second, [&](auto const& type)
                    {
                        return &type.get() == structType;
                    });
                    if (itr != range.second)
                    {
                        std::rotate(range.first, itr, itr + 1);
                        shouldAdvance = false;
                        break;
                    }
                }
                // Otherwise we're in a bit of an awkward situation. There's no definition guard for structs, so we
                // can't pull in the type and define it here, which means that we are instead relying on the assumption
                // that there are no cyclical dependencies between the namespaces
            }
        }

        if (shouldAdvance)
        {
            ++range.first;
        }
    }

    return result;
}

std::map<std::string, attributed_type> metadata_cache::get_attributed_types(TypeDef const& type) const
{
    auto get_system_type = [&](auto&& signature) -> const metadata_type*
    {
        for (auto&& arg : signature.FixedArgs())
        {
            if (auto type_param = std::get_if<ElemSig::SystemType>(&std::get<ElemSig>(arg.value).value))
            {
                auto requiredType = type.get_cache().find_required(type_param->name);
                return &find(requiredType.TypeNamespace(), requiredType.TypeName());
            }
        }

        return {};
    };

    std::map<std::string, attributed_type> result;

    for (auto&& attribute : type.CustomAttribute())
    {
        auto attribute_name = attribute.TypeNamespaceAndName();

        if (attribute_name.first != "Windows.Foundation.Metadata")
        {
            continue;
        }

        auto signature = attribute.Value();
        attributed_type info;

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
            name = info.type->swift_type_name();
        }
        else
        {
            // The only factory that can't have a name is the default activation factory.
            // There should only be one of those, so assert that's the case.
            assert(info.activatable);
            assert(result.find(name) == result.end());
        }

        result[name] = std::move(info);
    }

    return result;
}
