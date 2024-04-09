// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import C_BINDINGS_MODULE
import Foundation
import ucrt

@_fixed_layout
final public class HString {
  internal private(set) var hString: HSTRING?
  
  public init(_ string: String) throws {

    let codeUnitCount = string.utf16.count
    var pointer: UnsafeMutablePointer<UInt16>? = nil
    var hStringBuffer: HSTRING_BUFFER? = nil

    try CHECKED(WindowsPreallocateStringBuffer(UInt32(codeUnitCount), &pointer, &hStringBuffer));

    guard let pointer else { throw Error(hr: E_FAIL) }
        _ = UnsafeMutableBufferPointer(start: pointer, count: codeUnitCount).initialize(from: string.utf16)

    var hString: HSTRING?
        do { 
            try CHECKED(WindowsPromoteStringBuffer(hStringBuffer, &hString));
        }
        catch {
            WindowsDeleteStringBuffer(hStringBuffer)
            throw error
        }

    self.hString = hString
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

