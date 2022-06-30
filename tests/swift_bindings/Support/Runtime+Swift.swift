// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import TestComponent_CWinRT

public func RoGetActivationFactory<Factory: IInspectable>(_ activatableClassId: HString) throws -> Factory {
  var iid: IID = Factory.IID
  var factory: UnsafeMutableRawPointer?
  try CHECKED(RoGetActivationFactory(activatableClassId.hRef.hString, &iid, &factory))
  return Factory(consuming: factory?.bindMemory(to: WinSDK.IUnknown.self, capacity: 1))
}

public func RoActivateInstance<Instance: IInspectable>(_ activatableClassId: HString) throws -> Instance {
  var instance: UnsafeMutablePointer<TestComponent_CWinRT.IInspectable>?
  try CHECKED(RoActivateInstance(activatableClassId.hRef.hString, &instance))
  return Instance(consuming: UnsafeMutableRawPointer(instance)?.bindMemory(to: WinSDK.IUnknown.self, capacity: 1))
}
