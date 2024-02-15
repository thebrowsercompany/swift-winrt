// swift-tools-version: 5.10

import PackageDescription

let package = Package(
    name: "WinUI",
    products: [
        .library(name: "WinUI", type: .dynamic, targets: ["WinUI"]),
    ],
    dependencies: [
        .package(path: "../CWinRT"),
        .package(path: "../UWP"),
        .package(path: "../WinAppSDK"),
        .package(path: "../WindowsFoundation"),
        .package(path: "../WebView2Core"),
    ],
    targets: [
        .target(
            name: "WinUI",
            dependencies: [
                "CWinRT",
                "UWP",
                "WinAppSDK",
                "WindowsFoundation",
                "WebView2Core",
            ],
            path: "."
        ),
    ]
)
