// swift-tools-version: 5.7

import PackageDescription

let package = Package(
    name: "test_component",
    products: [
        .library(name: "test_component", type: .dynamic, targets: ["test_component"]),
    ],
    dependencies: [
        .package(path: "../CWinRT"),
        .package(path: "../UWP"),
        .package(path: "../WindowsFoundation"),
    ],
    targets: [
        .target(
            name: "test_component",
            dependencies: [
                "CWinRT",
                "UWP",
                "WindowsFoundation",
            ],
            path: "."
        ),
    ]
)
