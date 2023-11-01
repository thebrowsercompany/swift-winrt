import C_BINDINGS_MODULE
import WinSDK

public protocol Initializable {
  init()
}

public protocol InitializableFromAbi : HasIID {
    associatedtype ABI
    init?(ref: UnsafeMutablePointer<ABI>?)
}

public protocol HasIID {
  static var IID: SUPPORT_MODULE.IID { get }
}

public protocol AbiInterface {
    associatedtype CABI
    associatedtype SwiftABI : SUPPORT_MODULE.IUnknown
}

// A protocol for defining a type which implements a WinRT interface and defines
// the swift <-> winrt translation
public protocol AbiBridge {
    associatedtype CABI
    associatedtype SwiftProjection
    static func makeAbi() -> CABI
    static func from(abi: UnsafeMutablePointer<CABI>?) -> SwiftProjection?
}

public protocol ReferenceBridge : AbiBridge, HasIID {
}

public protocol AbiInterfaceBridge : AbiBridge & AbiInterface {
}

public protocol AbiInterfaceImpl<Bridge> {
    associatedtype Bridge: AbiInterfaceBridge
    var _default: Bridge.SwiftABI { get }
}
internal typealias AnyAbiInterfaceImpl<Bridge> = any AbiInterfaceImpl<Bridge>
public protocol WinRTAbiImpl<Bridge>: AbiInterfaceImpl where Bridge.SwiftABI: IInspectable {}
internal typealias AnyWinRTAbiImpl<Bridge> = any WinRTAbiImpl<Bridge>

// The WinRTWrapperBase class wraps an AbiBridge and is used for wrapping and unwrapping swift
// objects at the ABI layer. The contract for how to do this is defined by the AbiBridge protocol
open class WinRTWrapperBase<CInterface, Prototype> {
    public struct ComObject {
        public var comInterface: CInterface
        public var wrapper: Unmanaged<WinRTWrapperBase>?
    }

    public var instance: ComObject
    public var swiftObj: Prototype!

    open class var IID: SUPPORT_MODULE.IID { get { fatalError("not implemented") } }

    public init(_ pointer: CInterface, _ impl: Prototype!) {
        self.instance = ComObject(comInterface: pointer)
        self.swiftObj = impl
        self.instance.wrapper = Unmanaged<WinRTWrapperBase>.passUnretained(self)
    }

    @_alwaysEmitIntoClient @inline(__always)
    public func toABI<ResultType>(_ body: (UnsafeMutablePointer<CInterface>) throws -> ResultType)
        throws -> ResultType {

        try withUnsafeMutablePointer(to:&instance.comInterface){
          return try body($0)
        }
    }

    @_alwaysEmitIntoClient @inline(__always)
    public func copyTo(_ ptr: UnsafeMutablePointer<UnsafeMutablePointer<CInterface>?>?) {
        guard let ptr else { return }
        // Use toABI as derived classes may override this to get the ABI pointer of the swift
        // object they are holding onto
        let abi: UnsafeMutablePointer<CInterface> = try! toABI { $0 }
        abi.withMemoryRebound(to: C_IUnknown.self, capacity: 1) {
            _ = $0.pointee.lpVtbl.pointee.AddRef($0)
        }

        ptr.initialize(to: abi)
    }

    public func queryInterface(_ iid: SUPPORT_MODULE.IID) -> IUnknownRef? {
        // Use toABI as derived classes may override this to get the ABI pointer of the swift
        // object they are holding onto
        try! toABI {
            $0.withMemoryRebound(to: C_IUnknown.self, capacity: 1) {
                var iid = iid
                var result: UnsafeMutableRawPointer?
                guard $0.pointee.lpVtbl.pointee.QueryInterface($0, &iid, &result) == S_OK, let result else { return nil }
                return IUnknownRef(consuming: result)
            }
        }
    }

    public static func fromRaw(_ pUnk: UnsafeMutableRawPointer?) -> Unmanaged<WinRTWrapperBase>? {
      guard let pUnk = pUnk else { return nil }
      return pUnk.assumingMemoryBound(to: WinRTWrapperBase.ComObject.self).pointee.wrapper
    }

    public static func tryUnwrapFrom(raw pUnk: UnsafeMutableRawPointer?) -> Prototype? {
      guard let pUnk = pUnk else { return nil }
      return fromRaw(pUnk)?.takeUnretainedValue().swiftObj
    }

    // When unwrapping from the abi, we want to see if the object has an existing implementation so we can use
    // that to get to the existing swift object. if it doesn't exist then we can create a new implementation
    public static func tryUnwrapFrom(abi pointer: UnsafeMutablePointer<CInterface>?) -> Prototype? {
        guard let pointer = pointer else { return nil }
        let delegate = IUnknown(pointer)
        guard let wrapper: ISwiftImplemented = try? delegate.QueryInterface() else { return nil }
        let pUnk = UnsafeMutableRawPointer(wrapper.pUnk.borrow)

          // try to get the original wrapper so we can get the apps implementation. if that doesn't
          // exist, then return nil

        guard let wrapper  = pUnk.bindMemory(to: WinRTWrapperBase.ComObject.self, capacity: 1).pointee.wrapper else { return nil }
        return wrapper.takeRetainedValue().swiftObj
    }

