// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import Ctest_component
import Foundation

open class IInspectable: IUnknown {
  override open class var IID: IID { IID_IInspectable }

  public func GetIids(_ iidCount: UnsafeMutablePointer<ULONG>, _ iids: UnsafeMutablePointer<UnsafeMutablePointer<IID>?>?) throws {
    return try perform(as: Ctest_component.IInspectable.self) { pThis in
      try CHECKED(pThis.pointee.lpVtbl.pointee.GetIids(pThis, iidCount, iids))
    }
  }

  public func GetRuntimeClassName(_ className: UnsafeMutablePointer<HSTRING?>) throws {
    return try perform(as: Ctest_component.IInspectable.self) { pThis in
      try CHECKED(pThis.pointee.lpVtbl.pointee.GetRuntimeClassName(pThis, className))
    }
  }

  public func GetTrustLevel(_ trustLevel: UnsafeMutablePointer<TrustLevel>?) throws {
    return try perform(as: Ctest_component.IInspectable.self) { pThis in
      try CHECKED(pThis.pointee.lpVtbl.pointee.GetTrustLevel(pThis, trustLevel))
    }
  }
}

// Provide facilities for creating a general IInspectable vtbl which can hold onto any UnsealedWinRTClass.
// These types are used for composable types which don't provide an overrides interface. Composable types which
// follow this pattern should define their composability contract like the following:
// internal class IBaseNoOverrides : OverridesImpl {
//      internal typealias CABI = Ctest_component.IInspectable
//      internal typealias SwiftABI = __ABI_test_component.IBaseNoOverrides
//      internal typealias SwiftProjection = BaseNoOverrides
//      internal typealias c_defaultABI = __x_ABI_Ctest__component_CIBaseNoOverrides
//      internal typealias swift_overrides = test_component.IInspectable
// }
// internal typealias Composable = IBaseNoOverrides
public enum __ABI {
    fileprivate typealias AnyObjectWrapper = WinRTWrapperBase<Ctest_component.IInspectable, AnyObject>
    fileprivate static var IInspectableVTable: Ctest_component.IInspectableVtbl = .init(
        QueryInterface: {
            guard let pUnk = $0, let riid = $1, let ppvObject = $2 else { return E_INVALIDARG }
            guard riid.pointee == IUnknown.IID ||
                  riid.pointee == IInspectable.IID || 
                  riid.pointee == ISwiftImplemented.IID ||
                  riid.pointee == IIAgileObject.IID else { 
                      guard let instance = AnyObjectWrapper.tryUnwrapFrom(raw: $0) as? any UnsealedWinRTClass,
                            let inner = instance._inner else { return E_NOINTERFACE }
                        
                    return inner.pointee.lpVtbl.pointee.QueryInterface(inner, riid, ppvObject)

            }
            _ = pUnk.pointee.lpVtbl.pointee.AddRef(pUnk)
            ppvObject.pointee = UnsafeMutableRawPointer(pUnk)
            return S_OK
        },

        AddRef: {
             guard let wrapper = AnyObjectWrapper.fromRaw($0) else { return 1 }
             _ = wrapper.retain()
             return ULONG(_getRetainCount(wrapper.takeUnretainedValue().swiftObj))
        },

        Release: {
            guard let wrapper = AnyObjectWrapper.fromRaw($0) else { return 1 }
            return ULONG(_getRetainCount(wrapper.takeRetainedValue()))
        },

        GetIids: {
            let size = MemoryLayout<IID>.size
            let iids = CoTaskMemAlloc(UInt64(size) * 2).assumingMemoryBound(to: IID.self)
            iids[0] = IUnknown.IID
            iids[1] = IInspectable.IID

            $1!.pointee = 2
            $2!.pointee = iids
            return S_OK
        },

        GetRuntimeClassName: {
            guard let instance = AnyObjectWrapper.tryUnwrapFrom(raw: $0) else { return E_INVALIDARG }
            guard let unsealed = instance as? any UnsealedWinRTClass else {
                let string = NSStringFromClass(type(of: instance))
                let hstring = try! HString(string).detach()
                $1!.pointee = hstring
                return S_OK   
            }
            let hstring = unsealed.GetRuntimeClassName().detach()
            $1!.pointee = hstring
            return S_OK
        },

        GetTrustLevel: {
            _ = $0
            $1!.pointee = TrustLevel(rawValue: 0)
            return S_OK
        }
    )
}

extension ComposableImpl where CABI == Ctest_component.IInspectable {
  public static func makeAbi() -> CABI {
    let vtblPtr = withUnsafeMutablePointer(to: &__ABI.IInspectableVTable) { $0 }
    return .init(lpVtbl: vtblPtr)
  }
}

extension IInspectable {
  public func unwrap<T>() -> T {
        // Try to unwrap an app implemented object. If one doesn't exist then we'll create the proper WinRT type below
        if let instance = __ABI.AnyObjectWrapper.tryUnwrapFrom(abi: RawPointer(self)) {
            return instance as! T
        }

        // We don't use the `Composable` type here because we have to get the actual implementation of this base 
        // class and then get *that types* composing creator. This allows us to be able to properly create a derived type.
        // Note that we'll *never* be trying to create an app implemented object at this point
        let className = try! GetSwiftClassName() 
        let baseType = NSClassFromString(className) as! any UnsealedWinRTClass.Type
        return baseType._makeFromAbi.from(abi: self.pUnk.borrow) as! T
  }
}