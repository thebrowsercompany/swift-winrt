// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import C_BINDINGS_MODULE

public class HString {
  public private(set) var hRef: HStringStorage

public init(_ string: String?) throws {
    if let string = string {
      self.hRef = try string.withWideChars {
          var out: HSTRING?
          try CHECKED(WindowsCreateString($0, UINT32(string.count), &out))
          return HStringStorage(consuming: out)
        }
    } else {
      self.hRef = .init()
    }
    
  }

  public init(_ buffer: UnsafeBufferPointer<WCHAR>?) throws {
    self.hRef = try HStringStorage(buffer)
  }

  public init(consuming hString: C_BINDINGS_MODULE.HSTRING?) {
    self.hRef = HStringStorage(consuming: hString)
  }

  public func get() -> HSTRING? { hRef.hString }

  public func detach() -> HSTRING? {
    return hRef.detach()
  }
}