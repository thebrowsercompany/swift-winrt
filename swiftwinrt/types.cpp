#include "pch.h"

#include "types.h"
#include "abi_writer.h"
#include "winmd_reader.h"
#include "helpers.h"
#include "attributes.h"
#include "namespace_iterator.h"
#include "code_writers.h"

using namespace std::literals;
using namespace winmd::reader;

namespace swiftwinrt
{
    static void write_iunknown_methods(writer& w, generic_inst const& type);
    static void write_iinspectable_methods(writer& w, generic_inst const& type);

    template <typename T>
    static auto begin_type_definition(writer& w, T const& type)
    {
        if (type.is_experimental())
        {
            w.write("#if defined(ENABLE_WINRT_EXPERIMENTAL_TYPES)\n");
        }
    }

    template <typename T>
    static void end_type_definition(writer& w, T const& type)
    {
        if (type.is_experimental())
        {
            w.write("#endif // defined(ENABLE_WINRT_EXPERIMENTAL_TYPES)\n");
        }

        w.write('\n');
    }

    bool function_def::is_async() const
    {
        if (!return_type)
        {
            return false;
        }

        if (return_type.value().type->swift_logical_namespace() == winrt_foundation_namespace)
        {
            auto returnTypeName = return_type.value().type->swift_type_name();
            return returnTypeName == "IAsyncAction" ||
                returnTypeName == "IAsyncOperation`1" ||
                returnTypeName == "IAsyncActionWithProgress`1" ||
                returnTypeName == "IAsyncOperationWithProgress`2";
        }

        return false;
    }

    typedef_base::typedef_base(TypeDef const& type) :
        m_type(type),
        m_swiftFullName(get_full_type_name(type)),
        m_mangledName(swiftwinrt::mangled_name<false>(type)),
        m_genericParamMangledName(swiftwinrt::mangled_name<true>(type)),
        m_contractHistory(get_contract_history(type))
    {
        for_each_attribute(type, metadata_namespace, "VersionAttribute"sv, [&](bool, CustomAttribute const& attr)
        {
            m_platformVersions.push_back(decode_platform_version(attr));
        });
    }

    bool typedef_base::is_experimental() const
    {
        return swiftwinrt::is_experimental(m_type);
    }

    std::optional<deprecation_info> typedef_base::is_deprecated() const noexcept
    {
        return swiftwinrt::is_deprecated(m_type);
    }

    bool typedef_base::is_generic() const noexcept
    {
        return swiftwinrt::is_generic(m_type);
    }


    winmd::reader::ElementType enum_type::underlying_type() const
    {
        return underlying_enum_type(m_type);
    }

    void enum_type::write_c_forward_declaration(writer& w) const
    {
        if (!w.should_forward_declare(m_mangledName))
        {
            return;
        }

        w.write("typedef enum % %;\n\n", bind_c_type_name(*this), bind_c_type_name(*this));
    }

    void enum_type::write_c_abi_param(writer& w) const
    {
        w.write("enum %", bind_c_type_name(*this));
    }

