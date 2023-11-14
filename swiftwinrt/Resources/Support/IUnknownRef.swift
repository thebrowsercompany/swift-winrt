// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

@_fixed_layout
public final class IUnknownRef {
  private var pUnk: ComPtr<C_IUnknown>

  init<C_Interface>(_ pUnk: ComPtr<C_Interface>) {
    let pointer: UnsafeMutablePointer<C_IUnknown> =
        UnsafeMutableRawPointer(pUnk.get()).bindMemory(to: C_IUnknown.self, capacity: 1)
    self.pUnk = .init(pointer)
  }

  func detach() -> UnsafeMutableRawPointer? {
    return self.pUnk.detach()
  }

  public var borrow: UnsafeMutablePointer<C_IUnknown> {
    return self.pUnk.get()
  }
}
