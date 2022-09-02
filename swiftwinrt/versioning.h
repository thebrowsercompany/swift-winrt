#pragma once
#include "attributes.h"
#include <optional>

namespace swiftwinrt
{
    struct contract_version
    {
        std::string_view name;
        uint32_t version;
    };

    struct previous_contract
    {
        std::string_view contract_from;
        uint32_t version_low;
        uint32_t version_high;
        std::string_view contract_to;
    };

    struct contract_history
    {
        contract_version current_contract;

        // Sorted such that the first entry is the first contract the type was introduced in
        std::vector<previous_contract> previous_contracts;
    };

    static contract_version decode_contract_version_attribute(winmd::reader::CustomAttribute const& attribute)
    {
        // ContractVersionAttribute has three constructors, but only two we care about here:
        //      .ctor(string contract, uint32 version)
        //      .ctor(System.Type contract, uint32 version)
        auto signature = attribute.Value();
        auto& args = signature.FixedArgs();
        assert(args.size() == 2);

        contract_version result{};
        result.version = get_integer_attribute<uint32_t>(args[1]);
        call(std::get<ElemSig>(args[0].value).value,
            [&](ElemSig::SystemType t)
            {
                result.name = t.name;
            },
            [&](std::string_view name)
            {
                result.name = name;
            },
            [](auto&&)
            {
                assert(false);
            });

        return result;
    }

    static previous_contract decode_previous_contract_attribute(CustomAttribute const& attribute)
    {
        // PreviousContractVersionAttribute has two constructors:
        //      .ctor(string fromContract, uint32 versionLow, uint32 versionHigh)
        //      .ctor(string fromContract, uint32 versionLow, uint32 versionHigh, string contractTo)
        auto signature = attribute.Value();
        auto& args = signature.FixedArgs();
        assert(args.size() >= 3);

        previous_contract result{};
        result.contract_from = get_attribute_value<std::string_view>(args[0]);
        result.version_low = get_integer_attribute<uint32_t>(args[1]);
        result.version_high = get_integer_attribute<uint32_t>(args[2]);
        if (args.size() == 4)
        {
            result.contract_to = get_attribute_value<std::string_view>(args[3]);
        }

        return result;
    }

    // Roughly corresponds to Windows.Foundation.Metadata.Platform enum
    enum class meta_platform : int
    {
        windows = 0,
        windows_phone = 1,
    };

    struct platform_version
    {
        meta_platform platform;
        std::uint32_t version;
    };

    inline platform_version decode_platform_version(winmd::reader::CustomAttribute const& versionAttr)
    {
        using namespace winmd::reader;

        // There are two constructors for the VersionAttribute: one that takes a single version integer and another that
        // takes a version integer and a platform value
        auto sig = versionAttr.Value();
        auto& args = sig.FixedArgs();
        auto version = decode_integer<std::uint32_t>(std::get<ElemSig>(args[0].value).value);

        auto platform = meta_platform::windows;
        if (args.size() == 2)
        {
            platform = decode_enum<meta_platform>(std::get<ElemSig::EnumValue>(std::get<ElemSig>(args[1].value).value));
        }
        else
        {
            XLANG_ASSERT(args.size() == 1);
        }

        return platform_version{ platform, version };
    }

    using version = std::variant<contract_version, platform_version>;

    inline version version_from_attribute(winmd::reader::CustomAttribute const& attr)
    {
        using namespace winmd::reader;

        auto sig = attr.Value();
        auto& args = sig.FixedArgs();
        auto throw_invalid = [&]
        {
            auto [ns, name] = attr.TypeNamespaceAndName();
            swiftwinrt::throw_invalid("Attribute \"", ns, ".", name, "\" has no versioning information");
        };
        if (args.empty()) throw_invalid();

        // Whenever an attribute has versioning information, its constructor will come in one of three forms:
        //  1.  attribute(args..., version)
        //  2.  attribute(args..., version, platform)
        //  3.  attribute(args..., version, contract)
        auto& lastElem = std::get<ElemSig>(args.back().value);
        if (std::holds_alternative<std::string_view>(lastElem.value))
        {
            // Scenario '3' from above
            if (args.size() < 2) throw_invalid();

            auto contractName = std::get<std::string_view>(lastElem.value);
            auto version = decode_integer<std::uint32_t>(std::get<ElemSig>(args[args.size() - 2].value).value);
            return contract_version{ contractName, version };
        }
        else if (std::holds_alternative<ElemSig::EnumValue>(lastElem.value))
        {
            // Scenario '2' from above
            if (args.size() < 2) throw_invalid();

            auto platform = decode_enum<meta_platform>(std::get<ElemSig::EnumValue>(lastElem.value));
            auto version = decode_integer<std::uint32_t>(std::get<ElemSig>(args[args.size() - 2].value).value);
            return platform_version{ platform, version };
        }
        else
        {
            // Assume scenario '1' from above. This will throw if not an integer, but that's for the best
            return platform_version{ meta_platform::windows, decode_integer<std::uint32_t>(lastElem.value) };
        }
    }


