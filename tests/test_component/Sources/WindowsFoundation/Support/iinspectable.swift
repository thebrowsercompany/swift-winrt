// Copyright © 2021 Saleem Abdulrasool <compnerd@compnerd.org>
// SPDX-License-Identifier: BSD-3

import CWinRT
import Foundation

fileprivate let IID_IInspectable = IID(Data1: 0xAF86E2E0, Data2: 0xB12D, Data3: 0x4C6A, Data4: ( 0x9C, 0x5A, 0xD7, 0xAA, 0x65, 0x10, 0x1E, 0x90 )) // AF86E2E0-B12D-4c6a-9C5A-D7AA65101E90

open class IInspectable: IUnknown {
  override open class var IID: WindowsFoundation.IID { IID_IInspectable }

  public func GetIids(_ iidCount: UnsafeMutablePointer<ULONG>, _ iids: UnsafeMutablePointer<UnsafeMutablePointer<WindowsFoundation.IID>?>?) throws {
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
//      internal typealias SwiftABI = __ABI_WindowsFoundation.IBaseNoOverrides
//      internal typealias SwiftProjection = BaseNoOverrides
//      internal typealias c_defaultABI = __x_ABI_Ctest__component_CIBaseNoOverrides
//      internal typealias swift_overrides = WindowsFoundation.IInspectable
// }
// internal typealias Composable = IBaseNoOverrides
protocol AnyObjectWrapper {
    var obj: AnyObject? { get }
}

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
      public static func unwrapFrom(abi: ComPtr<C_IInspectable>?) -> Any? {
        guard let abi = abi else { return nil }
        if let instance = tryUnwrapFrom(abi: abi) {
          if let weakRef = instance as? AnyObjectWrapper { return weakRef.obj }
          return instance
        }

        let ref = IInspectable(abi)
        return makeFrom(abi: ref) ?? ref
      }
      public static func tryUnwrapFrom(raw pUnk: UnsafeMutableRawPointer?) -> AnyObject? {
        guard let pUnk else { return nil }
        return tryUnwrapFromBase(raw: pUnk)
      }

      internal static func queryInterface(_ pUnk: UnsafeMutablePointer<C_IInspectable>?, _ riid: UnsafePointer<WindowsFoundation.IID>?, _ ppvObject: UnsafeMutablePointer<UnsafeMutableRawPointer?>?) -> HRESULT {
        guard let pUnk, let riid, let ppvObject else { return E_INVALIDARG }
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
          ppvObject.pointee = result.detach()
          return S_OK
        }
        return E_NOINTERFACE
      }
    }

    internal static var IInspectableVTable: C_IInspectableVtbl = .init(
        QueryInterface: { AnyWrapper.queryInterface($0, $1, $2) },
        AddRef: { AnyWrapper.addRef($0) },
        Release: { AnyWrapper.release($0) },
        GetIids: {
            let size = MemoryLayout<WindowsFoundation.IID>.size
            let iids = CoTaskMemAlloc(UInt64(size) * 2).assumingMemoryBound(to: WindowsFoundation.IID.self)
            iids[0] = IUnknown.IID
            iids[1] = IInspectable.IID

            $1!.pointee = 2
            $2!.pointee = iids
            return S_OK
        },

        GetRuntimeClassName: {
            guard let instance = AnyWrapper.tryUnwrapFrom(raw: $0) else { return E_INVALIDARG }
            guard let winrtClass = instance as? WinRTClass else {
              let string = String(reflecting: type(of: instance))
              $1!.pointee = try! HString(string).detach()
              return S_OK
            }
            $1!.pointee = winrtClass.GetRuntimeClassName().detach()
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

@_spi(WinRTInternal)
public enum __IMPL_ {
    public enum AnyBridge: AbiInterfaceBridge {
        public static func makeAbi() -> CABI {
            let vtblPtr = withUnsafeMutablePointer(to: &__ABI_.IInspectableVTable) { $0 }
            return .init(lpVtbl: vtblPtr)
        }

        public static func from(abi: ComPtr<CABI>?) -> SwiftProjection? {
            guard let abi else { return nil }
            let ref = IInspectable(abi)
            return makeFrom(abi: ref) ?? ref
        }

        public typealias SwiftProjection = Any
        public typealias CABI = C_IInspectable
        public typealias SwiftABI = WindowsFoundation.IInspectable
    }
}