// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import WinSDK
import Ctest_component

open class IUnknown : HasIID {
  public let pUnk: IUnknownRef

  open class var IID: IID { IID_IUnknown }

  public required init(_ pointer: UnsafeMutablePointer<WinSDK.IUnknown>) {
    self.pUnk = IUnknownRef(pointer)
  }

  public required init(_ pointer: UnsafeMutableRawPointer) {
    let pUnk: UnsafeMutablePointer<WinSDK.IUnknown> =
        pointer.bindMemory(to: WinSDK.IUnknown.self, capacity: 1)
    self.pUnk = IUnknownRef(pUnk)
  }

  public required init(consuming pointer: UnsafeMutablePointer<WinSDK.IUnknown>) {
    self.pUnk = IUnknownRef(consuming: pointer)
  }

  public init(consuming pointer: UnsafeMutableRawPointer) {
    let pUnk: UnsafeMutablePointer<WinSDK.IUnknown> =
        pointer.bindMemory(to: WinSDK.IUnknown.self, capacity: 1)
    self.pUnk = IUnknownRef(consuming: pUnk)
  }
  
  @_alwaysEmitIntoClient @inline(__always)
  public func QueryInterface(_ iid: REFIID, _ ppvObject: UnsafeMutablePointer<UnsafeMutableRawPointer?>?) throws {
    let pUnk: UnsafeMutablePointer<WinSDK.IUnknown>! = self.pUnk.borrow
    try CHECKED(pUnk.pointee.lpVtbl.pointee.QueryInterface(pUnk, iid, ppvObject))
  }

  @_alwaysEmitIntoClient @inline(__always)
  public func AddRef() -> ULONG {
    let pUnk: UnsafeMutablePointer<WinSDK.IUnknown>! = self.pUnk.borrow
    return pUnk.pointee.lpVtbl.pointee.AddRef(pUnk)
  }

  @_alwaysEmitIntoClient @inline(__always)
  public func Release() -> ULONG {
    let pUnk: UnsafeMutablePointer<WinSDK.IUnknown>! = self.pUnk.borrow
    return pUnk.pointee.lpVtbl.pointee.Release(pUnk)
  }
}
