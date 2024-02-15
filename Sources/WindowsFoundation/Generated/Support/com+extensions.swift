// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import WinSDK
import CWinRT

public func ==<T: Equatable>(_ lhs: (T, T, T, T, T, T, T, T),
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

extension IInspectable : Equatable {
  // Test for COM-style equality.
  public static func ==(_ lhs: IInspectable, _ rhs: IInspectable) -> Bool {
    let lhsUnknown: IUnknown = try! lhs.QueryInterface()
    let rhsUnknown: IUnknown = try! rhs.QueryInterface()
    let equals = (lhsUnknown.pUnk.borrow == rhsUnknown.pUnk.borrow)

    return equals
  }
}

private var IID_IAgileObject: WindowsFoundation.IID {
    .init(Data1: 0x94ea2b94, Data2: 0xe9cc, Data3: 0x49e0, Data4: ( 0xc0, 0xff, 0xee, 0x64, 0xca, 0x8f, 0x5b, 0x90 )) // 94ea2b94-e9cc-49e0-c0ff-ee64ca8f5b90
}

public final class IAgileObject : IUnknown {
    override public class var IID: WindowsFoundation.IID { IID_IAgileObject }
}
