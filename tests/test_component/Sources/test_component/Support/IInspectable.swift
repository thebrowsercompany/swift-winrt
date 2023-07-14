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
public enum __ABI_ {
    public class AnyWrapper : WinRTWrapperBase<Ctest_component.IInspectable, AnyObject> {
      public init?(_ swift: Any?) {
        guard let swift else { return nil }
        if let winrtObj = swift as? IWinRTObject {
          let abi: UnsafeMutablePointer<Ctest_component.IInspectable> = RawPointer(winrtObj.thisPtr)
          super.init(abi.pointee, winrtObj)
        } else if let winrtInterface = swift as? WinRTInterface {
          // Hold a reference to created ABI on the stack here so that it doesn't get released before we can use it.
          let abi = winrtInterface.makeAbi()
          let abiPtr: UnsafeMutablePointer<Ctest_component.IInspectable> = RawPointer(abi)
          super.init(abiPtr.pointee, winrtInterface)
        } else if let propertyValue = PropertyValue.createFrom(swift) {
          let abi: UnsafeMutablePointer<Ctest_component.IInspectable> = RawPointer(propertyValue)
          super.init(abi.pointee, propertyValue)
        } else if swift is WinRTEnum {
          fatalError("cant create enum")
        } else if swift is WinRTStruct {
          fatalError("can't create struct")
        } else {
          let vtblPtr = withUnsafeMutablePointer(to: &IInspectableVTable) { $0 }
          let cAbi = Ctest_component.IInspectable(lpVtbl: vtblPtr)
          super.init(cAbi, swift as AnyObject)
        }
      }

      override public func toABI<ResultType>(_ body: (UnsafeMutablePointer<Ctest_component.IInspectable>) throws -> ResultType)
        throws -> ResultType {
        if let winrtObj = swiftObj as? IWinRTObject {
            let abi: UnsafeMutablePointer<Ctest_component.IInspectable> = RawPointer(winrtObj.thisPtr)
            return try body(abi)
        } else if let swiftAbi = swiftObj as? IInspectable {
           let abi: UnsafeMutablePointer<Ctest_component.IInspectable> = RawPointer(swiftAbi)
           return try body(abi)
        } else {
            return try super.toABI(body)
        }
      }
      public static func unwrapFrom(abi: UnsafeMutablePointer<Ctest_component.IInspectable>?) -> Any? {
        guard let abi = abi else { return nil }
        if let instance = tryUnwrapFrom(abi: abi) {
          return instance
        }

        let insp: IInspectable = .init(abi)
        let className = try! insp.GetSwiftClassName()
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

    internal static var IInspectableVTable: Ctest_component.IInspectableVtbl = .init(
        QueryInterface: {
            guard let pUnk = $0, let riid = $1, let ppvObject = $2 else { return E_INVALIDARG }
            guard riid.pointee == IUnknown.IID ||
                  riid.pointee == IInspectable.IID || 
                  riid.pointee == ISwiftImplemented.IID ||
                  riid.pointee == IAgileObject.IID else { 
                      guard let instance = AnyWrapper.tryUnwrapFrom(raw: $0) as? any UnsealedWinRTClass,
                            let inner = instance._inner else { return E_NOINTERFACE }
                        
                    return inner.pointee.lpVtbl.pointee.QueryInterface(inner, riid, ppvObject)

            }
            _ = pUnk.pointee.lpVtbl.pointee.AddRef(pUnk)
            ppvObject.pointee = UnsafeMutableRawPointer(pUnk)
            return S_OK
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
            let size = MemoryLayout<IID>.size
            let iids = CoTaskMemAlloc(UInt64(size) * 2).assumingMemoryBound(to: IID.self)
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

extension ComposableImpl where CABI == Ctest_component.IInspectable {
  public static func makeAbi() -> CABI {
    let vtblPtr = withUnsafeMutablePointer(to: &__ABI_.IInspectableVTable) { $0 }
    return .init(lpVtbl: vtblPtr)
  }
}