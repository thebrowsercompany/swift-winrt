// swift-tools-version: 5.7

import PackageDescription

let package = Package(
    name: "UWP",
    products: [
        .library(name: "UWP", type: .dynamic, targets: ["UWP"]),
    ],
    dependencies: [
        .package(path: "../CWinRT"),
        .package(path: "../WindowsFoundation"),
    ],
    targets: [
        .target(
            name: "UWP",
            dependencies: [
                "CWinRT",
                "WindowsFoundation",
            ],
            path: "."
        ),
    ]
)
