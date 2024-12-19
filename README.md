# Swift/WinRT
Swift Language Projection for WinRT

This project is based on the Microsoft created [cppwinrt](https://github.com/microsoft/cppwinrt) project and carries the same source structure.

Swift/WinRT generates *both* the C ABI definition, which is imported into a Swift module *and* the actual Swift bindings which call into the ABI.

## Project Structure

This project has 4 different main projects:
1. [SwiftWinRT](./swiftwinrt) (C++)
2. [TestComponent](./tests/test_component/cpp/) (C++)
3. [Swift Bindings](./tests/test_component/Sources/) (Swift)
4. [Test app](./tests/test_app/) (Swift)

For all C++ code (1 and 2 above) - they are built with **CMake**
For all Swift code (3 and 4 above) - they are built with **SPM**

## Developer Workflow

### Swift Toolchain
This project may require Swift toolchain builds which are more recent than the latest released version. For best results, find the release tag used by the [GitHub Actions build workflow](.github/actions/windows-build/action.yml) and download the corresponding toolchain build from [swift-build releases](https://github.com/thebrowsercompany/swift-build/releases).

### Initial Setup

Initialize submodules:
```
git submodule init
git submodule update --recursive
```

Install the required version of the Windows SDK if you don't already have it, currently `10.0.17763`. Note that the WinGet package id for that version of the Windows SDK incorrectly uses `10.0.17736` as the version number.

```
winget install --id Microsoft.WindowsSDK.10.0.17736
```

### Integrated Build with CMake

While SPM is the driving force for all Swift related code, we still maintain an integrated build system with CMake, which invokes SPM as can be seen [here](./tests/CMakeLists.txt)

You can use the following commands to build the project:

```
cmake --preset debug
cmake --build --preset debug
cmake --build --preset debug --target install
```

or, for a one-liner (on powershell);
`cmake --build --preset debug; cmake --build --preset debug --target install`

**NOTE: you don't want to re-run configure every-time, so the one-liner omits that step**

While it is possible to use this one-liner for your inner-loop, there is a known issue which causes the swift-winrt build to be dirtied every time, and re-running swift-winrt in debug mode is very slow! This is why we set `CMAKE_SKIP_INSTALL_ALL_DEPENDENCY` in our [CMakePresets.json](./CMakePresets.json). So when you can building/testing in `release` mode will be quicker. Generally, this is the golden path forward for when you don't need to debug tests. However, if you need to debug tests, then you will likely want to build debug. See [below](#optimal-developer-workflow-for-debugging-tests) for optimal `debug` config workflow.

### Debugging Tests in Visual Studio Code

The test code (written in Swift) is easily buildable and debuggable in VS Code. You can build using `Ctrl+Shift+B` and then debug via the standard VSCode debug window (or press `F5`).

**NOTE: When building tests in VSCode, swift-winrt *is not* re-run, as this only runs the SPM portion of the build.**

#### Optimal developer workflow for debugging tests

If you need to build debug, then the following workflow is recommended:
1. Open swiftwinrt directory in Visual Studio and select the release configuration (see [Debugging swift-winrt in Visual Studio](#debugging-swiftwinrt-in-visual-studio))
2. Open VSCode to the root of the repo and ensure the `Debug` preset is selected.

This workflow has a couple key benefits:
1. This will let you quickly re-run swift-winrt in release mode when needed while still being able to only build debug for the tests.
2. Controlling when swift-winrt is re-run has the benefit of allowing you to hand-edit any files you need to prototype with APIs.

You do have to be careful that when you are done iterating (especially on handwritten changes) that you run a full build. However, your PR will fail if this happens.

### Debugging swift-winrt in Visual Studio

When working on the bindings, the best experience is to use full Visual Studio (*not* VS Code) and open the [swift-winrt cmake file](./swiftwinrt/CMakeLists.txt)
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
      "projectTarget": "swiftwinrt.exe (swiftwinrt.exe)",
      "name": "swiftwinrt.exe (swiftwinrt.exe)",
      "args": [
        "@C:\\workspace\\swiftwinrt\\build\\debug\\tests\\test_component\\SwiftWinRT.rsp"
      ]
    }
  ]
}
```
