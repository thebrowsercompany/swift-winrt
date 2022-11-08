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
//      internal typealias c_ABI = Ctest_component.IInspectable
//      internal typealias swift_ABI = __ABI_test_component.IBaseNoOverrides
//      internal typealias swift_Projection = BaseNoOverrides
//      internal typealias c_defaultABI = __x_ABI_Ctest__component_CIBaseNoOverrides
//      internal typealias swift_overrides = test_component.IInspectable
// }
// internal typealias Composable = IBaseNoOverrides
enum __ABI {
    private typealias AnyObjectWrapper = WinRTWrapperBase<Ctest_component.IInspectable, AnyObject>
    fileprivate static var IInspectableVTable: Ctest_component.IInspectableVtbl = .init(
        QueryInterface: {
            guard let pUnk = $0, let riid = $1, let ppvObject = $2 else { return E_INVALIDARG }
            guard riid.pointee == IUnknown.IID ||
                  riid.pointee == IInspectable.IID || 
                  riid.pointee == ISwiftImplemented.IID ||
                  riid.pointee == IIAgileObject.IID else { 
                    guard let instance = AnyObjectWrapper.try_unwrap_from(raw: $0) as? any UnsealedWinRTClass,
                            let inner = instance._inner else { return E_NOINTERFACE }
                        
                    return inner.pointee.lpVtbl.pointee.QueryInterface(inner, riid, ppvObject)

            }
            _ = pUnk.pointee.lpVtbl.pointee.AddRef(pUnk)
            ppvObject.pointee = UnsafeMutableRawPointer(pUnk)
            return S_OK
        },

        AddRef: {
             guard let wrapper = AnyObjectWrapper.from_raw($0) else { return 1 }
             _ = wrapper.retain()
             return ULONG(_getRetainCount(wrapper.takeUnretainedValue().swiftObj))
        },

        Release: {
            guard let wrapper = AnyObjectWrapper.from_raw($0) else { return 1 }
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
            guard let instance = AnyObjectWrapper.try_unwrap_from(raw: $0) else { return E_INVALIDARG }
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

extension Ctest_component.IInspectable : Initializable {
    public init() {
        let vtblPtr = withUnsafeMutablePointer(to: &__ABI.IInspectableVTable) { $0 }
        self.init(lpVtbl: vtblPtr)
    }
}