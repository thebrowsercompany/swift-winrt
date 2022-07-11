// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import TestComponent_CWinRT

public class HString {
  public private(set) var hRef: HStringStorage

  public init(_ string: String?) throws {

    if let string = string {
      self.hRef = string.withWideChars {
          var out: HSTRING?
          WindowsCreateString($0, UInt32(string.count), &out)
          return HStringStorage(consuming: out)
        }
    } else {
      self.hRef = .init()
    }
    
  }

  public init() { self.hRef = .init() }

  public init(_ buffer: UnsafeBufferPointer<WCHAR>?) throws {
    self.hRef = try HStringStorage(buffer)
  }

  public init(consuming hString: TestComponent_CWinRT.HSTRING?) {
    self.hRef = HStringStorage(consuming: hString)
  }

  public func get() -> HSTRING? { hRef.hString }

  public func detach() -> HSTRING? {
    return hRef.detach()
  }
}