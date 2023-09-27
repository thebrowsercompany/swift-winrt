// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import Ctest_component
import Foundation

fileprivate let IID_IInspectable = IID(Data1: 0xAF86E2E0, Data2: 0xB12D, Data3: 0x4C6A, Data4: ( 0x9C, 0x5A, 0xD7, 0xAA, 0x65, 0x10, 0x1E, 0x90 )) // AF86E2E0-B12D-4c6a-9C5A-D7AA65101E90

open class IInspectable: IUnknown {
  override open class var IID: test_component.IID { IID_IInspectable }

  public func GetIids(_ iidCount: UnsafeMutablePointer<ULONG>, _ iids: UnsafeMutablePointer<UnsafeMutablePointer<test_component.IID>?>?) throws {
    return try perform(as: C_IInspectable.self) { pThis in
      try CHECKED(pThis.pointee.lpVtbl.pointee.GetIids(pThis, iidCount, iids))
    }
  }

  public func GetRuntimeClassName(_ className: UnsafeMutablePointer<HSTRING?>) throws {
    return try perform(as: C_IInspectable.self) { pThis in
      try CHECKED(pThis.pointee.lpVtbl.pointee.GetRuntimeClassName(pThis, className))
    }
  }

  public func GetTrustLevel(_ trustLevel: UnsafeMutablePointer<TrustLevel>?) throws {
    return try perform(as: C_IInspectable.self) { pThis in
      try CHECKED(pThis.pointee.lpVtbl.pointee.GetTrustLevel(pThis, trustLevel))
    }
  }
}

// Provide facilities for creating a general IInspectable vtbl which can hold onto any UnsealedWinRTClass.
// These types are used for composable types which don't provide an overrides interface. Composable types which
// follow this pattern should define their composability contract like the following:
// internal class IBaseNoOverrides : OverridesImpl {
//      internal typealias CABI = C_IInspectable
//      internal typealias SwiftABI = __ABI_test_component.IBaseNoOverrides
//      internal typealias SwiftProjection = BaseNoOverrides
//      internal typealias c_defaultABI = __x_ABI_Ctest__component_CIBaseNoOverrides
//      internal typealias swift_overrides = test_component.IInspectable
// }
// internal typealias Composable = IBaseNoOverrides
public enum __ABI_ {
    public class AnyWrapper : WinRTWrapperBase<C_IInspectable, AnyObject> {
      public init?(_ swift: Any?) {
        guard let swift else { return nil }
         if let propertyValue = PropertyValue.createFrom(swift) {
          let abi: UnsafeMutablePointer<C_IInspectable> = RawPointer(propertyValue)
          super.init(abi.pointee, propertyValue)
        } else {
          let vtblPtr = withUnsafeMutablePointer(to: &IInspectableVTable) { $0 }
          let cAbi = C_IInspectable(lpVtbl: vtblPtr)
          super.init(cAbi, swift as AnyObject)
        }
      }

      override public func toABI<ResultType>(_ body: (UnsafeMutablePointer<C_IInspectable>) throws -> ResultType)
        throws -> ResultType {
        if let swiftAbi = swiftObj as? IInspectable {
           let abi: UnsafeMutablePointer<C_IInspectable> = RawPointer(swiftAbi)
           return try body(abi)
        } else {
            return try super.toABI(body)
        }
      }
      public static func unwrapFrom(abi: UnsafeMutablePointer<C_IInspectable>?) -> Any? {
        guard let abi = abi else { return nil }
        if let instance = tryUnwrapFrom(abi: abi) {
          return instance
        }

        let insp: IInspectable = .init(abi)
        let className = try? insp.GetSwiftClassName()
        guard let className else { return insp }

        if let baseType = NSClassFromString(className) as? any UnsealedWinRTClass.Type {
          return baseType._makeFromAbi.from(abi: insp.pUnk.borrow)
        }

        let abiMaker = "\(className)_MakeFromAbi"
        if let implType = NSClassFromString(abiMaker) as? any MakeFromAbi.Type {
          return implType.from(abi: insp.pUnk.borrow)
        }

        print("unable to make \(className) from abi, defaulting to IInspectable")
        return insp
      }
    }

    internal static var IInspectableVTable: C_IInspectableVtbl = .init(
        QueryInterface: {
            guard let pUnk = $0, let riid = $1, let ppvObject = $2 else { return E_INVALIDARG }
            ppvObject.pointee = nil
            if riid.pointee == IUnknown.IID ||
                  riid.pointee == IInspectable.IID ||
                  riid.pointee == ISwiftImplemented.IID ||
                  riid.pointee == IAgileObject.IID {
              _ = pUnk.pointee.lpVtbl.pointee.AddRef(pUnk)
              ppvObject.pointee = UnsafeMutableRawPointer(pUnk)
              return S_OK
            }
            let swiftObj = AnyWrapper.tryUnwrapFrom(raw: pUnk)
            if let customQueryInterface = swiftObj as? CustomQueryInterface,
               let result = customQueryInterface.queryInterface(riid.pointee) {
                ppvObject.pointee = UnsafeMutableRawPointer(result.ref)
                return S_OK
            }
            return E_NOINTERFACE
        },

        AddRef: {
             guard let wrapper = AnyWrapper.fromRaw($0) else { return 1 }
             _ = wrapper.retain()
             return ULONG(_getRetainCount(wrapper.takeUnretainedValue().swiftObj))
        },

        Release: {
            guard let wrapper = AnyWrapper.fromRaw($0) else { return 1 }
            return ULONG(_getRetainCount(wrapper.takeRetainedValue()))
        },

        GetIids: {
            let size = MemoryLayout<test_component.IID>.size
            let iids = CoTaskMemAlloc(UInt64(size) * 2).assumingMemoryBound(to: test_component.IID.self)
            iids[0] = IUnknown.IID
            iids[1] = IInspectable.IID

            $1!.pointee = 2
            $2!.pointee = iids
            return S_OK
        },

        GetRuntimeClassName: {
            guard let instance = AnyWrapper.tryUnwrapFrom(raw: $0) else { return E_INVALIDARG }
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

extension ComposableImpl where CABI == C_IInspectable {
  public static func makeAbi() -> CABI {
    let vtblPtr = withUnsafeMutablePointer(to: &__ABI_.IInspectableVTable) { $0 }
    return .init(lpVtbl: vtblPtr)
  }
}