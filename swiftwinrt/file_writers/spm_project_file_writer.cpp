#include "spm_project_file_writer.h"
#include "utility/type_writers.h"
#include "utility/separator.h"
namespace swiftwinrt
{
    void write_spm_project_file(
        std::string const& swiftToolsVersion,
        std::string_view const& module,
        std::set<std::string_view> const& dependencies)
    {
        writer w;
        w.type_namespace = module;

        auto path = w.root_directory() / module / "Package.swift";
        auto exclude_cmake = settings.has_project_type(project_type::cmake) ? R"(,
            exclude: [ "CMakeLists.txt" ])" : "";
        auto package = R"(// swift-tools-version: %

import PackageDescription

let package = Package(
    name: "%",
    products: [
        .library(name: "%", type: .dynamic, targets: ["%"]),
    ],
    dependencies: [
        .package(path: "../CWinRT"),%
    ],
    targets: [
        .target(
            name: "%",
            dependencies: [
                "CWinRT",%
            ],
            path: "."%
        ),
    ]
)
)";

        w.write(package,
            swiftToolsVersion,
            module,
            module, module,
            bind([&](writer& w) {
        for (auto&& module : dependencies)
        {
            w.write(R"(
        .package(path: "../%"),)", module);
        }}),
            module,
            bind([&](writer& w) {
            for (auto module : dependencies)
            {
                w.write(R"(
                "%",)", module);
            }}
        ), exclude_cmake);
      
        w.flush_to_file(path);
    }
}