// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import WinSDK
import Ctest_component

fileprivate let IID_IUnknown = IID(Data1: 0x00000000, Data2: 0x0000, Data3: 0x0000, Data4: ( 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 )) // 00000000-0000-0000-C000-000000000046

open class IUnknown : HasIID {
  public let pUnk: IUnknownRef

  open class var IID: test_component.IID { IID_IUnknown }

  public required init(_ pointer: UnsafeMutablePointer<C_IUnknown>) {
    self.pUnk = IUnknownRef(pointer)
  }

  public required init(_ pointer: UnsafeMutableRawPointer) {
    let pUnk: UnsafeMutablePointer<C_IUnknown> =
        pointer.bindMemory(to: C_IUnknown.self, capacity: 1)
    self.pUnk = IUnknownRef(pUnk)
  }

  public required init(consuming pointer: UnsafeMutablePointer<C_IUnknown>) {
    self.pUnk = IUnknownRef(consuming: pointer)
  }

  public init(consuming pointer: UnsafeMutableRawPointer) {
    let pUnk: UnsafeMutablePointer<C_IUnknown> =
        pointer.bindMemory(to: C_IUnknown.self, capacity: 1)
    self.pUnk = IUnknownRef(consuming: pUnk)
  }

  @_alwaysEmitIntoClient @inline(__always)
  public func QueryInterface(_ iid: REFIID, _ ppvObject: UnsafeMutablePointer<UnsafeMutableRawPointer?>?) throws {
    let pUnk: UnsafeMutablePointer<C_IUnknown>! = self.pUnk.borrow
    try CHECKED(pUnk.pointee.lpVtbl.pointee.QueryInterface(pUnk, iid, ppvObject))
  }

  @_alwaysEmitIntoClient @inline(__always) @discardableResult
  public func AddRef() -> ULONG {
    let pUnk: UnsafeMutablePointer<C_IUnknown>! = self.pUnk.borrow
    return pUnk.pointee.lpVtbl.pointee.AddRef(pUnk)
  }

  @_alwaysEmitIntoClient @inline(__always) @discardableResult
  public func Release() -> ULONG {
    let pUnk: UnsafeMutablePointer<C_IUnknown>! = self.pUnk.borrow
    return pUnk.pointee.lpVtbl.pointee.Release(pUnk)
  }
}
