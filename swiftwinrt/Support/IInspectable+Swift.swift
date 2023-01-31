// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import C_BINDINGS_MODULE

extension IInspectable {
  public func GetIids() throws -> [IID] {
    var iids: UnsafeMutablePointer<IID>?
    var iidCount: ULONG = 0

    try self.GetIids(&iidCount, &iids)
    defer { CoTaskMemFree(iids) }

    return Array<IID>(unsafeUninitializedCapacity: Int(iidCount)) {
      _ = memcpy($0.baseAddress, iids, MemoryLayout<IID>.stride * Int(iidCount))
      $1 = Int(iidCount)
    }
  }

  public func GetRuntimeClassName() throws -> HString {
    var className: C_BINDINGS_MODULE.HSTRING?
    try self.GetRuntimeClassName(&className)
    return HString(consuming: className)
  }

  public func GetTrustLevel() throws -> TrustLevel {
    var trustLevel: TrustLevel = .BaseTrust
    try self.GetTrustLevel(&trustLevel)
    return trustLevel
  }

  // maps the namespace to a swift module. needs to be kept in sync with
  // get_swift_module defined in common.h in the code generator
  private func GetSwiftModule(from ns: String) -> String {
    if ns.starts(with: "Windows.Foundation") {
       return "WindowsFoundation"
    }
     else if ns.starts(with: "Windows.Storage") || 
              ns.starts(with: "Windows.System") ||
              ns.starts(with: "Windows.UI") ||
              ns.starts(with: "Windows.Networking") ||
              ns.starts(with: "Windows.ApplicationModel")
      {
          return "WindowsApplicationModel"
      }
      else if ns.starts(with: "Microsoft.UI.Composition")
      {
          return "MicrosoftUIComposition"
      }
      else if ns.starts(with: "Microsoft.UI.Xaml")
      {
          return "MicrosoftUIXaml"
      }
      else
      {
          var mod: String = ns
          mod.removeAll(where: { $0 == "." })
          return mod
      }
  }

  public func GetSwiftClassName() throws -> String {
    let className = try! String(hString: GetRuntimeClassName())
    let lastNsIndex = className.lastIndex(of: ".")
    guard let lastNsIndex = lastNsIndex else {
      fatalError("invalid class name")
    }
    let ns = className.prefix(upTo: lastNsIndex)
    print("namespace: ", ns)
    let lastNsIndexPlus1 = className.index(after: lastNsIndex)
    let typeName = className.suffix(from: lastNsIndexPlus1)
    print("type:", typeName)
    return GetSwiftModule(from: String(ns)) + "." + typeName
  }
}