    void enum_type::write_c_definition(writer& w) const
    {
        begin_type_definition(w, *this);

        w.write(R"^-^(enum %
    {
    )^-^", bind_c_type_name(*this));

        for (auto const& field : m_type.FieldList())
        {
            if (auto value = field.Constant())
            {
                auto isExperimental = swiftwinrt::is_experimental(field);
                if (isExperimental)
                {
                    w.write("#if defined(ENABLE_WINRT_EXPERIMENTAL_TYPES)\n");
                }
                // we use mangled names for enums because otherwise the WinAppSDK enums collide with the Windows ones
                w.write("    %_%", mangled_name(), field.Name());
     
                w.write(" = %,\n", value);
                if (isExperimental)
                {
                    w.write("#endif // defined(ENABLE_WINRT_EXPERIMENTAL_TYPES)\n");
                }
            }
        }

        w.write("};\n");
        end_type_definition(w, *this);
    }

    void struct_type::write_c_forward_declaration(writer& w) const
    {
        if (!w.should_forward_declare(m_mangledName))
        {
            return;
        }

        w.write("typedef struct % %;\n\n", bind_c_type_name(*this), bind_c_type_name(*this));
    }

    void struct_type::write_c_abi_param(writer& w) const
    {
        w.write("struct %", bind_c_type_name(*this));
    }

    void struct_type::write_c_definition(writer& w) const
    {
         begin_type_definition(w, *this);

        w.write(R"^-^(struct %
    {
    )^-^", bind_c_type_name(*this));

        for (auto const& member : members)
        {
            w.write("    % %;\n", [&](writer& w) { member.type->write_c_abi_param(w); }, member.field.Name());
        }

        w.write("};\n");

        end_type_definition(w, *this);
    }

    static std::string_view function_name(MethodDef const& def)
    {
        // If this is an overload, use the unique name
        auto fnName = def.Name();
        if (auto overloadAttr = get_attribute(def, metadata_namespace, "OverloadAttribute"sv))
        {
            auto sig = overloadAttr.Value();
            auto const& fixedArgs = sig.FixedArgs();
            XLANG_ASSERT(fixedArgs.size() == 1);
            fnName = std::get<std::string_view>(std::get<ElemSig>(fixedArgs[0].value).value);
        }

        return fnName;
    }


    template <typename T>
    static void write_c_iunknown_interface(writer& w, T const& type)
    {
        w.write(R"^-^(    HRESULT (STDMETHODCALLTYPE* QueryInterface)(%* This,
            REFIID riid,
            void** ppvObject);
        ULONG (STDMETHODCALLTYPE* AddRef)(%* This);
        ULONG (STDMETHODCALLTYPE* Release)(%* This);
    )^-^", bind_c_type_name(type), bind_c_type_name(type), bind_c_type_name(type));
    }

    template <typename T>
    static void write_c_iinspectable_interface(writer& w, T const& type)
    {
        write_c_iunknown_interface(w, type);
        w.write(R"^-^(    HRESULT (STDMETHODCALLTYPE* GetIids)(%* This,
            ULONG* iidCount,
            IID** iids);
        HRESULT (STDMETHODCALLTYPE* GetRuntimeClassName)(%* This,
            HSTRING* className);
        HRESULT (STDMETHODCALLTYPE* GetTrustLevel)(%* This,
            TrustLevel* trustLevel);
    )^-^", bind_c_type_name(type), bind_c_type_name(type), bind_c_type_name(type));
    }

    template <typename TypeName>
    static void write_c_function_declaration(writer& w, TypeName&& typeName, function_def const& func)
    {
        w.write("    HRESULT (STDMETHODCALLTYPE* %)(%* This", function_name(func.def), typeName);

        for (auto const& param : func.params)
        {
            auto refMod = param.signature.ByRef() ? "*"sv : ""sv;
            if (param.signature.Type().is_szarray())
            {
                w.write(",\n        UINT32% %Length", refMod, param.name);
                refMod = param.signature.ByRef() ? "**"sv : "*"sv;
            }

            auto constMod = is_const(param.signature) ? "const "sv : ""sv;
            w.write(",\n        %%% %",
                constMod,
                [&](writer& w) { param.type->write_c_abi_param(w); },
                refMod,
                param.name);
        }

        if (func.return_type)
        {
            auto refMod = "*"sv;
            if (func.return_type->signature.Type().is_szarray())
            {
                w.write(",\n        UINT32* %Length", func.return_type->name);
                refMod = "**"sv;
            }

            w.write(",\n        %% %",
                [&](writer& w) { func.return_type->type->write_c_abi_param(w); },
                refMod,
                func.return_type->name);
        }

        w.write(");\n");
    }

