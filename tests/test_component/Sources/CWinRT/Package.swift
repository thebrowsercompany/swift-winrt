// swift-tools-version:5.7

import Foundation
import PackageDescription

let package = Package(
    name: "CWinRT",
    products: [
        .library(name: "CWinRT", type: .dynamic, targets: ["CWinRT"]),
    ],
    targets: [
        .target(
            name: "CWinRT",
            path: ".",
            linkerSettings: [
                .unsafeFlags(["-nostartfiles"]),
            ]
        ),
    ]
)
