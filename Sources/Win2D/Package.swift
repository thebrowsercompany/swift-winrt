// swift-tools-version: 5.10

import PackageDescription

let package = Package(
    name: "Win2D",
    products: [
        .library(name: "Win2D", type: .dynamic, targets: ["Win2D"]),
    ],
     dependencies: [
        .package(path: "../CWinRT"),
        .package(path: "../UWP"),
        .package(path: "../WindowsAppSDK"),
        .package(path: "../WindowsFoundation"),
        .package(path: "../WinUI"),
    ],
    targets: [
        .target(
            name: "Win2D",
            dependencies: [
                "CWinRT",
                "UWP",
                "WindowsFoundation",
                "WinUI",
            ],
            path: "."
        ),
    ]
)
