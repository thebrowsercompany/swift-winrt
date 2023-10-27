# Swift/WinRT

The TheBrowserCompany.SwiftWinRT NuGet package automatically generates swift code, enabling you consume Windows Runtime classes.

The Swift/WinRT package is a tool only package. The tool requires the user to gather the appropriate inputs.

## Usage

Below is a table of the appropriate parameters to pass to swiftwinrt.exe

| Name         |    Description                                  |
|--------------|-------------------------------------------------|
| `-input`     | Input winmd file for generating bindings from   |
| `-include`   | Types/Namespaces to generate bindings for.      |
| `-exclude`   | Types/Namespaces to exclude                     |
| `-reference` | WinMD file for reference or Windows SDK version |
| `-output`    | Output location to place generated bindings     |
| `@<path.rsp>`| path to .rsp file with all parameters           |

Swift/WinRT is flexible on the bindings that it will generate. No matter the combination of parameters you provide, it will ensure you have code that will compile. Note that specific excludes will always override an include. It is recommended to use a response file to pass parameters to Swift/WinRT. Here is an example of what a response file may look like:

```
-include MyComponent
-include Windows.Foundation.Collections
-input C:/dev/MyComponent/build/MyComponent.winmd
-output C:/dev/MyComponent/Sources/MyComponent
-reference 10.0.17763.0
```

You could then invoke Swift/WinRT like:

> swiftwinrt.exe @c:/dev/MyComponent/MyComponent.rsp