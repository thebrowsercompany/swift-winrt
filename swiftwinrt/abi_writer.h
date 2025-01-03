#pragma once

#include "pch.h"

#include <cctype>
#include <cstring>

#include "settings.h"
#include "type_writers.h"
#include "metadata_cache.h"
#include "settings.h"
#include "common.h"
#include "helpers.h"

using namespace std::literals;
using namespace winmd::reader;

namespace swiftwinrt
{
    inline std::string_view mangled_name_macro_format(writer&)
    {
        return "%";
    }

    inline std::string_view c_typename_format(writer& w)
    {
        return mangled_name_macro_format(w);
    }

    inline void write_mangled_name_macro(writer& w, metadata_type const& type)
    {
        w.write(mangled_name_macro_format(w), type.mangled_name());
    }

    inline void write_mangled_name_macro(writer& w, generic_inst const& type)
    {
        w.write(type.mangled_name());
    }

    template <typename T>
    inline auto bind_mangled_name_macro(T const& type)
    {
        return [&](writer& w)
        {
            write_mangled_name_macro(w, type);
        };
    }

    template <typename T>
    inline void write_iid_name(writer& w, T const& type)
    {
        w.write("IID_%", bind_mangled_name_macro(type));
    }

    template <typename T>
    inline auto bind_iid_name(T const& type)
    {
        return [&](writer& w)
        {
            write_iid_name(w, type);
        };
    }

    template <typename T>
    inline void write_c_type_name(writer& w, T const& type)
    {
        write_c_type_name(w, type, "");
    }

    template <typename Suffix>
    inline void write_c_type_name(writer& w, typedef_base const& type, Suffix&& suffix)
    {
        w.write(c_typename_format(w), [&](writer& w) { w.write("%%", type.mangled_name(), suffix); });
    }

    template <typename Suffix>
    inline void write_c_type_name(writer& w, generic_inst const& type, Suffix&& suffix)
    {
        w.write("%%", type.mangled_name(), suffix);
    }

    template <typename T>
    auto bind_c_type_name(T const& type)
    {
        return [&](writer& w)
        {
            write_c_type_name(w, type);
        };
    }

    template <typename T, typename Suffix>
    auto bind_c_type_name(T const& type, Suffix&& suffix)
    {
        return [&](writer& w)
        {
            write_c_type_name(w, type, suffix);
        };
    }

    // Don't write "built-in types" since these are defined in other header files
    // These aren't completely removed from metadata because we need to generate them
    // on the swift side
    static std::set<std::string_view> removed_types = {
        "Windows.Foundation.Collections.CollectionChange",
        "Windows.Foundation.Collections.IVectorChangedEventArgs",
    };
    static bool should_write(metadata_type const& type)
    {
        return !removed_types.contains(type.swift_full_name());
    }

    static void write_includes(writer& w, type_cache const& types, std::string_view fileName)
    {
        // Forced dependencies
        w.write(R"^-^(// Header files for imported files
#include <inspectable.h>
#include <EventToken.h>
#include <windowscontracts.h>
)^-^");

        if (fileName != winrt_foundation_namespace)
        {
            w.write(R"^-^(#include "Windows.Foundation.h"
)^-^");
        }
        else
        {
            w.write(R"^-^(#include "IVectorChangedEventArgs.h"
)^-^");
        }

        bool hasCollectionsDependency = false;
        for (auto ns : types.dependent_namespaces)
        {
            if (ns == winrt_collections_namespace)
            {
                // The collections header is hand-rolled
                hasCollectionsDependency = true;
            }
            else if (ns == winrt_foundation_namespace)
            {
                // This is a forced dependency
            }
            else if (ns == system_namespace)
            {
                // The "System" namespace a lie
            }
            else if (ns == fileName)
            {
                // Don't include ourself
            }
            else
            {
                w.write(R"^-^(#include "%.h"
)^-^", ns);
            }
        }

        if (hasCollectionsDependency)
        {
            w.write(R"^-^(// Importing Collections header
#include "Windows.Foundation.Collections.h"
)^-^");
        }

        w.write("\n");
    }

    static void write_c_interface_forward_declarations(writer& w, type_cache const& types)
    {
        w.write("/* Forward Declarations */\n");

        for (auto const& type : types.delegates)
        {
            if (!type.get().is_generic() && should_write(type.get()))
            {
                type.get().write_c_forward_declaration(w);
            }
        }

        for (auto const& type : types.interfaces)
        {
            if (!type.get().is_generic() && should_write(type.get()))
            {
                type.get().write_c_forward_declaration(w);
            }
        }
    }

