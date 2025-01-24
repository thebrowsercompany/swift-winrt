#include "pch.h"

// we get warnings about unreferenced methods being removed even though they are referenced
// and actually haven't been removed
#pragma warning(push)
#pragma warning (disable: 4505)
#include <ctime>
#include "settings.h"
#include "type_helpers.h"
#include "types.h"
#include "type_writers.h"
#include "helpers.h"
#include "versioning.h"
#include "namespace_iterator.h"
#include "common.h"
#include "code_writers.h"
#include "component_writers.h"
#include "abi_writer.h"
#include "metadata_cache.h"
#include "metadata_filter.h"
#include "file_writers.h"
#include "project_file_writers.h"
#pragma warning(pop)


namespace swiftwinrt
{
    settings_type settings;

    struct usage_exception {};

    static constexpr option options[]
    {
        { "input", 0, option::no_max, "<spec>", "Windows metadata to include in projection" },
        { "reference", 0, option::no_max, "<spec>", "Windows metadata to reference from projection" },
        { "output", 0, 1, "<path>", "Location of generated projection and component templates" },
        { "component", 0, 1, "[<path>]", "Generate component templates, and optional implementation" },
        { "name", 0, 1, "<name>", "Specify explicit name for component files" },
        { "verbose", 0, 0, {}, "Show detailed progress information" },
        { "log", 0, 0, {}, "Write detailed information to log" },
        { "ns-prefix", 0, 1, "<always|optional|never>", "Sets policy for prefixing type names with 'ABI' namespace (default: never)" },
        { "overwrite", 0, 0, {}, "Overwrite generated component files" },
        { "support", 0, 1, "<module>", "module to include support files" },
        { "include", 0, option::no_max, "<prefix>", "One or more prefixes to include in input" },
        { "exclude", 0, option::no_max, "<prefix>", "One or more prefixes to exclude from input" },
        { "help", 0, option::no_max, {}, "Show detailed help with examples" },
        { "spm", 0, 0, "generate SPM project files"}, // generate SPM project files
        { "cmake", 0, 0, "generate CMake project files"}, // generate CMake project files
        { "?", 0, option::no_max, {}, {} },
        { "library", 0, 1, "<prefix>", "Specify library prefix (defaults to winrt)" },
        { "test", 0, 0 }, // the projections are for tests and place all code into a single module
        { "filter" }, // One or more prefixes to include in input (same as -include)
        { "license", 0, 0 }, // Generate license comment
        { "brackets", 0, 0 }, // Use angle brackets for #includes (defaults to quotes)
        { "fastabi", 0, 0 }, // Enable support for the Fast ABI
        { "ignore_velocity", 0, 0 }, // Ignore feature staging metadata and always include implementations
        { "synchronous", 0, 0 }, // Instructs cppwinrt to run on a single thread to avoid file system issues in batch builds
    };

