#pragma once

namespace swiftwinrt
{
    using namespace std::filesystem;
    using namespace winmd::reader;
    using namespace std::literals;

    constexpr std::string_view system_namespace = "System";
    constexpr std::string_view foundation_namespace = "Windows.Foundation";
    constexpr std::string_view collections_namespace = "Windows.Foundation.Collections";
    constexpr std::string_view metadata_namespace = "Windows.Foundation.Metadata";

    static auto remove_tick(std::string_view const& name)
    {
        return name.substr(0, name.rfind('`'));
    }

    template <typename First, typename...Rest>
    auto get_impl_name(First const& first, Rest const&... rest)
    {
        std::string result;

        auto convert = [&](auto&& value)
        {
            for (auto&& c : value)
            {
                result += c == '.' ? '_' : c;
            }
        };

        convert(first);
        ((result += '_', convert(rest)), ...);
        return result;
    }

    namespace details
    {
        inline void append_type_prefix(std::string& result, TypeDef const& type)
        {
            if ((get_category(type) == category::delegate_type) && (type.TypeNamespace() != collections_namespace))
            {
                // All delegates except those in the 'Windows.Foundation.Collections' namespace get an 'I' appended to the
                // front...
                result.push_back('I');
            }
        }

        template <bool IsGenericParam>
        void append_mangled_name(std::string& result, std::string_view name)
        {
            result.reserve(result.length() + name.length());
            for (auto ch : name)
            {
                if (ch == '.')
                {
                    result += IsGenericParam ? "__C" : "_C";
                }
                else if (ch == '_')
                {
                    result += IsGenericParam ? "__z" : "__";
                }
                else if (ch == '`')
                {
                    result += '_';
                }
                else
                {
                    result += ch;
                }
            }
        }

        inline bool is_generic(TypeDef const& type) noexcept
        {
            return distance(type.GenericParam()) != 0;
        }
    }

    template <bool IsGenericParam>
    inline std::string mangled_namespace(std::string_view typeNamespace)
    {
        if constexpr (IsGenericParam)
        {
            if (typeNamespace == collections_namespace)
            {
                return "__F";
            }
        }

        std::string result;
        details::append_mangled_name<IsGenericParam>(result, typeNamespace);
        return result;
    }

    template <bool IsGenericParam>
    inline std::string mangled_name(TypeDef const& type, ns_prefix ns_prefix_state)
    {
        std::string result = ns_prefix_state == ns_prefix::always ? "__x_ABI_C" : "__x_";

        if (details::is_generic(type))
        {
            // Generic types don't have the namespace included in the mangled name
            result += "__F";
        }
        else
        {
            result += mangled_namespace<IsGenericParam>(type.TypeNamespace());
            result += IsGenericParam ? "__C" : "_C";
        }

        details::append_type_prefix(result, type);
        details::append_mangled_name<IsGenericParam>(result, type.TypeName());
        return result;
    }

    struct writer : indented_writer_base<writer>
    {
        using writer_base<writer>::write;

        struct depends_compare
        {
            bool operator()(TypeDef const& left, TypeDef const& right) const
            {
                return left.TypeName() < right.TypeName();
            }
        };

        std::string type_namespace;
        ns_prefix ns_prefix_state = ns_prefix::always;
        bool abi_types{};
        bool delegate_types{};
        bool consume_types{};
        bool async_types{};
        std::map<std::string_view, std::set<TypeDef, depends_compare>> depends;
        std::vector<std::vector<std::string>> generic_param_stack;

        struct generic_param_guard
        {
            explicit generic_param_guard(writer* arg = nullptr)
                : owner(arg)
            {}

            ~generic_param_guard()
            {
                if (owner)
                {
                    owner->generic_param_stack.pop_back();
                }
            }

            generic_param_guard(generic_param_guard&& other)
                : owner(other.owner)
            {
                other.owner = nullptr;
            }

            generic_param_guard& operator=(generic_param_guard&& other)
            {
                owner = std::exchange(other.owner, nullptr);
                return *this;
            }

            generic_param_guard& operator=(generic_param_guard const&) = delete;
            writer* owner;
        };

        template<typename T>
        struct member_value_guard
        {
            writer* const owner;
            T writer::* const member;
            T const previous;
            explicit member_value_guard(writer* arg, T writer::* ptr, T value) :
                owner(arg), member(ptr), previous(std::exchange(owner->*member, value))
            {
            }

            ~member_value_guard()
            {
                owner->*member = previous;
            }

            member_value_guard(member_value_guard const&) = delete;
            member_value_guard& operator=(member_value_guard const&) = delete;

        };

        void add_depends(TypeDef const& type)
        {
            auto ns = type.TypeNamespace();

            if (ns != type_namespace)
            {
                depends[ns].insert(type);
            }
        }

