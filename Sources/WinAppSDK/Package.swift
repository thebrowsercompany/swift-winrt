// swift-tools-version: 5.10

import PackageDescription

let package = Package(
    name: "WinAppSDK",
    products: [
        .library(name: "WinAppSDK", type: .dynamic, targets: ["WinAppSDK"]),
    ],
    dependencies: [
        .package(path: "../CWinRT"),
        .package(path: "../CWinAppSDK"),
        .package(path: "../UWP"),
        .package(path: "../WindowsFoundation"),
    ],
    targets: [
        .target(
            name: "WinAppSDK",
            dependencies: [
                "CWinRT",
                "CWinAppSDK",
                "UWP",
                "WindowsFoundation",
            ],
            path: "."
        ),
    ]
)
