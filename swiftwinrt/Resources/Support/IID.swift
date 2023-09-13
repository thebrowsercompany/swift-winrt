import WinSDK
import C_BINDINGS_MODULE

public typealias IID = C_BINDINGS_MODULE.IID
public typealias REFIID = UnsafePointer<C_BINDINGS_MODULE.IID>
public typealias NativeIUnknown = C_BINDINGS_MODULE.IUnknown
public typealias NativeIInspectable = C_BINDINGS_MODULE.IInspectable
public typealias NativeIInspectableVtbl = C_BINDINGS_MODULE.IInspectableVtbl
public let IID_IUnknown = C_BINDINGS_MODULE.IID_IUnknown
public let IID_IInspectable = C_BINDINGS_MODULE.IID_IInspectable

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