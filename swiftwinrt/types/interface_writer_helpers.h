#pragma once

#include <type_traits>
#include <vector>

#include "abi_writer.h"
#include "attributes.h"
#include "utility/type_helpers.h"
#include "types/class_type.h"
#include "types/delegate_type.h"
#include "types/function_def.h"
#include "types/generic_inst.h"
#include "types/interface_type.h"
#include "types/type_definition_helpers.h"
#include "winmd_reader.h"

namespace swiftwinrt
{
    inline std::string_view function_name(winmd::reader::MethodDef const& def)
    {
        using namespace std::literals;

        auto fn_name = def.Name();
        if (auto overload_attr = get_attribute(def, metadata_namespace, "OverloadAttribute"sv))
        {
            auto sig = overload_attr.Value();
            auto const& fixed_args = sig.FixedArgs();
            XLANG_ASSERT(fixed_args.size() == 1);
            fn_name = std::get<std::string_view>(std::get<ElemSig>(fixed_args[0].value).value);
        }

        return fn_name;
    }

    template <typename T>
    inline void write_c_iunknown_interface(writer& w, T const& type)
    {
        (void)type;
        w.write(R"^-^(    HRESULT (STDMETHODCALLTYPE* QueryInterface)(%* This,
            REFIID riid,
            void** ppvObject);
        ULONG (STDMETHODCALLTYPE* AddRef)(%* This);
        ULONG (STDMETHODCALLTYPE* Release)(%* This);
    )^-^", bind_c_type_name(type), bind_c_type_name(type), bind_c_type_name(type));
    }

    template <typename T>
    inline void write_c_iinspectable_interface(writer& w, T const& type)
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
    inline void write_c_function_declaration(writer& w, TypeName&& type_name, function_def const& func)
    {
        using namespace std::literals;

        w.write("    HRESULT (STDMETHODCALLTYPE* %)(%* This", function_name(func.def), type_name);

        for (auto const& param : func.params)
        {
            auto ref_mod = param.signature.ByRef() ? "*"sv : ""sv;
            if (param.signature.Type().is_szarray())
            {
                w.write(",\n        UINT32% %Length", ref_mod, param.name);
                ref_mod = param.signature.ByRef() ? "**"sv : "*"sv;
            }

            auto const_mod = is_const(param.signature) ? "const "sv : ""sv;
            w.write(",\n        %%% %",
                const_mod,
                [&](writer& inner) { param.type->write_c_abi_param(inner); },
                ref_mod,
                param.name);
        }

        if (func.return_type)
        {
            auto ref_mod = "*"sv;
            if (func.return_type->signature.Type().is_szarray())
            {
                w.write(",\n        UINT32* %Length", func.return_type->name);
                ref_mod = "**"sv;
            }

            w.write(",\n        %% %",
                [&](writer& inner) { func.return_type->type->write_c_abi_param(inner); },
                ref_mod,
                func.return_type->name);
        }

        w.write(");\n");
    }

    template <typename T>
    inline void write_c_interface_definition(writer& w, T const& type)
    {
        constexpr bool is_interface = std::is_same_v<T, interface_type>;
        constexpr bool is_delegate = std::is_same_v<T, delegate_type>;
        constexpr bool is_generic = std::is_same_v<T, generic_inst>;
        static_assert(is_interface || is_delegate || is_generic);

        w.write(R"^-^(typedef struct %
    {
        BEGIN_INTERFACE

    )^-^", bind_c_type_name(type, "Vtbl"));

        const bool is_delegate_category = type.category() == category::delegate_type;
        if (is_delegate_category)
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
                using namespace std::literals;

                w.write("\n// Supplemental functions added by use of the fast ABI attribute\n");

                auto fast_attr = get_attribute(type.fast_class->type(), metadata_namespace, "FastAbiAttribute"sv);
                (void)fast_attr;

                std::vector<class_type const*> base_classes;
                auto base = type.fast_class->base_class;
                while (base)
                {
                    base_classes.push_back(base);
                    base = base->base_class;
                }

                std::for_each(base_classes.rbegin(), base_classes.rend(), [&](class_type const* base_class)
                {
                    w.write("    % (STDMETHODCALLTYPE* base_%)(%* This);\n",
                        [&](writer& inner) { base_class->write_c_abi_param(inner); },
                        base_class->cpp_logical_name(),
                        bind_c_type_name(type));
                });

                for (auto [iface, ver] : type.fast_class->supplemental_fast_interfaces)
                {
                    (void)ver;
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
}
