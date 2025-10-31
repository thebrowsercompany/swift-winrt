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
      var buffer: CWinRT.StaticWCharArray_512 = .init()
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

@_spi(WinRTInternal)
extension String: WinRTBridgeable {
    public typealias ABI = HSTRING?
    public func toABI() throws -> HSTRING? {
        let codeUnitCount = utf16.count
        var pointer: UnsafeMutablePointer<UInt16>? = nil
        var hStringBuffer: HSTRING_BUFFER? = nil

        // Note: Methods like String.withCString are not used here because they do a copy to create a null
        // terminated string, and requires an additional copy to create an HSTRING. Instead, a single copy is
        // done by using WindowsPreallocateStringBuffer to allocate a buffer and directly copying the string into it.
        try CHECKED(WindowsPreallocateStringBuffer(UInt32(codeUnitCount), &pointer, &hStringBuffer));
        guard let pointer else { throw Error(hr: E_FAIL) }
        _ = UnsafeMutableBufferPointer(start: pointer, count: codeUnitCount).initialize(from: utf16)
        
        do {
            var hString: HSTRING? = nil
            try CHECKED(WindowsPromoteStringBuffer(hStringBuffer, &hString));
            return hString
        } catch {
            WindowsDeleteStringBuffer(hStringBuffer)
            throw error
        }
    }

    public static func from(abi: HSTRING?) -> String {
        String(from: abi)
    }
}

extension Bool {
  public init(from val: boolean) {
    self.init(booleanLiteral: val != 0)
  }
}

@_spi(WinRTInternal)
extension Bool: WinRTBridgeable {
    public typealias ABI = boolean
    
    public func toABI() -> boolean {
        return .init(from: self)
    }
    
    public static func from(abi: boolean) -> Bool {
        return .init(from: abi)
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

@_spi(WinRTInternal)
extension Character: WinRTBridgeable {
    public typealias ABI = WCHAR
    
    public func toABI() -> WCHAR {
        return WCHAR(self.unicodeScalars.first?.value ?? 0)
    }
    
    public static func from(abi: WCHAR) -> Character {
        return Character(from: abi)
    }
}

extension UnsafeMutableRawPointer {
  public static var none : UnsafeMutableRawPointer? { return nil }
}