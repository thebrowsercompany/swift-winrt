// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import WinSDK

extension IUnknown {
  public func QueryInterface<Interface: IUnknown>() throws -> Interface {
    guard let pUnk = self.pUnk.borrow else { throw Error(hr: E_INVALIDARG) }

    var iid: IID = Interface.IID
    var pointer: UnsafeMutableRawPointer?
    try CHECKED(pUnk.pointee.lpVtbl.pointee.QueryInterface(pUnk, &iid, &pointer))
    return Interface(consuming: pointer?.bindMemory(to: WinSDK.IUnknown.self, capacity: 1))
  }
}

extension IUnknown {
  public static func CreateInstance<Interface: IUnknown>(`class` clsid: CLSID,
                                                         outer pUnkOuter: IUnknown? = nil,
                                                         context dwClsContext: CLSCTX = CLSCTX_INPROC_SERVER)
      throws -> Interface {
    var clsid: CLSID = clsid
    var iid: IID = Interface.IID

    var pointer: UnsafeMutableRawPointer?
    try CHECKED(CoCreateInstance(&clsid, RawPointer(pUnkOuter), DWORD(dwClsContext.rawValue), &iid, &pointer))
    return Interface(consuming: pointer?.bindMemory(to: WinSDK.IUnknown.self, capacity: 1))
  }
}

extension IUnknown {
  @_spi(IUnknown)
  @_alwaysEmitIntoClient @inline(__always)
  public func perform<Type, ResultType>(as type: Type.Type,
                                        _ body: (UnsafeMutablePointer<Type>) throws -> ResultType)
      throws -> ResultType {
    guard let pUnk = UnsafeMutableRawPointer(self.pUnk.borrow) else {
      throw Error(hr: E_INVALIDARG)
    }
    let pThis = pUnk.bindMemory(to: Type.self, capacity: 1)

    return try body(pThis)
  }
}