    static void print_usage(writer& w)
    {
        static auto printColumns = [](writer& w, std::string_view const& col1, std::string_view const& col2)
        {
            w.write_printf("  %-20s%s\n", col1.data(), col2.data());
        };

        static auto printOption = [](writer& w, option const& opt)
        {
            if(opt.desc.empty())
            {
                return;
            }
            printColumns(w, w.write_temp("-% %", opt.name, opt.arg), opt.desc);
        };

        auto format = R"(
Swift/WinRT v%
Copyright (c) The Browser Company. All rights reserved.

  swiftwinrt.exe [options...]

Options:

%  ^@<path>             Response file containing command line options

Where <spec> is one or more of:

  path                Path to winmd file or recursively scanned folder
  local               Local ^%WinDir^%\System32\WinMetadata folder
  sdk[+]              Current version of Windows SDK [with extensions]
  10.0.12345.0[+]     Specific version of Windows SDK [with extensions]
)";
        w.write(format, SWIFTWINRT_VERSION_STRING, bind_each(printOption, options));
    }

    static void process_args(reader const& args)
    {
        settings.log = args.exists("log");
        settings.verbose = settings.log || args.exists("verbose");
        settings.fastabi = args.exists("fastabi");

        settings.input = args.files("input", database::is_database);
        settings.reference = args.files("reference", database::is_database);

        settings.component = args.exists("component");

        settings.license = args.exists("license");
        settings.brackets = args.exists("brackets");
        settings.test = args.exists("test");
        settings.output_folder = args.value("output", ".");

        settings.support = args.value("support", "WindowsFoundation");

        create_directories(settings.output_folder);
        create_directories(writer::root_directory());
        create_directories(writer::root_directory() / "CWinRT");
        create_directories(writer::root_directory() / "CWinRT" / "include");

        for (auto && include : args.values("include"))
        {
            settings.include.insert(include);
        }

        for (auto && include : args.values("filter"))
        {
            settings.include.insert(include);
        }

        for (auto && exclude : args.values("exclude"))
        {
            settings.exclude.insert(exclude);
        }

        if (args.exists("spm"))
        {
            settings.project |= project_type::spm;
        }
        if (args.exists("cmake"))
        {
            settings.project |= project_type::cmake;
        }
        if (settings.component)
        {
            settings.component_overwrite = args.exists("overwrite");
            settings.component_name = args.value("name");

            if (settings.component_name.empty())
            {
                // For compatibility with C++/WinRT 1.0, the component_name defaults to the *first*
                // input, hence the use of values() here that will return the args in input order.

                auto& values = args.values("input");

                if (!values.empty())
                {
                    settings.component_name = path(values[0]).filename().replace_extension().string();
                }
            }

            settings.component_prefix = args.exists("prefix");
            settings.component_lib = args.value("library", "winrt");
            settings.component_opt = args.exists("optimize");
            settings.component_ignore_velocity = args.exists("ignore_velocity");

            settings.component_folder = args.value("component");

            if (!settings.component_folder.empty())
            {
                create_directories(settings.component_folder);
            }
        }
    }

    static auto get_files_to_cache()
    {
        std::vector<std::string> files;
        files.insert(files.end(), settings.input.begin(), settings.input.end());
        files.insert(files.end(), settings.reference.begin(), settings.reference.end());
        return files;
    }

    static void build_filters(cache const& c)
    {
        std::set<std::string> include;

        for (auto file : settings.input)
        {
            auto db = std::find_if(c.databases().begin(), c.databases().end(), [&](auto&& db)
            {
                return db.path() == file;
            });

            for (auto&& type : db->TypeDef)
            {
                if (!type.Flags().WindowsRuntime())
                {
                    continue;
                }

                std::string full_name{ type.TypeNamespace() };
                full_name += '.';
                full_name += type.TypeName();

                include.insert(full_name);
            }
        }


        settings.projection_filter = { settings.include.empty() ? include : settings.include, settings.exclude };

        settings.component_filter = { settings.include.empty() ? include : settings.include, settings.exclude };
    }

    static void build_fastabi_cache(cache const& c)
    {
        if (!settings.fastabi)
        {
            return;
        }

        for (auto&& [ns, members] : c.namespaces())
        {
            for (auto&& type : members.classes)
            {
                if (!has_fastabi(type))
                {
                    continue;
                }

                auto default_interface = get_default_interface(type);

                if (default_interface.type() == TypeDefOrRef::TypeDef)
                {
                    settings.fastabi_cache.try_emplace(default_interface.TypeDef(), type);
                }
                else
                {
                    settings.fastabi_cache.try_emplace(find_required(default_interface.TypeRef()), type);
                }
            }
        }
    }

    static int run(int const argc, char** argv)
    {
        int result{};
        writer w;
        std::filesystem::path log_file;
        try
        {
            auto start = get_start_time();

            reader args{ argc, argv, options };

            if (!args || args.exists("help") || args.exists("?"))
            {
                throw usage_exception{};
            }

            process_args(args);
            log_file = settings.output_folder / "swiftwinrt.log";

            cache c{ get_files_to_cache(), [](TypeDef const& type) {
                if (!type.Flags().WindowsRuntime())
                {
                    return false;
                }
                return true;
            }};
            metadata_cache mdCache{ c };

            auto include = args.values("include");
            include_only_used_filter mf{ mdCache, include };

            if (settings.verbose)
            {
                char* path = nullptr;
                _get_pgmptr(&path);
                w.write(" tool:  %\n", path);
                w.write(" ver:   %\n", SWIFTWINRT_VERSION_STRING);

                for (auto&& file : settings.input)
                {
                    w.write(" in:    %\n", file);
                }

                for (auto&& file : settings.reference)
                {
                    w.write(" ref:   %\n", file);
                }

                w.write(" out:   %\n", settings.output_folder.string());

                if (!settings.component_folder.empty())
                {
                    w.write(" cout:  %\n", settings.component_folder.string());
                }
            }

            if (settings.log)
            {
                w.flush_to_file(log_file);
            }
            else
            {
                w.flush_to_console();
            }

            task_group group;
            group.synchronous(args.exists("synchronous"));

            std::map<std::string, std::vector<std::string_view>> module_map; // map of module -> namespaces
            std::map<std::string, std::set<std::string>> module_dependencies; // module -> module dependencies
            path output_folder = settings.output_folder;
            for (auto&&[ns, members] : c.namespaces())
            {
                if (!has_projected_types(members))
                {
                    continue;
                }

                group.add([&, &ns = ns]
                {
                    // we want the C module to contain all of the types so that incremental builds of the
                    // projections is quick. we don't actually even need the end result of the C bindings
                    // and so it can be discarded after the app is built - meaning the size increase doesn't
                    // matter
                    include_all_filter filter{c};
                    auto types = mdCache.compile_namespaces({ ns }, filter);
                    write_abi_header(ns, types);
                });

                if (!mf.includes_any(members))
                {
                    continue;
                }
                auto module_name = get_swift_module(ns);

                auto [moduleMapItr, moduleAdded] = module_map.emplace(std::piecewise_construct,
                    std::forward_as_tuple(module_name),
                    std::forward_as_tuple());
                if (moduleAdded)
                {
                    create_directories(writer::root_directory() / module_name);
                }
                moduleMapItr->second.push_back(ns);
            }
            for (auto&& [module, namespaces] : module_map)
            {
                auto [moduleItr, added] = module_dependencies.emplace(std::piecewise_construct,
                    std::forward_as_tuple(module),
                    std::forward_as_tuple());
                assert(added);
                group.add([&,
                        &module = module,
                        &namespaces = namespaces,
                        &moduleDependencies = moduleItr->second]
                    {
                        swiftwinrt::task_group module_group;
                        module_group.add([&, &namespaces = namespaces]
                        {
                            // generics are written on a per module basis because this helps us reduce the
                            // amount of code that is generated.
                            auto types = mdCache.compile_namespaces(namespaces, mf);
                            write_module_generics(module, types, mf);
                        });

                        if (module == settings.support)
                        {
                            module_group.add([&]
                            {
                                write_swift_support_files(module);
                            });
                        }

                        for (auto& ns : namespaces)
                        {
                            module_group.add([&, &ns = ns]

                            {
                                auto types = mdCache.compile_namespaces({ ns }, mf);
                                write_namespace_abi (ns, types, mf);
                                write_namespace_wrapper(ns, types, mf);
                                write_namespace_impl(ns, types, mf);
                             });
                        }

                        module_group.get();

                        // don't write the cmake file if only building a single module
                        if (!settings.test)
                        {
                            if (module != settings.support)
                            {
                                moduleDependencies.emplace(settings.support);
                            }
                            auto dependentNamespaces = mdCache.get_dependent_namespaces(namespaces, mf);

                            for (auto&& dependent_ns : dependentNamespaces)
                            {
                                auto dependent_module = get_swift_module(dependent_ns);
                                if (dependent_module != module)
                                {
                                    moduleDependencies.emplace(dependent_module);
                                }
                            }

                            write_cmake_lists(module, moduleDependencies, namespaces);
                            write_singlemodule_package_swift(module, moduleDependencies);
                        }
                    });
            }

            group.add([] { write_cwinrt_build_files(); });

            group.get();

            if (settings.component)
            {
                throw std::exception("component generation not yet supported");
            }

            if (!settings.test)
            {
                write_root_cmake(module_map);
            }
            else
            {
                // don't write the root cmake for the test project, instead just write the
                // cmake file for the single module everything is built into, which doesn't
                // have any dependencies
                write_cmake_lists(settings.support, {}, module_map[settings.support]);
                write_multimodule_package_swift(module_dependencies);
            }

            write_include_all(c.namespaces());
            write_modulemap();
            if (settings.verbose)
            {
                w.write(" time:  %ms\n", get_elapsed_time(start));
            }
        }
        catch (usage_exception const&)
        {
            print_usage(w);
        }
        catch (std::exception const& e)
        {
            w.write("swiftwinrt : error %\n", e.what());
            if (settings.log && !log_file.empty())
            {
                // We're logging the error to a log file,
                // but also print it to simplify diagnosing build errors.
                fprintf(stderr, "error: %s", e.what());
            }
            result = 1;
        }

        if (settings.log && !log_file.empty())
        {
            w.flush_to_file(log_file, true);
        }
        else
        {
            w.flush_to_console(result == 0);
        }
        return result;
    }
}

int main(int const argc, char** argv)
{
    return swiftwinrt::run(argc, argv);
}
