// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import WinSDK

@_fixed_layout
public final class IUnknownRef {
  private var pUnk: UnsafeMutablePointer<C_IUnknown>

  init(_ pUnk: UnsafeMutablePointer<C_IUnknown>) {
    self.pUnk = pUnk
    _ = self.pUnk.pointee.lpVtbl.pointee.AddRef(self.pUnk)
  }

  convenience init(_ pointer: UnsafeMutableRawPointer) {
    let pUnk = pointer.bindMemory(to: C_IUnknown.self, capacity: 1)
    self.init(pUnk)
  }

  init(consuming pUnk: UnsafeMutablePointer<C_IUnknown>) {
    self.pUnk = pUnk
    // TODO: WIN-158 we shouldn't need to addref because these pointers already have an
    // added reference coming from winrt. However, this helps the app not crash so
    // doing this for now
    _ = self.pUnk.pointee.lpVtbl.pointee.AddRef(self.pUnk)
  }

  convenience init(consuming pointer: UnsafeMutableRawPointer) {
    let pUnk = pointer.bindMemory(to: C_IUnknown.self, capacity: 1)
    self.init(consuming: pUnk)
  }

  deinit {
    _ = self.pUnk.pointee.lpVtbl.pointee.Release(self.pUnk)
  }

 public var borrow: UnsafeMutablePointer<C_IUnknown> {
    return self.pUnk
  }

  public var ref: UnsafeMutablePointer<C_IUnknown> {
    _ = self.pUnk.pointee.lpVtbl.pointee.AddRef(self.pUnk)
    return self.pUnk
  }
}
