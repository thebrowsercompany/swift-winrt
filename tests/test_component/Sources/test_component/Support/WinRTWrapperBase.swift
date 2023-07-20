import Ctest_component
import WinSDK

public protocol CanMakeFromAbi {
    static var _makeFromAbi: any MakeFromAbi.Type { get }
}
public protocol Initializable {
  init()
}

public protocol InitializableFromAbi : HasIID {
    associatedtype ABI
    init?(ref: UnsafeMutablePointer<ABI>?)
}

public protocol HasIID {
  static var IID: IID { get }
}

public protocol AbiInterface {
    associatedtype CABI
    associatedtype SwiftABI : test_component.IUnknown
}

// A protocol for defining a type which implements a WinRT interface and defines
// the swift <-> winrt translation
public protocol AbiBridge {
    associatedtype CABI
    associatedtype SwiftProjection
    static func makeAbi() -> CABI
}

public protocol ReferenceImpl : AbiBridge where ValueType.ABI == CABI {
    associatedtype ValueType : InitializableFromAbi
}

public protocol AbiInterfaceImpl : AbiBridge & AbiInterface {
    static func from(abi: UnsafeMutablePointer<CABI>?) -> SwiftProjection?
    var _default: SwiftABI { get }
}

extension AbiInterfaceImpl where SwiftABI: IInspectable {
   public var thisPtr: test_component.IInspectable {  _default }
}

public protocol WinRTAbiBridge: AbiInterfaceImpl where SwiftABI: IInspectable {}
internal typealias AnyWinRTAbiBridge = any WinRTAbiBridge


// The WinRTWrapperBase class wraps an AbiBridge and is used for wrapping and unwrapping swift
// objects at the ABI layer. The contract for how to do this is defined by the AbiBridge protocol
open class WinRTWrapperBase<CInterface, Prototype> {
    public struct ComObject {
        public var comInterface: CInterface
        public var wrapper: Unmanaged<WinRTWrapperBase>?
    }

    public var instance: ComObject
    public var swiftObj: Prototype

    open class var IID: IID { get { fatalError("not implemented") } }

    public init(_ pointer: CInterface, _ impl: Prototype) {
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
        abi.withMemoryRebound(to: WinSDK.IUnknown.self, capacity: 1) { 
            _ = $0.pointee.lpVtbl.pointee.AddRef($0)
        }
        
        ptr.initialize(to: abi)
    }

    public func queryInterface(_ iid: IID) -> IUnknownRef? {
        // Use toABI as derived classes may override this to get the ABI pointer of the swift
        // object they are holding onto
        try! toABI {
            $0.withMemoryRebound(to: WinSDK.IUnknown.self, capacity: 1) {
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
}

open class WinRTWrapperBase2<I: AbiBridge> : WinRTWrapperBase<I.CABI, I.SwiftProjection> {
}

open class InterfaceWrapperBase<I: AbiInterfaceImpl> : WinRTWrapperBase2<I> {
    override public class var IID: IID { I.SwiftABI.IID }
    public init?(_ impl: I.SwiftProjection?) {
        guard let impl = impl else { return nil }
        // try to see if already wrapping an ABI pointer and if so, use that
        if let internalImpl = impl as? I {
            let abi: UnsafeMutablePointer<I.CABI> = RawPointer(internalImpl)
            super.init(abi.pointee, impl)
        } else {
            let abi = I.makeAbi()
            super.init(abi, impl)
        }
    }

    public static func unwrapFrom(abi pointer: UnsafeMutablePointer<I.CABI>?) -> I.SwiftProjection? {
        guard let pointer = pointer else { return nil }
        guard let unwrapped = tryUnwrapFrom(abi: pointer) else { return I.from(abi: pointer) }
        return unwrapped
    }

    override public func toABI<ResultType>(_ body: (UnsafeMutablePointer<I.CABI>) throws -> ResultType)
        throws -> ResultType {
        // If this is an implementation then we're holding onto a WinRT object pointer, get that pointer
        // and return that.
        if let internalImpl = swiftObj as? I {
            let abi: UnsafeMutablePointer<I.CABI> = RawPointer(internalImpl._default)
            return try body(abi)
        } else {
            return try super.toABI(body)
        }
    }
}
