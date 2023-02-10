// swift-tools-version:5.7

import Foundation
import PackageDescription

let sdkRoot = ProcessInfo.processInfo.environment["SDKROOT"]!
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
                .unsafeFlags(["-Xlinker", "\(sdkRoot)\\usr\\lib\\swift\\windows\\x86_64\\swiftCore.lib"]),
            ]
        ),
    ]
)
