// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import WinSDK

@_fixed_layout
@usableFromInline
internal final class IUnknownRef {
  private var pUnk: UnsafeMutablePointer<WinSDK.IUnknown>?

  init(_ pUnk: UnsafeMutablePointer<WinSDK.IUnknown>?) {
    self.pUnk = pUnk
    _ = self.pUnk?.pointee.lpVtbl.pointee.AddRef(self.pUnk)
  }

  convenience init(_ pointer: UnsafeMutableRawPointer?) {
    let pUnk = pointer?.bindMemory(to: WinSDK.IUnknown.self, capacity: 1)
    self.init(pUnk)
  }

  init(consuming pUnk: UnsafeMutablePointer<WinSDK.IUnknown>?) {
    self.pUnk = pUnk
  }

  convenience init(consuming pointer: UnsafeMutableRawPointer?) {
    let pUnk = pointer?.bindMemory(to: WinSDK.IUnknown.self, capacity: 1)
    self.init(consuming: pUnk)
  }

  deinit {
    _ = self.pUnk?.pointee.lpVtbl.pointee.Release(self.pUnk)
  }

  @usableFromInline
  internal var borrow: UnsafeMutablePointer<WinSDK.IUnknown>? {
    return self.pUnk
  }

  @usableFromInline
  internal var ref: UnsafeMutablePointer<WinSDK.IUnknown>? {
    _ = self.pUnk?.pointee.lpVtbl.pointee.AddRef(self.pUnk)
    return self.pUnk
  }
}