        [[nodiscard]] auto push_generic_params(std::pair<GenericParam, GenericParam> const& params)
        {
            if (empty(params))
            {
                return generic_param_guard{ nullptr };
            }

            std::vector<std::string> names;

            for (auto&& param : params)
            {
                names.push_back(std::string{ param.Name() });
            }

            generic_param_stack.push_back(std::move(names));
            return generic_param_guard{ this };
        }

        [[nodiscard]] auto push_generic_params(GenericTypeInstSig const& signature)
        {
            std::vector<std::string> names;

            for (auto&& arg : signature.GenericArgs())
            {
                names.push_back(write_temp("%", arg));
            }

            generic_param_stack.push_back(std::move(names));
            return generic_param_guard{ this };
        }

        [[nodiscard]] auto push_abi_types(bool value)
        {
            return member_value_guard(this, &writer::abi_types, value);
        }

        [[nodiscard]] auto push_async_types(bool value)
        {
            return member_value_guard(this, &writer::async_types, value);
        }

        [[nodiscard]] auto push_delegate_types(bool value)
        {
            return member_value_guard(this, &writer::delegate_types, value);
        }

        [[nodiscard]] auto push_consume_types(bool value)
        {
            return member_value_guard(this, &writer::consume_types, value);
        }

        void write(indent value)
        {
            for (std::size_t i = 0; i < value.additional_indentation; ++i)
            {
                write("    ");
            }
        }

        void write_value(int32_t value)
        {
            write_printf("%d", value);
        }

        void write_value(uint32_t value)
        {
            write_printf("%#0x", value);
        }

        void write_code(std::string_view const& value)
        {
            for (auto&& c : value)
            {
                if (c == '`')
                {
                    return;
                }
                else
                {
                    write(c);
                }
            }
        }

        void write(Constant const& value)
        {
            switch (value.Type())
            {
            case ConstantType::Int32:
                write_value(value.ValueInt32());
                break;
            case ConstantType::UInt32:
                write_value(value.ValueUInt32());
                break;
            default:
                throw std::invalid_argument("Unexpected constant type");
            }
        }

        void write(TypeDef const& type)
        {
            add_depends(type);
            auto ns = type.TypeNamespace();
            auto name = type.TypeName();
            auto generics = type.GenericParam();

            if (!empty(generics))
            {
                XLANG_ASSERT("**TODO: write generic type **");
                return;
            }

            if (name == "HResult" && ns == "Windows.Foundation")
            {
                write("HRESULT");
            }
            else if (abi_types)
            {
                write(mangled_name<false>(type, ns_prefix_state));
            }
            else
            {
                write("%", name);
            }
        }

        void write(TypeRef const& type)
        {
            if (type_name(type) == "System.Guid")
            {
                write("winrt::guid");
            }
            else
            {
                write(find_required(type));
            }
        }

        void write(GenericParam const& param)
        {
            write(param.Name());
        }

        void write(coded_index<TypeDefOrRef> const& type)
        {
            switch (type.type())
            {
            case TypeDefOrRef::TypeDef:
                write(type.TypeDef());
                break;
            case TypeDefOrRef::TypeRef:
                write(type.TypeRef());
                break;
            case TypeDefOrRef::TypeSpec:
                write(type.TypeSpec().Signature().GenericTypeInst());
                break;
            }
        }

        void write(GenericTypeInstSig const& type)
        {
            if (abi_types)
            {
                write("void*");
            }
            else
            {
                auto generic_type = type.GenericType();
                auto[ns, name] = get_type_namespace_and_name(generic_type);
                name.remove_suffix(name.size() - name.rfind('`'));
                add_depends(find_required(generic_type));

                if (consume_types)
                {
                    static constexpr std::string_view iterable("winrt::Windows::Foundation::Collections::IIterable<"sv);
                    static constexpr std::string_view vector_view("winrt::Windows::Foundation::Collections::IVectorView<"sv);
                    static constexpr std::string_view map_view("winrt::Windows::Foundation::Collections::IMapView<"sv);
                    static constexpr std::string_view vector("winrt::Windows::Foundation::Collections::IVector<"sv);
                    static constexpr std::string_view map("winrt::Windows::Foundation::Collections::IMap<"sv);

                    consume_types = false;
                    auto full_name = write_temp("winrt::@::%<%>", ns, name, bind_list(", ", type.GenericArgs()));
                    consume_types = true;

                    if (starts_with(full_name, iterable))
                    {
                        if (async_types)
                        {
                            write("param::async_iterable%", full_name.substr(iterable.size() - 1));
                        }
                        else
                        {
                            write("param::iterable%", full_name.substr(iterable.size() - 1));
                        }
                    }
                    else if (starts_with(full_name, vector_view))
                    {
                        if (async_types)
                        {
                            write("param::async_vector_view%", full_name.substr(vector_view.size() - 1));
                        }
                        else
                        {
                            write("param::vector_view%", full_name.substr(vector_view.size() - 1));
                        }
                    }

                    else if (starts_with(full_name, map_view))
                    {
                        if (async_types)
                        {
                            write("param::async_map_view%", full_name.substr(map_view.size() - 1));
                        }
                        else
                        {
                            write("param::map_view%", full_name.substr(map_view.size() - 1));
                        }
                    }
                    else if (starts_with(full_name, vector))
                    {
                        write("param::vector%", full_name.substr(vector.size() - 1));
                    }
                    else if (starts_with(full_name, map))
                    {
                        write("param::map%", full_name.substr(map.size() - 1));
                    }
                    else
                    {
                        write(full_name);
                    }
                }
                else
                {
                    write("winrt::@::%<%>", ns, name, bind_list(", ", type.GenericArgs()));
                }
            }
        }

