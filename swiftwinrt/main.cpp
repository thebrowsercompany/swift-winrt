#include "pch.h"

// we get warnings about unreferenced methods being removed even though they are referenced
// and actually haven't been removed
#pragma warning(push)
#pragma warning (disable: 4505)
#include <ctime>
#include "utility/metadata_cache.h"
#include "utility/metadata_filter.h"
#include "utility/metadata_helpers.h"
#include "utility/type_helpers.h"
#include "utility/settings.h"
#include "utility/swift_codegen_utils.h"
#include "utility/versioning.h"
#include "types.h"
#include "utility/type_writers.h"
#include "code_writers.h"
#include "file_writers/abi_writer.h"
#include "file_writers/file_writers.h"
#include "tasks/write_project_files.h"
#include "tasks/write_code_files.h"
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
        { "support", 0, 1, "<module>", "module to include support files" },
        { "include", 0, option::no_max, "<prefix>", "One or more prefixes to include in input" },
        { "exclude", 0, option::no_max, "<prefix>", "One or more prefixes to exclude from input" },
        { "help", 0, option::no_max, {}, "Show detailed help with examples" },
        { "spm", 0, 0, "generate SPM project files"}, // generate SPM project files
        { "task", 0, 1, "<GenerateProjectFiles|GenerateBindings>", "which task to run from the execuable (default: GenerateBindings)"},
        { "cmake", 0, 0, "generate CMake project files"}, // generate CMake project files
        { "swift-version", 0, 1, "<version>", "Specify Swift tools version for SPM project files"},
        { "file-per-type", 0, 1, "generate files per type"},
        { "?", 0, option::no_max, {}, {} },
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

        settings.license = args.exists("license");
        settings.brackets = args.exists("brackets");
        settings.output_folder = args.value("output", ".");
        settings.file_per_category = args.exists("file-per-category");

        settings.support = args.value("support", "WindowsFoundation");
        if (args.exists("spm"))
        {
            settings.project = settings.project | project_type::spm;
            if (!args.exists("swift-version"))
            {
                throw std::invalid_argument("SPM project generation requires -swift-version to be specified");
            }
            settings.swift_version = args.value("swift-version");
        }
        if (args.exists("cmake"))
        {
            settings.project = settings.project | project_type::cmake;
        }

        auto task_value = args.value("task", "GenerateBindings");
        if (task_value == "GenerateBindings")
        {
            settings.task = task::code_gen;
        }
        else if (task_value == "GenerateProjectFiles")
        {
            settings.task = task::proj_gen;
        }
        else
        {
            throw std::invalid_argument("Invalid task specified: " + task_value);
        }
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

            const auto module_map = get_swift_modules(c, mf);
            for (auto&& [module, namespaces] : module_map)
            {
                if (settings.task == task::code_gen)
                {
                    write_swift_code_files(group, module, namespaces, mdCache, mf);
                    write_c_code_files(group, module, namespaces, mdCache, include_all_filter{ c });
                }
                else if (settings.task == task::proj_gen)
                {
                    if (settings.has_project_type(project_type::spm))
                    {
                       write_spm_project_file(group, settings.swift_version, module, namespaces, mdCache, mf);
                    }
                    if (settings.has_project_type(project_type::cmake))
                    {
                        write_cmake_project_file(group, module, namespaces, mdCache, mf);
                    }
                }
            }

            group.get();
        
            if (settings.verbose)
            {
               // w.write(" time:  %ms\n", get_elapsed_time(start));
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
