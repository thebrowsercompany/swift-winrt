// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import WinSDK

extension IUnknown {
  public func QueryInterface<Interface: IUnknown>() throws -> Interface {
    var iid = Interface.IID
    let (pointer) = try ComPtrs.initialize(to: C_IUnknown.self) { abi in
      try CHECKED(self.pUnk.borrow.pointee.lpVtbl.pointee.QueryInterface(self.pUnk.borrow, &iid, &abi))
    }
    return Interface(pointer!)
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

extension IUnknown {
  public static func CreateInstance<Interface: IUnknown>(`class` clsid: CLSID,
                                                         outer pUnkOuter: IUnknown? = nil,
                                                         context dwClsContext: CLSCTX = CLSCTX_INPROC_SERVER)
      throws -> Interface {
    var clsid = clsid
    var iid = Interface.IID

    let (instance) = try ComPtrs.initialize(to: C_IUnknown.self) { instanceAbi in
        try CHECKED(CoCreateInstance(&clsid, RawPointer(pUnkOuter), DWORD(dwClsContext.rawValue), &iid, &instanceAbi))
    }
    // https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-cocreateinstance
    // "Upon successful return, *ppv contains the requested interface pointer."
    return try instance!.queryInterface()
  }
}