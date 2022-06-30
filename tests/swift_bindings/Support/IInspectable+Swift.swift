// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import TestComponent_CWinRT

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
    var className: TestComponent_CWinRT.HSTRING?
    try self.GetRuntimeClassName(&className)
    return HString(consuming: className)
  }

  public func GetTrustLevel() throws -> TrustLevel {
    var trustLevel: TrustLevel = .BaseTrust
    try self.GetTrustLevel(&trustLevel)
    return trustLevel
  }
}
