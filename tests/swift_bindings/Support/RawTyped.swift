// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import WinSDK

@_spi(IUnknown)
public func RawPointer<T: IUnknown, U>(_ pUnk: T?) -> UnsafeMutablePointer<U>? {
  guard let pUnk = pUnk else { return nil }
  if let pUnk: UnsafeMutableRawPointer = UnsafeMutableRawPointer(pUnk.pUnk.borrow) {
    return pUnk.bindMemory(to: U.self, capacity: 1)
  }
  return nil
}
