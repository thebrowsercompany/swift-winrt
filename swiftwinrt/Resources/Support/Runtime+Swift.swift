// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import WinSDK
import C_BINDINGS_MODULE

public func RoGetActivationFactory<Factory: IInspectable>(_ activatableClassId: HString) throws -> Factory {
  var iid = Factory.IID
  var factory: UnsafeMutableRawPointer?
  try CHECKED(RoGetActivationFactory(activatableClassId.get(), &iid, &factory))
  let inspectable = IInspectable(consuming: factory!.bindMemory(to: C_IUnknown.self, capacity: 1))
  return try inspectable.QueryInterface<Factory>()
}

public func RoActivateInstance(_ activatableClassId: HString) throws -> IUnknownRef {
  var instance: UnsafeMutablePointer<C_IInspectable>?
  try CHECKED(RoActivateInstance(activatableClassId.get(), &instance))
  return IUnknownRef(consuming: UnsafeMutableRawPointer(instance!).bindMemory(to: C_IUnknown.self, capacity: 1))
}

// ISwiftImplemented is a marker interface for code-gen types which are created by swift/winrt. It's used to QI
// an IUnknown VTABLE to see whether we can unwrap this type as a known swift object. The class is marked final
// because it isn't intended to actually be implemented.
private var IID_ISwiftImplemented: SUPPORT_MODULE.IID {
    .init(Data1: 0xbfd14ad5, Data2: 0x950b, Data3: 0x4b82, Data4: ( 0x96, 0xc, 0x1, 0xf4, 0xf4, 0x77, 0x7e, 0x57 )) // BFD14AD5-950B-4B82-960C-01F4F4777E57
}

public final class ISwiftImplemented : IInspectable {
    override public class var IID: SUPPORT_MODULE.IID { IID_ISwiftImplemented }
}