        void write_abi(ElementType type)
        {
            assert(abi_types);

            if (type == ElementType::Boolean){ write("boolean"); }
            else if (type == ElementType::Char) { write("CHAR"); }
            else if (type == ElementType::I1) { write("INT8"); }
            else if (type == ElementType::U1) { write("UINT8"); }
            else if (type == ElementType::I2) { write("INT16"); }
            else if (type == ElementType::U2) { write("UINT16"); }
            else if (type == ElementType::I4) { write("INT32"); }
            else if (type == ElementType::U4) { write("UINT32"); }
            else if (type == ElementType::I8) { write("INT64"); }
            else if (type == ElementType::U8) { write("UINT64"); }
            else if (type == ElementType::R4) { write("FLOAT"); }
            else if (type == ElementType::R8) { write("DOUBLE"); }
            else if (type == ElementType::String) { write("HSTRING"); }
            else if (type == ElementType::Object) { write("WinSDK.IInspectable"); }
            else
            {
                assert(false);
            }
        }

        void write_swift(ElementType type)
        {
            if (type == ElementType::Boolean) { write("Bool"); }
            else if (type == ElementType::Char) { write("Char"); }
            else if (type == ElementType::I1) { write("Int8"); }
            else if (type == ElementType::U1) { write("UInt8"); }
            else if (type == ElementType::I2) { write("Int16"); }
            else if (type == ElementType::U2) { write("UInt16"); }
            else if (type == ElementType::I4) { write("Int32"); }
            else if (type == ElementType::U4) { write("UInt32"); }
            else if (type == ElementType::I8) { write("Int64"); }
            else if (type == ElementType::U8) { write("UInt64"); }
            else if (type == ElementType::R4) { write("Float"); }
            else if (type == ElementType::R8) { write("Double"); }
            else if (type == ElementType::String) { write("String"); }
            else if (type == ElementType::Object) { write("IInspectable"); }
            else
            {
                assert(false);
            }
        }

        void write(TypeSig::value_type const& type)
        {
            call(type,
                [&](ElementType type)
                {
                    if (abi_types)
                    {
                        write_abi(type);
                    }
                    else
                    {
                        write_swift(type);
                    }
                },
                [&](GenericTypeIndex var)
                {
                    write(generic_param_stack.back()[var.index]);
                },
                    [&](auto&& type)
                {
                    write(type);
                });
        }

        void write(TypeSig const& signature)
        {
            if (!abi_types && signature.is_szarray())
            {
                write("com_array<%>", signature.Type());
            }
            else if (abi_types)
            {
                auto category = get_category(signature);

                if (category == param_category::object_type)
                {
                    if (auto default_interface = get_default_interface(signature))
                    {
                        write("UnsafeMutablePointer<%>", default_interface);
                    }
                    else
                    {
                        write("UnsafeMutablePointer<%>", signature.Type());
                    }
                }
                else
                {
                    write(signature.Type());
                }
            }
            else
            {
                write(signature.Type());
            }
        }

        void write(RetTypeSig const& value)
        {
            if (value)
            {
                auto category = get_category(value.Type());
                if (abi_types && (category == param_category::object_type || category == param_category::string_type))
                {
                    write("%?", value.Type());
                }
                else
                {
                    write(value.Type());
                }
            }
            else
            {
                write("");
            }
        }

        void write(Field const& value)
        {
            write(value.Signature().Type());
        }

        void write_root_include(std::string_view const& include)
        {
            auto format = R"(#include %winrt/%.h%
)";

            write(format,
                settings.brackets ? '<' : '\"',
                include,
                settings.brackets ? '>' : '\"');
        }

        void write_depends(std::string_view const& ns, char impl = 0)
        {
            if (impl)
            {
                write_root_include(write_temp("impl/%.%", ns, impl));
            }
            else
            {
                write_root_include(ns);
            }
        }

        void save_file(std::string_view const& ext = "")
        {
            auto filename{ settings.output_folder + "winrt/" };

            filename += type_namespace;

            if (!ext.empty())
            {
                filename += '+';
                filename += ext;
            }

            filename += ".swift";
            flush_to_file(filename);
        }
    };
}
