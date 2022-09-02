#pragma once

#include "pch.h"

#include <cctype>
#include <cstring>

#include "settings.h"
#include "type_writers.h"
#include "metadata_cache.h"
#include "settings.h"
#include "common.h"

using namespace std::literals;
using namespace winmd::reader;

namespace swiftwinrt
{
    inline std::string_view mangled_name_macro_format(writer& w)
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

    inline void write_uuid(writer& w, winmd::reader::TypeDef const& type)
    {
        auto iidStr = type_iid(type);
        w.write(std::string_view{ iidStr.data(), iidStr.size() - 1 });
    }

    inline void write_uuid(writer& w, typedef_base const& type)
    {
        write_uuid(w, type.type());
    }

    inline void write_generated_uuid(writer& w, metadata_type const& type)
    {
        sha1 signatureHash;
        static constexpr std::uint8_t namespaceGuidBytes[] =
        {
            0x11, 0xf4, 0x7a, 0xd5,
            0x7b, 0x73,
            0x42, 0xc0,
            0xab, 0xae, 0x87, 0x8b, 0x1e, 0x16, 0xad, 0xee
        };
        signatureHash.append(namespaceGuidBytes, std::size(namespaceGuidBytes));
        type.append_signature(signatureHash);

        auto iidHash = signatureHash.finalize();
        iidHash[6] = (iidHash[6] & 0x0F) | 0x50;
        iidHash[8] = (iidHash[8] & 0x3F) | 0x80;
        w.write_printf("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            iidHash[0], iidHash[1], iidHash[2], iidHash[3],
            iidHash[4], iidHash[5],
            iidHash[6], iidHash[7],
            iidHash[8], iidHash[9],
            iidHash[10], iidHash[11], iidHash[12], iidHash[13], iidHash[14], iidHash[15]);
    }

    inline void write_uuid(writer& w, generic_inst const& type)
    {
        write_generated_uuid(w, type);
    }

    template <typename T>
    auto bind_uuid(T const& type)
    {
        return [&](writer& w)
        {
            write_uuid(w, type);
        };
    }


    static void write_includes(writer& w, type_cache const& types, std::string_view fileName)
    {
        // Forced dependencies
        w.write(R"^-^(// Header files for imported files
#include "inspectable.h"
#include "AsyncInfo.h"
#include "EventToken.h"
#include "windowscontracts.h"
)^-^");

        if (fileName != foundation_namespace)
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
            if (ns == collections_namespace)
            {
                // The collections header is hand-rolled
                hasCollectionsDependency = true;
            }
            else if (ns == foundation_namespace)
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
            else if (ns.starts_with("Windows"))
            {
                // Don't include anything from windows
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
            if (!type.get().is_generic())
            {
                type.get().write_c_forward_declaration(w);
            }
        }

        for (auto const& type : types.interfaces)
        {
            if (!type.get().is_generic())
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
            inst.get().write_c_forward_declaration(w);
        }
    }

    static void write_c_dependency_forward_declarations(writer& w, type_cache const& types)
    {
        for (auto const& type : types.external_dependencies)
        {
            type.get().write_c_forward_declaration(w);
        }

        for (auto const& type : types.internal_dependencies)
        {
            type.get().write_c_forward_declaration(w);
        }
    }

    static void write_c_type_definitions(writer& w, type_cache const& types)
    {
        for (auto const& enumType : types.enums)
        {
            enumType.get().write_c_definition(w);
        }

        for (auto const& structType : types.structs)
        {
            structType.get().write_c_definition(w);
        }

        for (auto const& delegateType : types.delegates)
        {
            delegateType.get().write_c_definition(w);
        }

        for (auto const& interfaceType : types.interfaces)
        {
            interfaceType.get().write_c_definition(w);
        }

        for (auto const& classType : types.classes)
        {
            classType.get().write_c_definition(w);
        }
    }

    inline void write_abi_header(std::string_view fileName, settings_type const& config, type_cache const& types)
    {
        writer w;
        w.type_namespace = fileName;
        write_preamble(w);

        w.write("#pragma once\n");
        write_includes(w, types, fileName);

        write_c_interface_forward_declarations(w, types);
        write_c_generic_definitions(w, types);
        write_c_dependency_forward_declarations(w, types);
        write_c_type_definitions(w, types);

        w.save_header();
    }

    inline void write_include_all(std::map<std::string, std::vector<std::string_view>>& namespaces, settings_type const& config)
    {
        for (auto& [module, ns_list] : namespaces)
        {
            writer w;
            write_preamble(w);
            w.write("#pragma once\n");
            w.write("#pragma clang diagnostic push\n");
            w.write("#pragma clang diagnostic ignored \"-Wmicrosoft-enum-forward-reference\"\n");
            // sort so the file stays consistent
            std::sort(ns_list.begin(), ns_list.end());
            for (auto& ns : ns_list)
            {
                w.write("#include \"%.h\"\n", ns);
            }
            w.write("#pragma clang diagnostic pop\n");

            auto filename{ settings.output_folder + module + "/c/" };
            filename += "C" + module + ".h";

            w.flush_to_file(filename);
        }
           

    }
}