    template <typename T>
    static void write_c_interface_definition(writer& w, T const& type)
    {
        constexpr bool is_interface = std::is_same_v<T, interface_type>;
        constexpr bool is_delegate = std::is_same_v<T, delegate_type>;
        constexpr bool is_generic = std::is_same_v<T, generic_inst>;
        static_assert(is_interface || is_delegate | is_generic);

        w.write(R"^-^(typedef struct %
    {
        BEGIN_INTERFACE

    )^-^", bind_c_type_name(type, "Vtbl"));

        bool isDelegate = type.category() == category::delegate_type;
        if (isDelegate)
        {
            write_c_iunknown_interface(w, type);
        }
        else
        {
            write_c_iinspectable_interface(w, type);
        }

        for (auto const& func : type.functions)
        {
            write_c_function_declaration(w, bind_c_type_name(type), func);
        }

        if constexpr (is_interface)
        {
            if (type.fast_class)
            {
                w.write("\n// Supplemental functions added by use of the fast ABI attribute\n");

                auto fastAttr = get_attribute(type.fast_class->type(), metadata_namespace, "FastAbiAttribute"sv);

                // If the class "derives" from any other class, the fast pointer-to-base functions come first
                std::vector<class_type const*> baseClasses;
                auto base = type.fast_class->base_class;
                while (base)
                {
                    baseClasses.push_back(base);
                    base = base->base_class;
                }

                std::for_each(baseClasses.rbegin(), baseClasses.rend(), [&](class_type const* baseClass)
                {
                    w.write("    % (STDMETHODCALLTYPE* base_%)(%* This);\n",
                        [&](writer& w) { baseClass->write_c_abi_param(w); },
                        baseClass->cpp_logical_name(),
                        bind_c_type_name(type));
                });

                for (auto [iface, ver] : type.fast_class->supplemental_fast_interfaces)
                {
                    w.write("\n    // Supplemental functions added for the % interface\n", iface->swift_full_name());

                    for (auto const& func : iface->functions)
                    {
                        write_c_function_declaration(w, bind_c_type_name(type), func);
                    }
                }
            }
        }

        w.write(R"^-^(
        END_INTERFACE
    } %;

    interface %
    {
        CONST_VTBL struct %* lpVtbl;
    };

    )^-^", bind_c_type_name(type, "Vtbl"), bind_c_type_name(type), bind_c_type_name(type, "Vtbl"));

    }

    delegate_type::delegate_type(winmd::reader::TypeDef const& type) :
        typedef_base(type)
    {
        m_abiName.reserve(1 + type.TypeName().length());
        details::append_type_prefix(m_abiName, type);
        m_abiName += type.TypeName();
    }

    void delegate_type::append_signature(sha1& hash) const
    {
        using namespace std::literals;
        hash.append("delegate({"sv);
        auto iid = type_iid(m_type);
        hash.append(std::string_view{ iid.data(), iid.size() - 1 });
        hash.append("})"sv);
    }