    static void write_c_generic_definitions(writer& w, type_cache const& types)
    {
        w.write(R"^-^(// Parameterized interface forward declarations (C)

// Collection interface definitions

)^-^");

        for (auto const& [name, inst] : types.generic_instantiations)
        {
            if (should_write(inst.get()))
            {
                inst.get().write_c_forward_declaration(w);

            }
        }
    }

    static void write_c_dependency_forward_declarations(writer& w, type_cache const& types)
    {
        for (auto const& type : types.external_dependencies)
        {
            if (should_write(type.get()))
            {
                type.get().write_c_forward_declaration(w);
            }

        }

        for (auto const& type : types.internal_dependencies)
        {
            if (should_write(type.get()))
            {
                type.get().write_c_forward_declaration(w);
            }
        }
    }

    static void write_c_type_definitions(writer& w, type_cache const& types)
    {
        for (auto const& enumType : types.enums)
        {
            if (should_write(enumType.get()))
            {
                enumType.get().write_c_definition(w);
            }
        }

        for (auto const& structType : types.structs)
        {
            if (should_write(structType.get()))
            {
                structType.get().write_c_definition(w);
            }
        }

        for (auto const& delegateType : types.delegates)
        {
            if (should_write(delegateType.get()))
            {
                delegateType.get().write_c_definition(w);
            }
        }

        for (auto const& interfaceType : types.interfaces)
        {
            if (should_write(interfaceType.get()))
            {
                interfaceType.get().write_c_definition(w);
            }
        }

        for (auto const& classType : types.classes)
        {
            if (should_write(classType.get()))
            {
                classType.get().write_c_definition(w);
            }
        }
    }

    inline void write_abi_header(std::string_view fileName, type_cache const& types)
    {
        writer w;
        w.type_namespace = fileName;
        write_preamble(w, /* swift_code: */ false);

        w.write("#pragma once\n");
        write_includes(w, types, fileName);

        write_c_interface_forward_declarations(w, types);
        write_c_generic_definitions(w, types);
        write_c_dependency_forward_declarations(w, types);
        write_c_type_definitions(w, types);

        w.save_header();
    }

    inline void write_include_all(std::map<std::string_view, winmd::reader::cache::namespace_members> const& namespaces)
    {
        writer w;
        w.c_mod = settings.get_c_module_name();
        write_preamble(w, /* swift_code: */ false);
        w.write(R"(#pragma once
#include <wtypesbase.h>
#include <minwindef.h>
#include <winnt.h>
#include <combaseapi.h> // IUnknown, CoCreateInstance
#include <inspectable.h> // IInspectable
#include <oleauto.h> // BSTR, Sys***String***
#include <roapi.h> // Ro***
#include "RestrictedErrorInfo.h" // IRestrictedErrorInfo (C definition)
#include <roerrorapi.h> // GetRestrictedErrorInfo
#include <winstring.h> // HSTRING, Windows***String***

// undefine win32 apis which collide with WinRT method names
#undef GetCurrentTime
#undef FindText
#undef GetClassName
#undef GetObject
#undef GetGlyphIndices

#define ENABLE_WINRT_EXPERIMENTAL_TYPES

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmicrosoft-enum-forward-reference"

#include "CppInteropWorkaround.h" // TODO(WIN-860): Remove workaround once C++ interop issues with WinSDK.GUID are fixed.
#include "MemoryBuffer.h" // IMemoryBufferByteAccess (C definition)
#include "WeakReference.h" // IWeakReference[Source] (C definition)
#include "robuffer.h" // IBufferByteAccess (C definition)
)");
        for (auto& [ns, members] : namespaces)
        {
            if (has_projected_types(members))
            {
                w.write("#include \"%.h\"\n", ns);

            }
        }

        w.write("#pragma clang diagnostic pop\n");

        w.type_namespace = w.c_mod;
        w.save_header();

    }

    inline void write_modulemap()
    {
        writer w;
        write_preamble(w, /* swift_code: */ false);
        w.write(R"^_^(module % {
    header "%.h"
    export *
}
)^_^", settings.get_c_module_name(), settings.get_c_module_name());
        w.save_modulemap();
    }
}
