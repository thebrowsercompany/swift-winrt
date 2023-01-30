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

extension _GUID: Equatable {
   public static func ==(_ lhs: _GUID, _ rhs: _GUID) -> Bool {
      return lhs.Data1 == rhs.Data1 &&
         lhs.Data2 == rhs.Data2 &&
         lhs.Data3 == rhs.Data3 &&
         lhs.Data4 == rhs.Data4
   }
}

extension UUID {
  /// Create a UUID from a string such as "E621E1F8-C36C-495A-93FC-0C247A3E6E5F"
  ///
  /// returns nil for invalid strings
  public init?(uuidString string: String){
    var _self : _GUID = .init()

    // use UuidFromString because it expects the correct format. 
    // See https://devblogs.microsoft.com/oldnewthing/20151015-00/?p=91351
    let result = UuidFromStringA(makeCString(from: string), &_self)
    if result != S_OK { return nil }
    self = _self
  }
}

extension IInspectable : Equatable {
  // Test for COM-style equality.
  public static func ==(_ lhs: IInspectable, _ rhs: IInspectable) -> Bool {
    var iid: IID = IID_IUnknown
    var lhsUnknown: UnsafeMutableRawPointer?
    try! lhs.QueryInterface(&iid, &lhsUnknown)
    var rhsUnknown: UnsafeMutableRawPointer?
    try! rhs.QueryInterface(&iid, &rhsUnknown)
    let equals = (lhsUnknown == rhsUnknown)

    _ = IUnknown(lhsUnknown).Release()
    _ = IUnknown(rhsUnknown).Release()

    return equals
  }
}

private var IID_IAgileObject: IID {
    IID(Data1: 0x94ea2b94, Data2: 0xe9cc, Data3: 0x49e0, Data4: ( 0xc0, 0xff, 0xee, 0x64, 0xca, 0x8f, 0x5b, 0x90 )) // 94ea2b94-e9cc-49e0-c0ff-ee64ca8f5b90
}

final class IIAgileObject : test_component.IUnknown {
    override public class var IID: IID { IID_IAgileObject }
}
