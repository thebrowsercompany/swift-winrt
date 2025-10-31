// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import WinSDK
import CWinRT

private var IID_IActivationFactory: WindowsFoundation.IID {
    .init(Data1: 0x00000035, Data2: 0x0000, Data3: 0x0000, Data4: ( 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 )) // 00000035-0000-0000-C000-000000000046
}

public class IActivationFactory: WindowsFoundation.IInspectable {
   override public class var IID: WindowsFoundation.IID { IID_IActivationFactory }

  public func ActivateInstance() throws -> WindowsFoundation.IInspectable {
    return try perform(as: C_IActivationFactory.self) { pThis in
      let instance = try ComPtrs.initialize { instanceAbi in
        try CHECKED(pThis.pointee.lpVtbl.pointee.ActivateInstance(pThis, &instanceAbi))
      }
      return .init(instance!)
    }
  }
}

public func RoGetActivationFactory<Factory: IInspectable>(_ activatableClassId: StaticString) throws -> Factory {
  var iid = Factory.IID
  let (factory) = try ComPtrs.initialize(to: C_IInspectable.self) { factoryAbi in
    try activatableClassId.withHStringRef { activatableClassIdHStr in
      try CHECKED(RoGetActivationFactory(activatableClassIdHStr, &iid, &factoryAbi))
    }
  }
  return try factory!.queryInterface()
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
