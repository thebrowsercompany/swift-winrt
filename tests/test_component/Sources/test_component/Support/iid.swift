import WinSDK
import Ctest_component

public typealias IID = Ctest_component.IID
public typealias REFIID = UnsafePointer<Ctest_component.IID>
public typealias NativeIUnknown = Ctest_component.IUnknown
public typealias NativeIInspectable = Ctest_component.IInspectable
public typealias NativeIInspectableVtbl = Ctest_component.IInspectableVtbl
public let IID_IUnknown = Ctest_component.IID_IUnknown
public let IID_IInspectable = Ctest_component.IID_IInspectable

extension IID: Equatable {
   public static func ==(_ lhs: IID, _ rhs: IID) -> Bool {
      return lhs.Data1 == rhs.Data1 &&
         lhs.Data2 == rhs.Data2 &&
         lhs.Data3 == rhs.Data3 &&
         lhs.Data4 == rhs.Data4
   }
}

public func ~=(_ lhs: IID, _ rhs: IID) -> Bool {
  return lhs.Data1 == rhs.Data1 &&
         lhs.Data2 == rhs.Data2 &&
         lhs.Data3 == rhs.Data3 &&
         lhs.Data4 == rhs.Data4
}