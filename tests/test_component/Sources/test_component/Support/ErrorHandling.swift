// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import WinSDK

@_alwaysEmitIntoClient @inline(__always) @discardableResult
public func CHECKED(_ body: () -> HRESULT) throws -> HRESULT {
  let hr: HRESULT = body()
  guard hr >= 0 else { throw Error(hr: hr) }
  return hr
}

@_alwaysEmitIntoClient @inline(__always) @discardableResult
public func CHECKED(_ body: @autoclosure () -> HRESULT) throws -> HRESULT {
  return try CHECKED(body)
}
