// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import CWinRT
import Foundation
import ucrt

@_fixed_layout
final public class HString {
  internal private(set) var hString: HSTRING?
  
  public init(_ string: String) throws {
    self.hString = try string.withCString(encodedAs: UTF16.self) {
      var result: HSTRING?
      try CHECKED(WindowsCreateString($0, UINT32(wcslen($0)), &result))
      return result
    }
  }

  public init(_ hString: HSTRING?) throws {
    try CHECKED(WindowsDuplicateString(hString, &self.hString))
  }

  public init(consuming hString: HSTRING?) {
    self.hString = hString
  }

  public init() { }
  public init(_ buffer: UnsafeBufferPointer<WCHAR>?) throws {
    try CHECKED(WindowsCreateString(buffer?.baseAddress ?? nil,
                                    UINT32(buffer?.count ?? 0),
                                    &self.hString))
  }

  public func get() -> HSTRING? { hString }

  public func detach() -> HSTRING? {
    let tempString = hString
    hString = nil
    return tempString
  }

  deinit {
    try! CHECKED(WindowsDeleteString(self.hString))
  }
}

