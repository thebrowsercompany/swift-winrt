// swift-tools-version:5.7

import Foundation
import PackageDescription

let package = Package(
    name: "Ctest_component",
    products: [
        .library(name: "Ctest_component", type: .dynamic, targets: ["Ctest_component"]),
    ],
    targets: [
        .target(
            name: "Ctest_component",
            path: ".",
            linkerSettings: [
                .unsafeFlags(["-nostartfiles"]),
            ]
        ),
    ]
)
