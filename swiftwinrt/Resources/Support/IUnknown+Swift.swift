// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import WinSDK

extension IUnknown {
  public func QueryInterface<Interface: IUnknown>() throws -> Interface {
    var iid = Interface.IID
    let (pointer) = try ComPtrs.initialize(to: C_IUnknown.self) { abi in
      try CHECKED(self.pUnk.borrow.pointee.lpVtbl.pointee.QueryInterface(self.pUnk.borrow, &iid, &abi))
    }
    return Interface(pointer)
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
