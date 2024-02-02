// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import WinSDK
import C_BINDINGS_MODULE

public func RoGetActivationFactory<Factory: IInspectable>(_ activatableClassId: String) throws -> Factory {
  var iid = Factory.IID

  let hstring = try HString(activatableClassId)
  do {
    let (factory) = try ComPtrs.initialize(to: C_IInspectable.self) { factoryAbi in
      try CHECKED(RoGetActivationFactory(hstring.get(), &iid, &factoryAbi))
    }
    return try factory!.queryInterface()
  } catch let error as Error where error.hr == REGDB_E_CLASSNOTREG {
    // Class isn't registered, try searching for DLLs in the search path
    // and get the activation factory that way
    guard let factory = try TryGetActivationFactory(activatableClassId, hstring) else { throw Error(hr: REGDB_E_CLASSNOTREG) }
    return try factory.queryInterface()
  }
}

private typealias PFN_DllGetActivationFactory = @convention(c) (HSTRING?, UnsafeMutablePointer<UnsafeMutablePointer<C_IActivationFactory>?>?) -> HRESULT

private func TryGetActivationFactoryModule(_ activatableClassId: String) -> HMODULE? {
  // Remove the last component of the activatableClassId to get the namespace
  // and try to load the library from that namespace. If that doesn't work, then try
  // again with the namespace.
  guard let lastIndex = activatableClassId.lastIndex(of: ".") else { return nil }
  let namespace = activatableClassId[..<lastIndex]
  let module = LoadLibraryA("\(namespace).dll")
  guard let module else { return TryGetActivationFactoryModule(String(namespace)) }
  return module
}

private func TryGetActivationFactory(_ activatableClassId: String, _ activatableClassHString: HString) throws -> ComPtr<C_IActivationFactory>? {
  guard let module = TryGetActivationFactoryModule(activatableClassId) else { throw Error(hr: REGDB_E_CLASSNOTREG) }
  guard let pfn = GetProcAddress(module, "DllGetActivationFactory") else { throw Error(hr: REGDB_E_CLASSNOTREG) }
  let pfnFactory: PFN_DllGetActivationFactory = unsafeBitCast(pfn, to: PFN_DllGetActivationFactory.self)
  return try ComPtrs.initialize { factoryAbi in
    try CHECKED(pfnFactory(activatableClassHString.get(), &factoryAbi))
  }
}

public func RoActivateInstance<Instance: IInspectable>(_ activatableClassId: String) throws -> Instance {
  let hstring = try HString(activatableClassId)
  let (instance) = try ComPtrs.initialize { instanceAbi in
    do {
      try CHECKED(RoActivateInstance(hstring.get(), &instanceAbi))
    } catch let error as Error where error.hr == REGDB_E_CLASSNOTREG {
      guard let factory = try TryGetActivationFactory(activatableClassId, hstring) else { throw Error(hr: REGDB_E_CLASSNOTREG) }
      try CHECKED(factory.get().pointee.lpVtbl.pointee.ActivateInstance(factory.get(), &instanceAbi))
    }
  }

  return try instance!.queryInterface()
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