    void delegate_type::write_c_forward_declaration(writer& w) const
    {
        if (!w.should_forward_declare(m_mangledName))
        {
            return;
        }

        w.write(R"^-^(#ifndef __%_FWD_DEFINED__
#define __%_FWD_DEFINED__
)^-^", bind_mangled_name_macro(*this), bind_mangled_name_macro(*this));

        w.write(R"^-^(typedef interface % %;

#endif // __%_FWD_DEFINED__

    )^-^",
            bind_c_type_name(*this),
            bind_c_type_name(*this),
            bind_mangled_name_macro(*this));
    }

    void delegate_type::write_c_abi_param(writer& w) const
    {
        w.write("%*", bind_c_type_name(*this));
    }

    static void write_delegate_definition(writer& w, delegate_type const& type, void (*func)(writer&, delegate_type const&))
    {
        // Generics don't get generated definitions
        if (type.is_generic())
        {
            return;
        }

         begin_type_definition(w, type);

        w.write(R"^-^(#if !defined(__%_INTERFACE_DEFINED__)
    #define __%_INTERFACE_DEFINED__
    )^-^", bind_mangled_name_macro(type), bind_mangled_name_macro(type));

        func(w, type);

        w.write(R"^-^(
    EXTERN_C const IID %;
    #endif /* !defined(__%_INTERFACE_DEFINED__) */
    )^-^", bind_iid_name(type), bind_mangled_name_macro(type));

        end_type_definition(w, type);
    }


    void delegate_type::write_c_definition(writer& w) const
    {
        write_delegate_definition(w, *this, &write_c_interface_definition<delegate_type>);
    }

    void interface_type::append_signature(sha1& hash) const
    {
        using namespace std::literals;
        hash.append("{"sv);
        auto iid = type_iid(m_type);
        hash.append(std::string_view{ iid.data(), iid.size() - 1 });
        hash.append("}"sv);
    }

    void interface_type::write_c_forward_declaration(writer& w) const
    {
        if (!w.should_forward_declare(m_mangledName))
        {
            return;
        }

        w.write(R"^-^(#ifndef __%_FWD_DEFINED__
#define __%_FWD_DEFINED__
    typedef interface % %;

#endif // __%_FWD_DEFINED__

)^-^",
            bind_mangled_name_macro(*this),
            bind_mangled_name_macro(*this),
            bind_c_type_name(*this),
            bind_c_type_name(*this),
            bind_mangled_name_macro(*this));
    }

    void interface_type::write_c_abi_param(writer& w) const
    {
        w.write("%*", bind_c_type_name(*this));
    }

    static void write_interface_definition(writer& w, interface_type const& type, void (*func)(writer&, interface_type const&))
    {
        // Generics don't get generated definitions
        if (type.is_generic())
        {
            return;
        }

         begin_type_definition(w, type);

        w.write(R"^-^(#if !defined(__%_INTERFACE_DEFINED__)
    #define __%_INTERFACE_DEFINED__
    extern const __declspec(selectany) _Null_terminated_ WCHAR InterfaceName_%_%[] = L"%";
    )^-^",
            bind_mangled_name_macro(type),
            bind_mangled_name_macro(type),
            bind_list("_", namespace_range{ type.swift_abi_namespace() }),
            type.cpp_abi_name(),
            type.swift_full_name());

        func(w, type);

        w.write(R"^-^(
    EXTERN_C const IID %;
#endif /* !defined(__%_INTERFACE_DEFINED__) */
    )^-^", bind_iid_name(type), bind_mangled_name_macro(type));

        end_type_definition(w, type);
    }

    void interface_type::write_c_definition(writer& w) const
    {
        write_interface_definition(w, *this, &write_c_interface_definition<interface_type>);
    }

    class_type::class_type(winmd::reader::TypeDef const& type) :
        typedef_base(type)
    {
        using namespace winmd::reader;
        if (auto defaultIface = get_default_interface(type))
        {
            auto [ns, _] = type_name::get_namespace_and_name(defaultIface);
            m_abiNamespace = ns;
        }
    }

    void class_type::write_c_forward_declaration(writer& w) const
    {
        if (!default_interface)
        {
            XLANG_ASSERT(false);
            swiftwinrt::throw_invalid("Cannot forward declare class '", m_swiftFullName, "' since it has no default interface");
        }

        default_interface->write_c_forward_declaration(w);
    }

    void class_type::write_c_abi_param(writer& w) const
    {
        if (!default_interface)
        {
            XLANG_ASSERT(false);
            swiftwinrt::throw_invalid("Class '", m_swiftFullName, "' cannot be used as a function argument since it has no "
                "default interface");
        }

        default_interface->write_c_abi_param(w);
    }

    void class_type::write_c_definition(writer&) const
    {
    }

    bool class_type::is_composable() const
    {
        return swiftwinrt::is_composable(type());
    }

    void generic_inst::append_signature(sha1& hash) const
    {
        using namespace std::literals;
        hash.append("pinterface({"sv);
        auto iid = type_iid(m_genericType->type());
        hash.append(std::string_view{ iid.data(), iid.size() - 1 });
        hash.append("}"sv);
        for (auto param : m_genericParams)
        {
            hash.append(";"sv);
            param->append_signature(hash);
        }
        hash.append(")"sv);
    }

    void generic_inst::write_c_forward_declaration(writer& w) const
    {
        if (!w.begin_declaration(m_mangledName))
        {
            if (w.should_forward_declare(m_mangledName))
            {
                w.write("typedef interface % %;\n\n", m_mangledName, m_mangledName);
            }

            return;
        }

        // Also need to make sure that all generic parameters are declared
        for (auto param : m_genericParams)
        {
            param->write_c_forward_declaration(w);
        }

        // First make sure that any generic requried interface/function argument/return types are declared
        for (auto dep : dependencies)
        {
            dep->write_c_forward_declaration(w);
        }

        auto isExperimental = is_experimental();
        if (isExperimental)
        {
            w.write("#if defined(ENABLE_WINRT_EXPERIMENTAL_TYPES)\n");
        }

        w.write(R"^-^(#if !defined(__%_INTERFACE_DEFINED__)
    #define __%_INTERFACE_DEFINED__

    typedef interface % %;

    //  Declare the parameterized interface IID.
    EXTERN_C const IID IID_%;

    )^-^", m_mangledName, m_mangledName, m_mangledName, m_mangledName, m_mangledName);

        write_c_interface_definition(w, *this);

        w.write(R"^-^(
    #endif // __%_INTERFACE_DEFINED__
    )^-^", m_mangledName);

        if (isExperimental)
        {
            w.write("#endif // defined(ENABLE_WINRT_EXPERIMENTAL_TYPES)\n");
        }

        w.write('\n');
        w.end_declaration(m_mangledName);
    }

    template<typename T>
    static void write_vtable_method(writer& w, function_def const& func, T const& type);

    void generic_inst::write_swift_declaration(writer& w) const
    {
        auto push_param_guard = w.push_generic_params(*this);
        w.write("internal static var %VTable: %Vtbl = .init(\n",
            mangled_name(),
            mangled_name());

        const bool is_delegate = generic_type()->category() == category::delegate_type;
        {
            auto indent = w.push_indent();
            write_iunknown_methods(w, *this);
            separator s{ w, ",\n\n" };

            if (!is_delegate)
            {
                write_iinspectable_methods(w, *this);
                s(); // get first separator out of the way for no-op
            }
        
            for (auto&& method : functions)
            {
                s();
                write_vtable_method(w, method, *this);
            }
        }

        w.write(R"(
)
)");
        if (is_delegate)
        {
            write_generic_delegate_wrapper(w, *this);
            return;
        }
        else if (is_winrt_generic_collection(*this))
        {
            w.write("typealias % = InterfaceWrapperBase<%>\n",
                bind_wrapper_name(*this),
                bind_impl_fullname(*this));
            return;
        }
        auto format = R"(internal class %: WinRTWrapperBase<%, %> {
    override class var IID: IID { IID_% }
    init?(_ value: %?) {
        guard let value = value else { return nil }
        let abi = withUnsafeMutablePointer(to: &%VTable) {
            %(lpVtbl:$0)
        }
        super.init(abi, %(value: value))
    }
}
)";
        w.write(format,
            bind_wrapper_name(*this),
            mangled_name(),
            remove_backtick(generic_type()->cpp_logical_name()),
            mangled_name(),
            get_full_swift_type_name(w, generic_params()[0]),
            mangled_name(),
            mangled_name(),
            bind_impl_name(*this)
            );
    }

    void generic_inst::write_c_abi_param(writer& w) const
    {
        w.write("%*", m_mangledName);
    }

    element_type const& element_type::from_type(winmd::reader::ElementType type)
    {
        static element_type const boolean_type{ ElementType::Boolean, "Bool"sv, "bool"sv, "boolean"sv, "boolean"sv, "boolean"sv, "b1"sv };
        static element_type const char_type{ ElementType::Char, "Character"sv, "wchar_t"sv, "wchar_t"sv, "WCHAR"sv, "wchar__zt"sv, "c2"sv };
        static element_type const u1_type{ ElementType::U1, "UInt8"sv, "::byte"sv, "::byte"sv, "BYTE"sv, "byte"sv, "u1"sv };
        static element_type const i2_type{ ElementType::I2, "Int16"sv, "short"sv, "short"sv, "INT16"sv, "short"sv, "i2"sv };
        static element_type const u2_type{ ElementType::U2, "UInt16"sv, "UINT16"sv, "UINT16"sv, "UINT16"sv, "UINT16"sv, "u2"sv };
        static element_type const i4_type{ ElementType::I4, "Int32"sv, "int"sv, "int"sv, "INT32"sv, "int"sv, "i4"sv };
        static element_type const u4_type{ ElementType::U4, "UInt32"sv, "UINT32"sv, "UINT32"sv, "UINT32"sv, "UINT32"sv, "u4"sv };
        static element_type const i8_type{ ElementType::I8, "Int64"sv, "__int64"sv, "__int64"sv, "INT64"sv, "__z__zint64"sv, "i8"sv };
        static element_type const u8_type{ ElementType::U8, "UInt64"sv, "UINT64"sv, "UINT64"sv, "UINT64"sv, "UINT64"sv, "u8"sv };
        static element_type const r4_type{ ElementType::R4, "Float"sv, "float"sv, "float"sv, "FLOAT"sv, "float"sv, "f4"sv };
        static element_type const r8_type{ ElementType::R8, "Double"sv, "double"sv, "double"sv, "DOUBLE"sv, "double"sv, "f8"sv };
        static element_type const string_type{ ElementType::String, "String"sv, "HSTRING"sv, "HSTRING"sv, "HSTRING"sv, "HSTRING"sv, "string"sv };
        static element_type const object_type{ ElementType::Object, "IInspectable"sv, "IInspectable"sv, "IInspectable"sv, "IInspectable*"sv, "IInspectable"sv, "cinterface(IInspectable)"sv };

        switch (type)
        {
        case ElementType::Boolean: return boolean_type;
        case ElementType::Char: return char_type;
        case ElementType::U1: return u1_type;
        case ElementType::I2: return i2_type;
        case ElementType::U2: return u2_type;
        case ElementType::I4: return i4_type;
        case ElementType::U4: return u4_type;
        case ElementType::I8: return i8_type;
        case ElementType::U8: return u8_type;
        case ElementType::R4: return r4_type;
        case ElementType::R8: return r8_type;
        case ElementType::String: return string_type;
        case ElementType::Object: return object_type;
        default: swiftwinrt::throw_invalid("Unrecognized ElementType: ", std::to_string(static_cast<int>(type)));
        }
    }

    bool element_type::is_blittable() const
    {
        switch (m_type)
        {
        case ElementType::Boolean:
        case ElementType::Char:
        case ElementType::String:
        case ElementType::Object:
            return false;
        default:
            return true;
        }
    }
    void element_type::write_c_abi_param(writer& w) const
    {
        w.write(m_cppName);
    }

    system_type const& system_type::from_name(std::string_view typeName)
    {
        if (typeName == "Guid"sv)
        {
            static system_type const guid_type{ "UUID"sv, "GUID"sv, "g16"sv };
            return guid_type;
        }

        XLANG_ASSERT(false);
        swiftwinrt::throw_invalid("Unknown type '", typeName, "' in System namespace");
    }

    void system_type::write_c_abi_param(writer& w) const
    {
        w.write(m_cppName);
    }

    mapped_type const* mapped_type::from_typedef(winmd::reader::TypeDef const& type)
    {
        if (type.TypeNamespace() == winrt_foundation_namespace)
        {
            if (type.TypeName() == "HResult"sv)
            {
                static mapped_type const hresult_type{ type, "HRESULT"sv, "HRESULT"sv, "struct(Windows.Foundation.HResult;i4)"sv };
                return &hresult_type;
            }
            else if (type.TypeName() == "EventRegistrationToken"sv)
            {
                static mapped_type event_token_type{ type, "EventRegistrationToken"sv, "EventRegistrationToken"sv, "struct(Windows.Foundation.EventRegistrationToken;i8)"sv };
                return &event_token_type;
            }
            else if (type.TypeName() == "AsyncStatus"sv)
            {
                static mapped_type const async_status_type{ type, "AsyncStatus"sv, "AsyncStatus"sv, "enum(Windows.Foundation.AsyncStatus;i4)"sv };
                return &async_status_type;
            }
            else if (type.TypeName() == "IAsyncInfo"sv)
            {
                static mapped_type const async_info_type{ type, "IAsyncInfo"sv, "IAsyncInfo"sv, "{00000036-0000-0000-c000-000000000046}"sv };
                return &async_info_type;
            }
        }

        return nullptr;
    }

    void mapped_type::write_c_abi_param(writer& w) const
    {
        w.write(m_cppName);

        auto typeCategory = get_category(m_type);
        if ((typeCategory == category::delegate_type) ||
            (typeCategory == category::interface_type) ||
            (typeCategory == category::class_type))
        {
            w.write('*');
        }
    }
}

