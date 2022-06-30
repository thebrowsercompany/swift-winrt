// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import WinSDK

private func ==<T: Equatable>(_ lhs: (T, T, T, T, T, T, T, T),
                              _ rhs: (T, T, T, T, T, T, T, T)) -> Bool {
  return lhs.0 == rhs.0 &&
         lhs.1 == rhs.1 &&
         lhs.2 == rhs.2 &&
         lhs.3 == rhs.3 &&
         lhs.4 == rhs.4 &&
         lhs.5 == rhs.5 &&
         lhs.6 == rhs.6 &&
         lhs.7 == rhs.7
}

internal func ==(_ lhs: _GUID, _ rhs: _GUID) -> Bool {
  return lhs.Data1 == rhs.Data1 &&
         lhs.Data2 == rhs.Data2 &&
         lhs.Data3 == rhs.Data3 &&
         lhs.Data4 == rhs.Data4
}

internal func ~=(_ lhs: _GUID, _ rhs: _GUID) -> Bool {
  return lhs.Data1 == rhs.Data1 &&
         lhs.Data2 == rhs.Data2 &&
         lhs.Data3 == rhs.Data3 &&
         lhs.Data4 == rhs.Data4
}

extension _GUID: CustomStringConvertible {
  public var description: String {
    withUnsafePointer(to: self) { pGUID in
      Array<WCHAR>(unsafeUninitializedCapacity: 40) {
        $1 = Int(StringFromGUID2(pGUID, $0.baseAddress, CInt($0.count)))
      }.withUnsafeBufferPointer {
        String(decodingCString: $0.baseAddress!, as: UTF16.self)
      }
    }
  }
}
