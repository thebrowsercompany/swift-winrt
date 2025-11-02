// swift-tools-version: 5.7

import PackageDescription

let package = Package(
    name: "WindowsFoundation",
    products: [
        .library(name: "WindowsFoundation", type: .dynamic, targets: ["WindowsFoundation"]),
    ],
    dependencies: [
        .package(path: "../CWinRT"),
    ],
    targets: [
        .target(
            name: "WindowsFoundation",
            dependencies: [
                "CWinRT",
            ],
            path: "."
        ),
    ]
)
