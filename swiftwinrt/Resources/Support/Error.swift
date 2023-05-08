// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import WinSDK
import C_BINDINGS_MODULE

private func getDescriptionFromErrorInfo() -> String? {
  var errorInfo: UnsafeMutablePointer<IRestrictedErrorInfo>?
  var result = GetRestrictedErrorInfo(&errorInfo)
  guard result == S_OK else { return nil }
  defer {
    _ = errorInfo?.pointee.lpVtbl.pointee.Release(errorInfo)
  }
  var errorDescription: BSTR?
  var restrictedDescription: BSTR?
  var capabilitySid: BSTR?
  defer {
    SysFreeString(errorDescription)
    SysFreeString(restrictedDescription)
    SysFreeString(capabilitySid)
  }
  _ = errorInfo?.pointee.lpVtbl.pointee.GetErrorDetails(
    errorInfo!,
    &errorDescription,
    &result,
    &restrictedDescription,
    &capabilitySid)
  
  guard SysStringLen(errorDescription) > 0 else { return nil }
  return String(decodingCString: errorDescription!, as: UTF16.self)
}

private func hrToString(_ hr: HRESULT) -> String {
  let dwFlags: DWORD = DWORD(FORMAT_MESSAGE_ALLOCATE_BUFFER)
                       | DWORD(FORMAT_MESSAGE_FROM_SYSTEM)
                       | DWORD(FORMAT_MESSAGE_IGNORE_INSERTS)

    var buffer: UnsafeMutablePointer<WCHAR>? = nil
    let dwResult: DWORD = withUnsafeMutablePointer(to: &buffer) {
      $0.withMemoryRebound(to: WCHAR.self, capacity: 2) {
        FormatMessageW(dwFlags, nil, DWORD(bitPattern: hr),
                       MAKELANGID(WORD(LANG_NEUTRAL), WORD(SUBLANG_DEFAULT)),
                       $0, 0, nil)
      }
    }
    guard dwResult > 0, let message = buffer else {
      return "HRESULT(0x\(String(DWORD(bitPattern: hr), radix: 16)))"
    }
    defer { LocalFree(buffer) }
    return "0x\(String(DWORD(bitPattern: hr), radix: 16)) - \(String(decodingCString: message, as: UTF16.self))"
}

public struct Error: Swift.Error, CustomStringConvertible {
  public let hr: HRESULT
  public let description: String
  public init(hr: HRESULT) {
    self.hr = hr

    if let errorDescription = getDescriptionFromErrorInfo() {
      self.description = errorDescription
    } else {
      self.description = hrToString(hr)
    }

  }
}


public func failWith(err: HRESULT) -> HRESULT {
  return err
}
