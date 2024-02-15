// swift-tools-version: 5.10

import PackageDescription
import Foundation

let currentDirectory = Context.packageDirectory

let linkerSettings: [LinkerSetting] = [
/* Figure out magic incantation so we can delay load these dlls
    .unsafeFlags(["-L\(currentDirectory)/native_assets/lib"]),
    .unsafeFlags(["-Xlinker" , "/DELAYLOAD:Microsoft.WindowsAppRuntime.Bootstrap.dll"]),
*/
]

let package = Package(
    name: "CWinAppSDK",
    products: [
        .library(name: "CWinAppSDK", targets: ["CWinAppSDK"]),
    ],
    dependencies: [
    ],
    targets: [
        .target(
            name: "CWinAppSDK",
            path: ".",
            resources: [
                .copy("native_assets/bin/Microsoft.WindowsAppRuntime.Bootstrap.dll"),
            ],
            linkerSettings: linkerSettings
        )
    ]
)
