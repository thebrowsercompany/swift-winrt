#pragma once

namespace swiftwinrt
{
    namespace impl
    {
        template <typename T, typename... Types>
        struct variant_index;

        template <typename T, typename First, typename... Types>
        struct variant_index<T, First, Types...>
        {
            static constexpr bool found = std::is_same_v<T, First>;
            static constexpr std::size_t value = std::conditional_t<found,
                std::integral_constant<std::size_t, 0>,
                variant_index<T, Types...>>::value + (found ? 0 : 1);
        };
    }

    template <typename Variant, typename T>
    struct variant_index;

    template <typename... Types, typename T>
    struct variant_index<std::variant<Types...>, T> : impl::variant_index<T, Types...>
    {
    };

    template <typename Variant, typename T>
    constexpr std::size_t variant_index_v = variant_index<Variant, T>::value;

    template <typename T>
    auto get_integer_attribute(FixedArgSig const& signature)
    {
        auto variant = std::get<ElemSig>(signature.value).value;
        switch (variant.index())
        {
        case variant_index_v<decltype(variant), std::make_unsigned_t<T>>: return static_cast<T>(std::get<std::make_unsigned_t<T>>(variant));
        case variant_index_v<decltype(variant), std::make_signed_t<T>>: return static_cast<T>(std::get<std::make_signed_t<T>>(variant));
        default: return std::get<T>(variant); // Likely throws, but that's intentional
        }
    }

    template <typename T>
    auto get_attribute_value(FixedArgSig const& signature)
    {
        return std::get<T>(std::get<ElemSig>(signature.value).value);
    }

    template <typename T>
    auto get_attribute_value(CustomAttribute const& attribute, uint32_t const arg)
    {
        return get_attribute_value<T>(attribute.Value().FixedArgs()[arg]);
    }

    // Constructor arguments are not consistently encoded using the same type (e.g. using a signed 32-bit integer as an
    // argument to a constructor that takes an unsigned 32-bit integer)
    template <typename T, typename Variant>
    T decode_integer(Variant const& value)
    {
        switch (value.index())
        {
        case variant_index_v<Variant, std::int8_t>: return static_cast<T>(std::get<std::int8_t>(value));
        case variant_index_v<Variant, std::uint8_t>: return static_cast<T>(std::get<std::uint8_t>(value));
        case variant_index_v<Variant, std::int16_t>: return static_cast<T>(std::get<std::int16_t>(value));
        case variant_index_v<Variant, std::uint16_t>: return static_cast<T>(std::get<std::uint16_t>(value));
        case variant_index_v<Variant, std::int32_t>: return static_cast<T>(std::get<std::int32_t>(value));
        case variant_index_v<Variant, std::uint32_t>: return static_cast<T>(std::get<std::uint32_t>(value));
        case variant_index_v<Variant, std::int64_t>: return static_cast<T>(std::get<std::int64_t>(value));
        case variant_index_v<Variant, std::uint64_t>: return static_cast<T>(std::get<std::uint64_t>(value));
        case variant_index_v<Variant, char16_t>: return static_cast<T>(std::get<char16_t>(value));
        default: return std::get<T>(value); // This should throw, but that's intentional
        }
    }

    template <typename T>
    struct type_identity
    {
        using type = T;
    };

    template <typename T>
    T decode_enum(winmd::reader::ElemSig::EnumValue const& value)
    {
        using integral_type = typename std::conditional_t<std::is_enum_v<T>, std::underlying_type<T>, type_identity<T>>::type;
        return static_cast<T>(decode_integer<integral_type>(value.value));
    }

    template <typename T, typename Func>
    inline void for_each_attribute(
        T const& type,
        std::string_view namespaceFilter,
        std::string_view typeNameFilter,
        Func&& func)
    {
        bool first = true;
        for (auto const& attr : type.CustomAttribute())
        {
            auto [ns, name] = attr.TypeNamespaceAndName();
            if ((ns == namespaceFilter) && (name == typeNameFilter))
            {
                func(first, attr);
                first = false;
            }
        }
    }

}