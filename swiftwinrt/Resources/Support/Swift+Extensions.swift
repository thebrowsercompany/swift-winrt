// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import C_BINDINGS_MODULE

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

  public func withHStringRef(_ body: (HSTRING?) throws -> Void) rethrows {
    try self.withCString(encodedAs: UTF16.self) {
      var hString: HSTRING?
      var header: HSTRING_HEADER = .init()
      try CHECKED(WindowsCreateStringReference($0, UInt32(self.count), &header, &hString))
      try body(hString)
    }
  }
}

extension StaticString {
    public func withHStringRef(_ body: (HSTRING?) throws -> Void) rethrows {
      var buffer: C_BINDINGS_MODULE.StaticWCharArray_512 = .init()
      try withUnsafeMutableBytes(of: &buffer.Data) { (bytes:UnsafeMutableRawBufferPointer) in
          let bytesPtr = bytes.assumingMemoryBound(to: WCHAR.self)
          self.withUTF8Buffer { utf8buffer in
              try! CHECKED(MultiByteToWideChar(UInt32(CP_UTF8), 0, utf8buffer.baseAddress, Int32(utf8buffer.count), bytesPtr.baseAddress, Int32(bytes.count)))
          }
          var hString: HSTRING?
          var header: HSTRING_HEADER = .init()
          try CHECKED(WindowsCreateStringReference(bytesPtr.baseAddress, UINT32(self.utf8CodeUnitCount), &header, &hString))
          try body(hString)
        }
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