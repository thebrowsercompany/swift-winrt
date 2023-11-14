// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import WinSDK

public func RawPointer<T: IUnknown, U>(_ pUnk: T) -> UnsafeMutablePointer<U> {
  return UnsafeMutableRawPointer(pUnk.pUnk.borrow).bindMemory(to: U.self, capacity: 1)
}

public func RawPointer<T: IUnknown, U>(_ pUnk: T) -> ComPtr<U> {
  return ComPtr(UnsafeMutableRawPointer(pUnk.pUnk.borrow).bindMemory(to: U.self, capacity: 1))
}

public func RawPointer<T: IUnknown, U>(_ pUnk: T?) -> UnsafeMutablePointer<U>? {
  guard let pUnk else { return nil }
  let result: UnsafeMutablePointer<U> = RawPointer(pUnk)
  return result
}

public func RawPointer<T: WinRTClass, U>(_ obj: T?) -> UnsafeMutablePointer<U>? {
  return obj?._getABI()
}

public func RawPointer<T: AbiInterfaceImpl, U>(_ obj: T) -> UnsafeMutablePointer<U> {
  return RawPointer(obj._default)
}

public func RawPointer<T: AbiInterfaceImpl, U>(_ obj: T?) -> UnsafeMutablePointer<U>? {
  guard let obj else { return nil}
  let result: UnsafeMutablePointer<U> = RawPointer(obj)
  return result
}