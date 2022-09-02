// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import Ctest_component

public func RoGetActivationFactory<Factory: test_component.IInspectable>(_ activatableClassId: HString) throws -> Factory {
  var iid: IID = Factory.IID
  var factory: UnsafeMutableRawPointer?
  try CHECKED(RoGetActivationFactory(activatableClassId.hRef.hString, &iid, &factory))
  return Factory(consuming: factory?.bindMemory(to: WinSDK.IUnknown.self, capacity: 1))
}

public func RoActivateInstance<Instance: test_component.IInspectable>(_ activatableClassId: HString) throws -> Instance {
  var instance: UnsafeMutablePointer<Ctest_component.IInspectable>?
  try CHECKED(RoActivateInstance(activatableClassId.hRef.hString, &instance))
  return Instance(consuming: UnsafeMutableRawPointer(instance)?.bindMemory(to: WinSDK.IUnknown.self, capacity: 1))
}

// ISwiftImplemented is a marker interface for code-gen types which are created by swift/winrt. It's used to QI
// an IUnknown VTABLE to see whether we can unwrap this type as a known swift object. The class is marked final
// because it isn't intended to actually be implemented.
private var IID_ISwiftImplemented: IID {
    IID(Data1: 0xbfd14ad5, Data2: 0x950b, Data3: 0x4b82, Data4: ( 0x96, 0xc, 0x1, 0xf4, 0xf4, 0x77, 0x7e, 0x57 )) // BFD14AD5-950B-4B82-960C-01F4F4777E57
}

final class ISwiftImplemented : test_component.IInspectable {
    override public class var IID: IID { IID_ISwiftImplemented }
}
