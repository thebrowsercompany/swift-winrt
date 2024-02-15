// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import WinSDK
import CWinRT

public func RoGetActivationFactory<Factory: IInspectable>(_ activatableClassId: HString) throws -> Factory {
  var iid = Factory.IID
  let (factory) = try ComPtrs.initialize(to: C_IInspectable.self) { factoryAbi in
    try CHECKED(RoGetActivationFactory(activatableClassId.get(), &iid, &factoryAbi))
  }
  return try factory!.queryInterface()
}

public func RoActivateInstance<Instance: IInspectable>(_ activatableClassId: HString) throws -> Instance {
  let (instance) = try ComPtrs.initialize { instanceAbi in
    try CHECKED(RoActivateInstance(activatableClassId.get(), &instanceAbi))
  }
  return try instance!.queryInterface()
}

// ISwiftImplemented is a marker interface for code-gen types which are created by swift/winrt. It's used to QI
// an IUnknown VTABLE to see whether we can unwrap this type as a known swift object. The class is marked final
// because it isn't intended to actually be implemented.
private var IID_ISwiftImplemented: WindowsFoundation.IID {
    .init(Data1: 0xbfd14ad5, Data2: 0x950b, Data3: 0x4b82, Data4: ( 0x96, 0xc, 0x1, 0xf4, 0xf4, 0x77, 0x7e, 0x57 )) // BFD14AD5-950B-4B82-960C-01F4F4777E57
}

public final class ISwiftImplemented : IInspectable {
    override public class var IID: WindowsFoundation.IID { IID_ISwiftImplemented }
}
