// swift-tools-version: 5.10

import PackageDescription

let package = Package(
    name: "swift-winrt",
    products: [
        .library(name: "swift-winrt", targets: ["swift-winrt"]),
    ],
    dependencies: [
        .package(path: "Sources/CWinRT"),
        .package(path: "Sources/UWP"),
        .package(path: "Sources/WebView2Core"),
        .package(path: "Sources/Win2D"),
        .package(path: "Sources/WinAppSDK"),
        .package(path: "Sources/WindowsFoundation"),
        .package(path: "Sources/WinUI"),
    ],
    targets: [
        .target(
            name: "swift-winrt",
            dependencies: [
                .product(name: "WindowsFoundation", package: "WindowsFoundation"),
                .product(name: "WinAppSDK", package: "WinAppSDK"),
                .product(name: "WinUI", package: "WinUI"),
                .product(name: "Win2D", package: "Win2D"),
                .product(name: "WebView2Core", package: "WebView2Core"),
                .product(name: "UWP", package: "UWP"),
            ]
        ),
    ]
)
