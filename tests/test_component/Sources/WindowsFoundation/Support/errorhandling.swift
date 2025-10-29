// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import WinSDK

@_alwaysEmitIntoClient @inline(__always)
public func CHECKED(_ body: () -> HRESULT) throws {
  let hr: HRESULT = body()
  guard hr >= 0 else { throw Error(hr: hr) }
}

@_alwaysEmitIntoClient @inline(__always)
public func CHECKED(_ body: @autoclosure () -> HRESULT) throws {
  try CHECKED(body)
}
