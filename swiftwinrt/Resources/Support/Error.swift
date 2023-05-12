// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import WinSDK
import C_BINDINGS_MODULE

// winerror.h

@_transparent
public var E_FAIL: WinSDK.HRESULT {
  HRESULT(bitPattern: 0x80004005)
}

@_transparent
public var E_INVALIDARG: WinSDK.HRESULT {
  HRESULT(bitPattern: 0x80070057)
}

@_transparent
public var E_NOTIMPL: WinSDK.HRESULT {
  HRESULT(bitPattern: 0x80004001)
}

@_transparent
public var E_NOINTERFACE: WinSDK.HRESULT {
  HRESULT(bitPattern: 0x80004002)
}

@_transparent
public var E_ACCESSDENIED: WinSDK.HRESULT {
  HRESULT(bitPattern: 0x80070005)
}

@_transparent
public var RPC_E_WRONG_THREAD: WinSDK.HRESULT {
  HRESULT(bitPattern: 0x8001010E)
}

@_transparent
public var E_BOUNDS: WinSDK.HRESULT {
  HRESULT(bitPattern: 0x8000000B)
}

@_transparent
public var CLASS_E_CLASSNOTAVAILABLE: WinSDK.HRESULT {
  HRESULT(bitPattern: 0x80040111)
}

@_transparent
public var REGDB_E_CLASSNOTREG: WinSDK.HRESULT {
  HRESULT(bitPattern: 0x80040154)
}

@_transparent
public var E_CHANGED_STATE: WinSDK.HRESULT {
  HRESULT(bitPattern: 0x8000000C)
}

@_transparent
public var E_ILLEGAL_METHOD_CALL: WinSDK.HRESULT {
  HRESULT(bitPattern: 0x8000000E)
}

@_transparent
public var E_ILLEGAL_STATE_CHANGE: WinSDK.HRESULT {
  HRESULT(bitPattern: 0x8000000D)
}

@_transparent
public var E_ILLEGAL_DELEGATE_ASSIGNMENT: WinSDK.HRESULT {
  HRESULT(bitPattern: 0x80000018)
}

@_transparent
public var ERROR_CANCELLED: WinSDK.HRESULT {
  HRESULT(bitPattern: 0x800704C7)
}

@_transparent
public var E_OUTOFMEMORY: WinSDK.HRESULT {
  HRESULT(bitPattern: 0x8007000E)
}

@_transparent
public var CO_E_NOTINITIALIZED: WinSDK.HRESULT {
  HRESULT(bitPattern: 0x800401F0)
}

@_transparent
public var ERROR_FILE_NOT_FOUND: WinSDK.HRESULT {
  HRESULT(bitPattern: 0x80070002)
}

@_transparent
public var E_LAYOUTCYCLE: WinSDK.HRESULT {
  HRESULT(bitPattern: 0x802B0014)
}

@_transparent
public var E_XAMLPARSEFAILED : WinSDK.HRESULT {
  HRESULT(bitPattern: 0x802B000A)
}


private func getDescriptionFromErrorInfo() -> String? {
  var errorInfo: UnsafeMutablePointer<IErrorInfo>?
  let result = GetErrorInfo(0, &errorInfo)
  guard result == S_OK else { return nil }
  defer {
    _ = errorInfo?.pointee.lpVtbl.pointee.Release(errorInfo)
  }
  var errorDescription: BSTR?
  defer {
    SysFreeString(errorDescription)
  }
  _ = errorInfo?.pointee.lpVtbl.pointee.GetDescription(errorInfo, &errorDescription)
  guard SysStringLen(errorDescription) > 0 else { return nil }
  return String(decodingCString: errorDescription!, as: UTF16.self)
}

private func getDescriptionFromRestrictedErrorInfo() -> String? {
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
    errorInfo,
    &errorDescription,
    &result,
    &restrictedDescription,
    &capabilitySid)
  
  guard SysStringLen(restrictedDescription) > 0 else { return nil }
  return String(decodingCString: restrictedDescription!, as: UTF16.self)
}

private func getErrorDescription() -> String? {
  return getDescriptionFromRestrictedErrorInfo() ?? getDescriptionFromErrorInfo()
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

public struct Error : Swift.Error, CustomStringConvertible {
  public let description: String
  public let hr: HRESULT

  public init(hr: HRESULT) {
    self.description = getErrorDescription() ?? hrToString(hr)
    self.hr = hr
  }
}

public func failWith(err: HRESULT) -> HRESULT {
  return err
}
