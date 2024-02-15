// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import CWinRT

extension String {
  public init(from hString: HSTRING?) {
    var length: UINT32 = 0
    let pwszBuffer: PCWSTR = WindowsGetStringRawBuffer(hString, &length)
    self.init(decoding: UnsafeBufferPointer(start: pwszBuffer, count: Int(length)), as: UTF16.self)
  }

  public init(hString: HString) {
    var length: UINT32 = 0
    let pwszBuffer: PCWSTR = WindowsGetStringRawBuffer(hString.get(), &length)
    self.init(decoding: UnsafeBufferPointer(start: pwszBuffer, count: Int(length)), as: UTF16.self)
  }
}

extension Bool {
  public init(from val: boolean) {
    self.init(booleanLiteral: val != 0)
  }
}

extension Character {
  public init(from wchar: WCHAR) {
    if let scalar = Unicode.Scalar(wchar) {
      self.init(scalar)
    } else {
      self.init("")
    }
  }
}

extension UnsafeMutableRawPointer {
  public static var none : UnsafeMutableRawPointer? { return nil }
}