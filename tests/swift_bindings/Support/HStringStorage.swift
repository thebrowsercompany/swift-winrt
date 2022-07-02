// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import TestComponent_CWinRT

@_fixed_layout
final public class HStringStorage {
  internal private(set) var hString: HSTRING?

  public init(_ hString: HSTRING?) throws {
    try CHECKED(WindowsDuplicateString(hString, &self.hString))
  }

  public init(consuming hString: HSTRING?) {
    self.hString = hString
  }

  public init(_ buffer: UnsafeBufferPointer<WCHAR>?) throws {
    try CHECKED(WindowsCreateString(buffer?.baseAddress ?? nil,
                                    UINT32(buffer?.count ?? 0),
                                    &self.hString))
  }

  public func detach() -> HSTRING? {
    let tempString = hString
    hString = nil
    return tempString
  }
  
  deinit {
    try! CHECKED(WindowsDeleteString(self.hString))
  }
}

