#pragma once
#include "type_helpers.h"
#include "settings.h"
#include "metadata_filter.h"
namespace swiftwinrt
{
    using namespace std::filesystem;
    using namespace winmd::reader;
    using namespace std::literals;

    inline std::string_view remove_tick(std::string_view const& name)
    {
        return name.substr(0, name.rfind('`'));
    }

    struct writer;
    std::string get_full_swift_type_name(writer const&, TypeDef const& type);
    std::string get_full_swift_type_name(writer const&, const metadata_type* type);
    std::string get_swift_module(std::string_view const& ns);
    std::string_view get_swift_name(function_param const&);

    param_category get_category(const metadata_type*, TypeDef*);

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
            if ((get_category(type) == category::delegate_type) && (type.TypeNamespace() != winrt_collections_namespace))
            {
                // All delegates except those in the 'Windows.Foundation.Collections' namespace get an 'I' appended to the
                // front...
                result.push_back('I');
            }
        }

        inline void append_type_prefix(std::string& result, metadata_type const& type)
        {
            if (dynamic_cast<const delegate_type*>(&type) != nullptr && (type.swift_logical_namespace() != winrt_collections_namespace))
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
            if (typeNamespace == winrt_collections_namespace)
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

    template <bool IsGenericParam>
    inline std::string mangled_name(metadata_type const& type)
    {
        std::string result = "__x_ABI_C";

        if (is_generic_def(type) || is_generic_inst(type))
        {
            // Generic types don't have the namespace included in the mangled name
            result += "__F";
        }
        else
        {
            result += mangled_namespace<IsGenericParam>(type.swift_abi_namespace());
            result += IsGenericParam ? "__C" : "_C";
        }

        details::append_type_prefix(result, type);
        details::append_mangled_name<IsGenericParam>(result, type.swift_type_name());
        return result;
    }

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
        bool writing_generic{};

        std::set<std::string> depends;
        std::set<winmd::reader::Event> events;
        std::vector<generic_param_vector> generic_param_stack;
        swiftwinrt::metadata_filter filter;
        swiftwinrt::metadata_cache const* cache;

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

        [[nodiscard]] auto push_generic_params(generic_inst const& signature)
        {
            generic_param_vector names = signature.generic_params();
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


        [[nodiscard]] auto push_mangled_names_if_needed(param_category forCategory)
        {
            if (!abi_types)
            {
                return push_mangled_names(false);
            }
            // generics, objects (interfaces, classes), structs and enums all
            // need the mangled name. other fundamental types just need the cpp
            // abi type
            switch (forCategory) {
            case param_category::generic_type:
            case param_category::object_type:
            case param_category::struct_type:
            case param_category::enum_type:
                return push_mangled_names(true);
            default:
                return push_mangled_names(false);
            }
        }


        [[nodiscard]] auto push_writing_generic(bool value)
        {
            return member_value_guard(this, &writer::writing_generic, value);
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
            if (auto gti = dynamic_cast<const generic_inst*>(type))
            {
                write(gti);
                return;
            }
            if (is_generic_def(type))
            {
                auto name = type->swift_type_name();
                auto guard{ push_writing_generic(true) };
                auto typeName = mangled_names ? mangled_name<true>(*type) : std::string(remove_tick(name));
                bool first = !mangled_names;
                write("%%", typeName, bind_each([&](writer& w, generic_param_type generic_param) {
                    if (first)
                    {
                        first = false;
                    }
                    else
                    {
                        w.write("_");
                    }

                    w.write(generic_param);
                    },
                    generic_param_stack.back()));
                return;
            }

            if (auto mapped = dynamic_cast<mapped_type const*>(type))
            {
                // mapped types are defined in headers and *not* metadata files, so these don't follow the same
                // naming conventions that other types do. We just grab the type name and will use that.
                auto swiftTypeName = mapped->swift_type_name();
                if (swiftTypeName == "HResult")
                {
                    write("HRESULT");
                }
                else
                {
                    write(swiftTypeName);
                }
            }
            else if (mangled_names)
            {
                auto classType = dynamic_cast<const class_type*>(type);
                if (classType && !writing_generic)
                {
                    // not writing a generic, write the default interface instead. class names in a generic 
                    // use the mangled name of the class type
                    write(classType->default_interface->mangled_name());
                }
                else if (writing_generic)
                {
                    write(type->generic_param_mangled_name());
                }
                else
                {
                    auto name = type->mangled_name();
                    if (name == "IInspectable" && !writing_generic)
                    {
                        write("%.IInspectable", c_mod);
                    }
                    else
                    {
                        write(name);
                    }
                }
            }
            // In generics, don't use the ABI name or we get IVectorIBase instead of IVectorBase
            else if (abi_types && !writing_generic)
            {
                write(type->cpp_abi_name());
            }
            else if (type->swift_logical_namespace() != type_namespace || full_type_names)
            {
                write(get_full_swift_type_name(*this, type));
            }
            else
            {
                write(type->swift_type_name());
            }
        }

        void write(metadata_type const& type)
        {
            write(&type);
        }

        void write_abi(ElementType type)
        {
            assert(mangled_names || abi_types);

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

        // duplicated with the names defined in types.cpp
        void write_mangled(ElementType type)
        {
            assert(writing_generic);
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

        void write(ElementType type)
        {
            if (mangled_names && writing_generic)
            {
                write_mangled(type);
            }
            else if (mangled_names || abi_types)
            {
                write_abi(type);
            }
            else
            {
                write_swift(type);
            }
        }

        void write(generic_inst const& generic)
        {
            auto guard{ push_writing_generic(true) };

            if (mangled_names)
            {
                write(generic.mangled_name());
            }
            else
            {
                if (abi_types)
                {
                    // Consolidate with implementation in write(metadata_type)
                    auto guard = push_generic_params(generic);
                    write(generic.generic_type());
                }
                else
                {
                    write("%<", remove_tick(generic.generic_type()->swift_type_name()));
                    bool first = true;
                    for (auto&& generic_arg : generic.generic_params()) {
                        if (!first) write(", ");
                        write(generic_arg);
                        first = false;
                    }
                    write(">");
                }
            }
        }

        void write(const generic_inst* generic)
        {
            write(*generic);
        }
        
        std::filesystem::path root_directory()
        {
            return settings.output_folder ;
        }

        std::filesystem::path project_directory()
        {
            if (settings.test)
            {
                return root_directory() / settings.support;
            }
            else
            {
                return root_directory() / get_swift_module(type_namespace);
            }
        }

        void save_file(std::string_view const& ext = "")
        {
            auto filename = type_namespace;

            if (!ext.empty())
            {
                filename += '+';
                filename += ext;
            }

            filename += ".swift";
            flush_to_file(project_directory() / filename);
        }

        void save_header()
        {
            flush_to_file(root_directory() / "CWinRT" / "include" / (type_namespace + ".h"));
        }

        void save_cmake()
        {
            flush_to_file(project_directory() / "CMakeLists.txt");
        }
    };
}
