// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import WinSDK

extension IUnknown {
  public static func from(_ pointer: UnsafeMutableRawPointer?) -> Self? {
    guard let pointer = pointer else { return nil }
    return Self(pointer)
  }

  public func QueryInterface<Interface: IUnknown>() throws -> Interface {
    var iid: IID = Interface.IID
    var pointer: UnsafeMutableRawPointer?
    try CHECKED(self.pUnk.borrow.pointee.lpVtbl.pointee.QueryInterface(self.pUnk.borrow, &iid, &pointer))
    // https://learn.microsoft.com/en-us/windows/win32/api/unknwn/nf-unknwn-iunknown-queryinterface(refiid_void)
    // "Upon successful return, *ppvObject (the dereferenced address) contains a pointer to the requested interface"
    return Interface(consuming: pointer!.bindMemory(to: NativeIUnknown.self, capacity: 1))
  }
}

extension IUnknown {
  @_alwaysEmitIntoClient @inline(__always)
  public func perform<Type, ResultType>(as type: Type.Type,
                                        _ body: (UnsafeMutablePointer<Type>) throws -> ResultType)
      throws -> ResultType {
    let pThis = UnsafeMutableRawPointer(self.pUnk.borrow).bindMemory(to: Type.self, capacity: 1)
    return try body(pThis)
  }
}