    static contract_version get_initial_contract_version(TypeDef const& type)
    {
        // Most types don't have previous contracts, so optimize for that scenario to avoid unnecessary allocations
        contract_version current_contract{};

        // The initial contract, assuming the type has moved contracts, is the only contract name that doesn't appear as
        // a "to contract" argument to a PreviousContractVersionAttribute. Note that this assumes that a type does not
        // "return" to a prior contract, however this is a restriction enforced by midlrt
        std::vector<contract_version> previous_contracts;
        std::vector<std::string_view> to_contracts;
        for (auto&& attribute : type.CustomAttribute())
        {
            auto [ns, name] = attribute.TypeNamespaceAndName();
            if (ns != "Windows.Foundation.Metadata")
            {
                continue;
            }

            if (name == "ContractVersionAttribute")
            {
                assert(current_contract.name.empty());
                current_contract = decode_contract_version_attribute(attribute);
            }
            else if (name == "PreviousContractVersionAttribute")
            {
                auto prev = decode_previous_contract_attribute(attribute);

                // If this contract was the target of an earlier contract change, we know this isn't the initial one
                if (std::find(to_contracts.begin(), to_contracts.end(), prev.contract_from) == to_contracts.end())
                {
                    previous_contracts.push_back(contract_version{ prev.contract_from, prev.version_low });
                }

                if (!prev.contract_to.empty())
                {
                    auto itr = std::find_if(previous_contracts.begin(), previous_contracts.end(), [&](auto const& ver)
                    {
                        return ver.name == prev.contract_to;
                    });
                    if (itr != previous_contracts.end())
                    {
                        *itr = previous_contracts.back();
                        previous_contracts.pop_back();
                    }

                    to_contracts.push_back(prev.contract_to);
                }
            }
            else if (name == "VersionAttribute")
            {
                // Prefer contract versioning, if present. Otherwise, use an empty contract name to indicate that this
                // is not a contract version
                if (current_contract.name.empty())
                {
                    current_contract.version = get_attribute_value<uint32_t>(attribute, 0);
                }
            }
        }

        if (!previous_contracts.empty())
        {
            assert(previous_contracts.size() == 1);
            return previous_contracts[0];
        }

        return current_contract;
    }

    template <typename T>
    inline std::optional<contract_history> get_contract_history(T const& value)
    {
        auto contractAttr = get_attribute(value, "Windows.Foundation.Metadata", "ContractVersionAttribute");
        if (!contractAttr)
        {
            return std::nullopt;
        }

        contract_history result;
        result.current_contract = decode_contract_version_attribute(contractAttr);

        std::size_t mostRecentContractIndex = std::numeric_limits<std::size_t>::max();
        for_each_attribute(value, "Windows.Foundation.Metadata", "PreviousContractVersionAttribute",
            [&](bool /*first*/, auto const& attr)
            {
                auto prevSig = attr.Value();
                auto const& prevArgs = prevSig.FixedArgs();

                // The PreviousContractVersionAttribute has two constructors, both of which start with the same three
                // arguments - the only ones that we care about
                previous_contract prev =
                {
                    std::get<std::string_view>(std::get<ElemSig>(prevArgs[0].value).value),
                    decode_integer<std::uint32_t>(std::get<ElemSig>(prevArgs[1].value).value),
                    decode_integer<std::uint32_t>(std::get<ElemSig>(prevArgs[2].value).value),
                };
                if (prevArgs.size() == 4)
                {
                    prev.contract_to = std::get<std::string_view>(std::get<ElemSig>(prevArgs[3].value).value);
                    if (prev.contract_to == result.current_contract.name)
                    {
                        XLANG_ASSERT(mostRecentContractIndex == std::numeric_limits<std::size_t>::max());
                        mostRecentContractIndex = result.previous_contracts.size();
                    }

                    result.previous_contracts.push_back(prev);
                }
                else
                {
                    // This is the last contract that the type was in before moving to its current contract. Always insert
                    // it at the tail
                    prev.contract_to = result.current_contract.name;
                    XLANG_ASSERT(mostRecentContractIndex == std::numeric_limits<std::size_t>::max());
                    mostRecentContractIndex = result.previous_contracts.size();
                    result.previous_contracts.push_back(prev);
                }
            });

        if (!result.previous_contracts.empty())
        {
            if (mostRecentContractIndex == std::numeric_limits<std::size_t>::max())
            {
                // Not a great error message, however this scenario should not be allowable by mildlrt
                swiftwinrt::throw_invalid("Invalid contract history");
            }

            // NOTE: The following loop is N^2, however contract history should be rare and short when present, so this
            // should likely beat out any alternative graph creating algorithm in terms of wall clock execution time
            std::swap(result.previous_contracts[mostRecentContractIndex], result.previous_contracts.back());
            for (std::size_t size = result.previous_contracts.size() - 1; size; --size)
            {
                auto& last = result.previous_contracts[size];
                auto end = result.previous_contracts.begin() + size;
                auto itr = std::find_if(result.previous_contracts.begin(), end, [&](auto const& test)
                    {
                        return test.contract_to == last.contract_from;
                    });
                if (itr == end)
                {
                    swiftwinrt::throw_invalid("Invalid contract history");
                }
                std::swap(*itr, result.previous_contracts[size - 1]);
            }
        }

        return result;
    }


    template <typename T>
    std::optional<version> match_versioning_scheme(version const& ver, T const& value)
    {
        using namespace std::literals;

        if (std::holds_alternative<contract_version>(ver))
        {
            if (auto history = get_contract_history(value))
            {
                if (history->previous_contracts.empty())
                {
                    return history->current_contract;
                }

                auto range = history->previous_contracts.front();
                return contract_version{ range.contract_from, range.version_low };
            }

            return std::nullopt;
        }
        else
        {
            auto const& plat = std::get<platform_version>(ver);
            std::optional<version> result;
            for_each_attribute(value, metadata_namespace, "VersionAttribute"sv, [&](bool /*first*/, auto const& attr)
                {
                    auto possibleMatch = decode_platform_version(attr);
                    if (possibleMatch.platform == plat.platform)
                    {
                        XLANG_ASSERT(!result);
                        result = possibleMatch;
                    }
                });

            return result;
        }
    }

}