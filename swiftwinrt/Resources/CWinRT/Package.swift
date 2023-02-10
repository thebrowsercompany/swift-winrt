// swift-tools-version:5.7

import Foundation
import PackageDescription

let package = Package(
    name: "C_BINDINGS_MODULE",
    products: [
        .library(name: "C_BINDINGS_MODULE", type: .dynamic, targets: ["C_BINDINGS_MODULE"]),
    ],
    targets: [
        .target(
            name: "C_BINDINGS_MODULE",
            path: ".",
            linkerSettings: [
                .unsafeFlags(["-nostartfiles"]),
            ]
        ),
    ]
)