    public static func addRef(_ pUnk: UnsafeMutablePointer<CInterface>?) -> ULONG {
        guard let wrapper = fromRaw(pUnk) else { return 1 }
        _ = wrapper.retain()
        return ULONG(_getRetainCount(wrapper.takeUnretainedValue()))
    }

    public static func release(_ pUnk: UnsafeMutablePointer<CInterface>?) -> ULONG {
        guard let wrapper = fromRaw(pUnk) else { return 1 }
        return ULONG(_getRetainCount(wrapper.takeRetainedValue()))
    }

    fileprivate static func queryInterfaceBase(_ pUnk: UnsafeMutablePointer<CInterface>, _ riid: UnsafePointer<SUPPORT_MODULE.IID>, _ result: UnsafeMutablePointer<UnsafeMutableRawPointer?>) -> HRESULT {
        guard let instance = tryUnwrapFrom(raw: pUnk) else { return E_FAIL }
        do
        {
            switch riid.pointee {
                case IID_IMarshal:
                    try makeMarshaler(IUnknownRef(pUnk), result)
                default:
                    guard let customQI = instance as? CustomQueryInterface,
                          let iUnknownRef = customQI.queryInterface(riid.pointee) else { return E_NOINTERFACE }
                    result.pointee = UnsafeMutableRawPointer(iUnknownRef.ref)
            }
            return S_OK
        } catch {
            return (error as? SUPPORT_MODULE.Error)?.hr ?? E_FAIL
        }
    }
}

open class WinRTAbiBridgeWrapper<I: AbiBridge> : WinRTWrapperBase<I.CABI, I.SwiftProjection> {

    public static func unwrapFrom(abi pointer: UnsafeMutablePointer<I.CABI>?) -> I.SwiftProjection? {
        guard let pointer = pointer else { return nil }
        guard let unwrapped = tryUnwrapFrom(abi: pointer) else { return I.from(abi: pointer) }
        return unwrapped
    }

    open class func queryInterface(_ pUnk: UnsafeMutablePointer<I.CABI>?, _ riid: UnsafePointer<SUPPORT_MODULE.IID>?, _ ppvObject: UnsafeMutablePointer<UnsafeMutableRawPointer?>?) -> HRESULT {
        guard let pUnk, let riid, let ppvObject else { return E_INVALIDARG }
        ppvObject.pointee = nil
         switch riid.pointee {
            case IUnknown.IID, IInspectable.IID, ISwiftImplemented.IID, IAgileObject.IID, Self.IID:
                _ = addRef(pUnk)
                ppvObject.pointee = UnsafeMutableRawPointer(pUnk)
                return S_OK
            default:
                return super.queryInterfaceBase(pUnk, riid, ppvObject)
        }
    }
}

open class InterfaceWrapperBase<I: AbiInterfaceBridge> : WinRTAbiBridgeWrapper<I> {
    override public class var IID: SUPPORT_MODULE.IID { I.SwiftABI.IID }
    public init?(_ impl: I.SwiftProjection?) {
        guard let impl = impl else { return nil }
        // try to see if already wrapping an ABI pointer and if so, use that
        if let internalImpl = impl as? AnyAbiInterfaceImpl<I> {
            let abi: UnsafeMutablePointer<I.CABI> = RawPointer(internalImpl)
            super.init(abi.pointee, impl)
        } else {
            let abi = I.makeAbi()
            super.init(abi, impl)
        }
    }

    override public func toABI<ResultType>(_ body: (UnsafeMutablePointer<I.CABI>) throws -> ResultType)
        throws -> ResultType {
        // If this is an implementation then we're holding onto a WinRT object pointer, get that pointer
        // and return that.
        if let internalImpl = swiftObj as? AnyAbiInterfaceImpl<I> {
            let abi: UnsafeMutablePointer<I.CABI> = RawPointer(internalImpl._default)
            return try body(abi)
        } else {
            return try super.toABI(body)
        }
    }
}

public class ReferenceWrapperBase<I: ReferenceBridge>: WinRTAbiBridgeWrapper<I> {
    override public class var IID: SUPPORT_MODULE.IID { I.IID }

    public init?(_ value: I.SwiftProjection?) {
        guard let value = value else { return nil }
        let abi = I.makeAbi()
        super.init(abi, value)
    }

    override public class func queryInterface(_ pUnk: UnsafeMutablePointer<I.CABI>?, _ riid: UnsafePointer<SUPPORT_MODULE.IID>?, _ ppvObject: UnsafeMutablePointer<UnsafeMutableRawPointer?>?) -> HRESULT {
        guard let pUnk, let riid, let ppvObject else { return E_INVALIDARG }
        ppvObject.pointee = nil
         switch riid.pointee {
            case __ABI_Windows_Foundation.IPropertyValueWrapper.IID:
                guard let value = tryUnwrapFrom(raw: pUnk),
                    let wrapper = __ABI_Windows_Foundation.IPropertyValueWrapper(__IMPL_Windows_Foundation.IPropertyValueImpl(value: value)) else { return E_FAIL }
                guard let iUnk = wrapper.queryInterface(__ABI_Windows_Foundation.IPropertyValueWrapper.IID) else { return E_NOINTERFACE }
                ppvObject.pointee = UnsafeMutableRawPointer(iUnk.ref)
                return S_OK
            default:
                return super.queryInterface(pUnk, riid, ppvObject)
        }
    }
}