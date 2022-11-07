#pragma once
#include "type_helpers.h"
#include "settings.h"
#include "metadata_filter.h"
namespace swiftwinrt
{
    using namespace std::filesystem;
    using namespace winmd::reader;
    using namespace std::literals;

    static auto remove_tick(std::string_view const& name)
    {
        return name.substr(0, name.rfind('`'));
    }

    class writer;
    std::string get_full_swift_type_name(writer const&, TypeDef const& type);
    std::string get_full_swift_type_name(writer const&, const metadata_type* type);
    std::string get_swift_module(std::string_view const& ns);

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
    inline std::string mangled_name(TypeDef const& type)
    {
        std::string result = "__x_ABI_C";

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

    enum class valuetype_copy_semantics
    {
        equal,
        blittable,
        nonblittable
    };

    using generic_param_type = std::variant<const metadata_type*, TypeSig>;
    using generic_param_vector = std::vector<std::pair<generic_param_type, valuetype_copy_semantics>>;

    struct writer : indented_writer_base<writer>
    {
        using writer_base<writer>::write;

        std::string type_namespace;
        std::string support;
        std::string c_mod;
        bool abi_types{};
        bool delegate_types{};
        bool consume_types{};
        bool async_types{};
        bool full_type_names{};
        bool impl_names{};
        bool mangled_names{};

        std::set<std::string> depends;
        std::set<winmd::reader::Event> events;
        std::vector<generic_param_vector> generic_param_stack;
        swiftwinrt::metadata_filter filter;

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
            auto type_module = get_swift_module(type.TypeNamespace());

            if (type_module != get_swift_module(type_namespace))
            {
                depends.insert(type_module);
            }
        }

        void add_depends(metadata_type const& type)
        {
            auto type_module = get_swift_module(type.swift_logical_namespace());

            if (type_module != get_swift_module(type_namespace) && !type_module.empty())
            {
                depends.insert(type_module);
            }
        }
        
        [[nodiscard]] auto push_generic_params(GenericTypeInstSig const& signature)
        {
            generic_param_vector names;

            for (auto&& arg : signature.GenericArgs())
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

            generic_param_stack.push_back(std::move(names));
            return generic_param_guard{ this };
        }

        [[nodiscard]] auto push_generic_params(generic_inst const& signature)
        {
            generic_param_vector names;

            for (auto&& arg : signature.generic_params())
            {
                auto semantics = valuetype_copy_semantics::equal;
                const TypeDef* underlying_type;
                if (auto element = dynamic_cast<const struct_type*>(arg))
                {
                    if (is_struct_blittable(element->type()))
                    {
                        semantics = valuetype_copy_semantics::blittable;
                    }
                    else
                    {
                        semantics = valuetype_copy_semantics::nonblittable;
                    }
                    underlying_type = &element->type();
                }
                else if (auto element = dynamic_cast<const element_type*>(arg))
                {
                    if (!element->is_blittable())
                    {
                        semantics = valuetype_copy_semantics::nonblittable;
                    }
                }
                names.emplace_back(arg, semantics);
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

        [[nodiscard]] auto push_full_type_names(bool value)
        {
            return member_value_guard(this, &writer::full_type_names, value);
        }

        [[nodiscard]] auto push_impl_names(bool value)
        {
            return member_value_guard(this, &writer::impl_names, value);
        }

        [[nodiscard]] auto push_mangled_names(bool value)
        {
            return member_value_guard(this, &writer::mangled_names, value);
        }

        private:
            enum class declaration_stage
            {
                begin,
                forward,
                end,
            };

            // A set of already declared (or in the case of generics, defined) types
            std::map<std::string_view, declaration_stage> m_declaredTypes;
        public:

        bool begin_declaration(std::string_view mangledName)
        {
            auto [itr, added] = m_declaredTypes.emplace(mangledName, declaration_stage::begin);
            return added;
        }

        void end_declaration(std::string_view mangledName)
        {
            auto itr = m_declaredTypes.find(mangledName);
            XLANG_ASSERT(itr != m_declaredTypes.end());
            XLANG_ASSERT(itr->second != declaration_stage::end);
            itr->second = declaration_stage::end;
        }

        bool should_forward_declare(std::string_view mangledName)
        {
            auto [itr, added] = m_declaredTypes.emplace(mangledName, declaration_stage::begin);
            if (itr->second != declaration_stage::begin)
            {
                return false;
            }

            itr->second = declaration_stage::forward;
            return true;
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

        void write(metadata_type const* type)
        {
            add_depends(*type);
            if (abi_types)
            {
                write(type->mangled_name());
            }
            else if (type->swift_logical_namespace() != type_namespace || full_type_names)
            {
                write(get_full_swift_type_name(*this, type));
            }
            else
            {
                write(type->cpp_logical_name());
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
            else if (name == "EventRegistrationToken" && ns == "Windows.Foundation")
            {
                write("EventRegistrationToken");
            }
            else if (name == "AsyncStatus" && ns == "Windows.Foundation")
            {
                write("AsyncStatus");
            }
            else if (name == "IAsyncInfo" && ns == "Windows.Foundation")
            {
                // IAsync info is special in that it is defined in it's own header file and doesn't have a mangled name.
                if (abi_types)
                {
                    write("%.IAsyncInfo", c_mod);
                }
                else
                {
                    write("IAsyncInfo");
                }
            }
            else if (abi_types)
            {
                // This is a bit of a hack, we should fill in the generic param stack instead,p
                // or just write the names the same way even if part of a generic definition
                if (mangled_names)
                {
                    write(mangled_name<true>(type));
                }
                else
                {
                    write(mangled_name<false>(type));
                }
            }
            else if (ns != type_namespace || full_type_names)
            {
                auto full_swift_name = get_full_swift_type_name(*this, type);
                write("%", full_swift_name);
            }
            else
            {
                write("%", name);
            }
        }

        void write(TypeRef const& type)
        {
            if (get_full_type_name(type) == "System.Guid")
            {
                if (abi_types || mangled_names)
                {
                    write("GUID");
                }
                else
                {
                    write("UUID");
                }
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
            auto generic_type = type.GenericType();
            auto generic_typedef = find_required(generic_type);
            auto [ns, name] = get_type_namespace_and_name(generic_type);
            if (abi_types)
            {
                auto mangled = push_mangled_names(true);
                auto mangledName = write_temp("%_%", mangled_name<true>(generic_typedef), bind_list("_", type.GenericArgs()));
                write(mangledName);
            }
            else
            {
                auto [ns, name] = get_type_namespace_and_name(generic_type);
                name.remove_suffix(name.size() - name.rfind('`'));

                add_depends(generic_typedef);
                if (ns == "Windows.Foundation" && name == "IReference")
                {
                    auto refType = type.GenericArgs().first->Type();
                    write("%?", refType);
                }
                else if (ns == "Windows.Foundation" && name == "TypedEventHandler")
                {
                    write("^@escaping (%) -> ()", bind_list(", ", type.GenericArgs()));
                }
                else if (ns == "Windows.Foundation" && name == "EventHandler")
                {
                    auto argsType = type.GenericArgs().first->Type();
                    write("^@escaping (%.IInspectable,%) -> ()", support, argsType);
                }
                else
                {
                    // Do nothing for types we can't write 
                    //assert(false);
                }
            }
        }

        void write_abi(ElementType type)
        {
            assert(abi_types);

            if (type == ElementType::Boolean){ write("boolean"); }
            else if (type == ElementType::Char) { write("WCHAR"); }
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
            else if (type == ElementType::Object) { write("%.IInspectable", c_mod); }
            else
            {
                assert(false);
            }
        }

        void write_swift(ElementType type)
        {
            if (type == ElementType::Boolean) { write("Bool"); }
            else if (type == ElementType::Char) { write("Character"); }
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
            else if (type == ElementType::Object) { write("%.IInspectable", support); }
            else
            {
                assert(false);
            }
        }

        // duplicated with the names defined in types.cpp
        void write_mangled(ElementType type)
        {
            if (type == ElementType::Boolean) { write("boolean"); }
            else if (type == ElementType::Char) { write("wchar__zt"); }
            else if (type == ElementType::U1) { write("byte"); }
            else if (type == ElementType::I2) { write("short"); }
            else if (type == ElementType::U2) { write("UINT16"); }
            else if (type == ElementType::I4) { write("int"); }
            else if (type == ElementType::U4) { write("UINT32"); }
            else if (type == ElementType::I8) { write("__z__zint64"); }
            else if (type == ElementType::U8) { write("UINT64"); }
            else if (type == ElementType::R4) { write("float"); }
            else if (type == ElementType::R8) { write("double"); }
            else if (type == ElementType::String) { write("HSTRING"); }
            else if (type == ElementType::Object) { write("IInspectable"); }
            else
            {
                assert(false);
            }
        }

        void write(generic_param_type const& generic_param)
        {
            if (std::holds_alternative<TypeSig>(generic_param))
            {
                write(std::get<TypeSig>(generic_param).Type());
            }
            else 
            {
                auto type = std::get<const metadata_type*>(generic_param);
                add_depends(*type);
                if (mangled_names)
                {
                    write(type->mangled_name());
                }
                else if (abi_types)
                {
                    write(type->cpp_abi_name());
                }
                else
                {
                    auto full_name = get_full_swift_type_name(*this, std::get<const metadata_type*>(generic_param));
                    if (full_name == "IInspectable")
                    {
                        write("%.IInspectable", support);
                    }
                    else
                    {
                        write(full_name);
                    }
                }
            } 
        }

        void write(TypeSig::value_type const& type)
        {
            call(type,
                [&](ElementType type)
                {
                    if (mangled_names)
                    {
                        write_mangled(type);
                    }
                    else if (abi_types)
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
                    auto variant = generic_param_stack.back()[var.index].first;
                    write(variant);
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

                if (!mangled_names && (category == param_category::object_type || category == param_category::generic_type))
                {
                    if (is_class(signature))
                    {
                        auto default_interface = get_default_interface(signature);
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
                if (abi_types && (category == param_category::object_type || category == param_category::string_type || category == param_category::generic_type))
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

        void write(generic_inst const& generic)
        {
            if (abi_types)
            {
                write(generic.mangled_name());
            }
            else
            {
                auto generic_type = generic.generic_type();
                add_depends(generic_type->type());
                if (generic_type->swift_full_name().starts_with("Windows.Foundation.IReference"))
                {
                    auto refType = generic.generic_params()[0];
                    write("%?", refType);
                }
                else if (generic_type->swift_full_name().starts_with("Windows.Foundation.TypedEventHandler"))
                {
                    auto senderType = generic.generic_params()[0];
                    auto argsType = generic.generic_params()[1];
                    write("(%,%) -> ()", senderType, argsType);
                }
                else if (generic_type->swift_full_name().starts_with("Windows.Foundation.EventHandler"))
                {
                    auto argsType = generic.generic_params()[0];
                    write("(%.IInspectable,%) -> ()", support, argsType);
                }
                else
                {
                    // Do nothing for types we can't write 
                    //assert(false);
                }
            }
        }
        
        std::string file_directory(std::string subdir)
        {
            if (settings.test)
            {
                return { settings.output_folder + subdir };
            }
            else
            {
                return { settings.output_folder + subdir + get_swift_module(type_namespace) + "/"};
            }
        }
        void save_file(std::string_view const& ext = "")
        {
            auto filename{ file_directory("/swift/") };
            filename += type_namespace;

            if (!ext.empty())
            {
                filename += '+';
                filename += ext;
            }

            filename += ".swift";
            flush_to_file(filename);
        }

        void save_header()
        {
            auto filename{ settings.output_folder + "/c/" };
            filename += type_namespace;
            filename += ".h";
            flush_to_file(filename);
        }

        void save_cmake()
        {
            auto filename{ file_directory("/swift/") };
            filename += "CMakeLists.txt";
            flush_to_file(filename);
        }
    };
}
