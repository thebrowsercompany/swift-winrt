// swift-tools-version: 5.10

import PackageDescription

let package = Package(
    name: "WebView2Core",
    products: [
        .library(name: "WebView2Core", type: .dynamic, targets: ["WebView2Core"]),
    ],
    dependencies: [
        .package(path: "../CWinRT"),
        .package(path: "../UWP"),
        .package(path: "../WindowsFoundation"),
    ],
    targets: [
        .target(
            name: "WebView2Core",
            dependencies: [
                "CWinRT",
                "UWP",
                "WindowsFoundation",
            ],
            path: "."
        ),
    ]
)
