// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

@_fixed_layout
public final class IUnknownRef {
  private var pUnk: ComPtr<C_IUnknown>

  init<C_Interface>(_ pUnk: consuming ComPtr<C_Interface>) {
    let raw = pUnk.detach()
    self.pUnk = ComPtr(consuming: raw?.assumingMemoryBound(to: C_IUnknown.self))!
  }

  func detach() -> UnsafeMutableRawPointer? {
    return self.pUnk.detach()
  }

  public var borrow: UnsafeMutablePointer<C_IUnknown> {
    return self.pUnk.get()
  }
}
