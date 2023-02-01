# swiftwinrt
Swift Language Projection for WinRT

This project is based on the Microsoft created [cppwinrt](https://github.com/microsoft/cppwinrt) project and carries the same source structure.

Swift/WinRT generates *both* the C ABI definition, which is imported into a Swift module *and* the actual Swift bindings which call into the ABI. 

### Developer Worklow

#### Debugging Swift/WinRT

When working on the bindings, the best experience is to use full Visual Studio (*not* VS Code) and open the [swiftwinrt cmake file](./swiftwinrt/CMakeLists.txt)
in VS and build that way. 

The build process generates a .rsp file which can be used to easily generate the bindings. So first, run a build so that this file is generated.

Select the startup project to `swiftwinrt.exe` and update the debug settings by going to
`Debug -> Debug and Launch Settings for swiftwinrt` and setting your arguments to look like this (paths may be different):


```json
{
  "version": "0.2.1",
  "defaults": {},
  "configurations": [
    {
      "type": "default",
      "project": "CMakeLists.txt",
      "projectTarget": "swiftcodegen.exe (tool\\swiftcodegen_poc\\swiftcodegen.exe)",
      "name": "swiftcodegen.exe (tool\\swiftcodegen_poc\\swiftcodegen.exe)",
      "args": [
        "@C:\\workspace\\chromium\\src\\arc\\ArcCoreDemoWin\\_build\\debug\\Source\\WinRT\\SwiftWinRT.rsp"
      ]
    }
  ]
}
```
