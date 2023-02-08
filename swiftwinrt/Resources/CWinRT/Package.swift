// swift-tools-version:5.7

import Foundation
import PackageDescription

let sdkRoot = ProcessInfo.processInfo.environment["SDKROOT"] ?? "C:\\Library\\Developer\\Platforms\\Windows.platform\\Developer\\SDKs\\Windows.sdk"
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
                .unsafeFlags(["-Xlinker", "\(sdkRoot)\\usr\\lib\\swift\\windows\\x86_64\\swiftCore.lib"]),
            ]
        ),
    ]
)