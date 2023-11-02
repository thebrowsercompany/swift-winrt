// Copyright © 2023 The Browser Company
// SPDX-License-Identifier: BSD-3
import C_BINDINGS_MODULE

fileprivate extension ComPtr {
    func asIInspectable<ResultType>(_ body: (UnsafeMutablePointer<C_IInspectable>) throws -> ResultType) rethrows -> ResultType? {
        try self.get().withMemoryRebound(to: C_IInspectable.self, capacity: 1) { try body($0) }
    }
}

open class WinRTObject<CInterface>: ComObject<CInterface>, IInspectable2 {
  public func GetIids(_ iidCount: UnsafeMutablePointer<ULONG>, _ iids: UnsafeMutablePointer<UnsafeMutablePointer<test_component.IID>?>?) throws {
    try ptr.asIInspectable { pThis in
      try CHECKED(pThis.pointee.lpVtbl.pointee.GetIids(pThis, iidCount, iids))
    }
  }

  public func GetRuntimeClassName(_ className: UnsafeMutablePointer<HSTRING?>) throws {
    try ptr.asIInspectable { pThis in
      try CHECKED(pThis.pointee.lpVtbl.pointee.GetRuntimeClassName(pThis, className))
    }
  }

  public func GetTrustLevel(_ trustLevel: UnsafeMutablePointer<TrustLevel>?) throws {
    try ptr.asIInspectable { pThis in
      try CHECKED(pThis.pointee.lpVtbl.pointee.GetTrustLevel(pThis, trustLevel))
    }
  }
}
