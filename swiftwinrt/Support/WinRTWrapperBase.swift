import C_BINDINGS_MODULE
import WinSDK

public protocol IWinRTObject: AnyObject {

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
    associatedtype c_ABI
    associatedtype swift_ABI : test_component.IUnknown
}

// A protocol for defining a type which implements a WinRT interface and defines
// the swift <-> winrt translation
public protocol AbiImpl {
    associatedtype c_ABI
    associatedtype swift_Projection
    static func makeAbi() -> c_ABI
}

public protocol ReferenceImpl : AbiImpl where ValueType.ABI == c_ABI {
    associatedtype ValueType : InitializableFromAbi
}

public protocol AbiInterfaceImpl : AbiImpl  & AbiInterface {
    static func from(abi: UnsafeMutablePointer<c_ABI>?) -> swift_Projection
    var _default: swift_ABI { get }
}

// The WinRTWrapperBase class wraps an AbiImpl and is used for wrapping and unwrapping swift
// objects at the ABI layer. The contract for how to do this is defined by the AbiImpl protocol
open class WinRTWrapperBase<CInterface, Prototype> {
    public struct ComObject {
        public var comInterface: CInterface
        public var wrapper: Unmanaged<WinRTWrapperBase>?
    }

    public var instance: ComObject
    public var swiftObj: Prototype

    open class var IID: IID { get { fatalError("not implemented") } }

    deinit {
        // nil out the wrapper, so that we don't try to decrememnt the ref count in the `Release` method, this
        // causes an infinite loop
        self.instance.wrapper = nil
    }

    public init(_ pointer: CInterface, _ impl: Prototype) {
        self.instance = ComObject(comInterface: pointer)
        self.swiftObj = impl
        self.instance.wrapper = Unmanaged<WinRTWrapperBase>.passUnretained(self)
    }

    @_alwaysEmitIntoClient @inline(__always)
    public func to_abi<ResultType>(_ body: (UnsafeMutablePointer<CInterface>) throws -> ResultType)
        throws -> ResultType {

        try withUnsafeMutablePointer(to:&instance.comInterface){
          return try body($0)
        }
    }

    public static func from_raw(_ pUnk: UnsafeMutableRawPointer?) -> Unmanaged<WinRTWrapperBase>? {
      return pUnk?.assumingMemoryBound(to: WinRTWrapperBase.ComObject.self).pointee.wrapper
    }

    public static func try_unwrap_from(raw pUnk: UnsafeMutableRawPointer?) -> Prototype? {
      return from_raw(pUnk)?.takeUnretainedValue().swiftObj
    }
    
    // When unwrapping from the abi, we want to see if the object has an existing implementation so we can use
    // that to get to the existing swift object. if it doesn't exist then we can create a new implementation
    public static func try_unwrap_from(abi pointer: UnsafeMutablePointer<CInterface>?) -> Prototype? {
        let delegate = IUnknown(pointer)
        let wrapperOpt: ISwiftImplemented? = try? delegate.QueryInterface()
        guard let wrapper = wrapperOpt,
              let pUnk = UnsafeMutableRawPointer(wrapper.pUnk.borrow) else { return nil }

          // try to get the original wrapper so we can get the apps implementation. if that doesn't
          // exist, then return nil

        guard let wrapper  = pUnk.bindMemory(to: WinRTWrapperBase.ComObject.self, capacity: 1).pointee.wrapper else { return nil }
        return wrapper.takeRetainedValue().swiftObj
    }
}

open class WinRTWrapperBase2<I: AbiImpl> : WinRTWrapperBase<I.c_ABI, I.swift_Projection> {
}

open class InterfaceWrapperBase<I: AbiInterfaceImpl> : WinRTWrapperBase2<I> {
    override public class var IID: IID { I.swift_ABI.IID }
    public init?(_ impl: I.swift_Projection?) {
        guard let impl = impl else { return nil }
        // try to see if already wrapping an ABI pointer and if so, use that
        if let internalImpl = impl as? I {
            let abi: UnsafeMutablePointer<I.c_ABI>? = RawPointer(internalImpl._default)
            guard let abi = abi else { return nil }
            super.init(abi.pointee, impl)
        } else {
            let abi = I.makeAbi()
            super.init(abi, impl)
        }
    }

     public static func unwrap_from(abi pointer: UnsafeMutablePointer<I.c_ABI>?) -> I.swift_Projection {
        let delegate = IUnknown(pointer)
        let wrapperOpt: ISwiftImplemented? = try? delegate.QueryInterface()
        guard let wrapper = wrapperOpt,
              let pUnk = UnsafeMutableRawPointer(wrapper.pUnk.borrow) else { 
                 return I.from(abi: pointer)
              }

          // try to get the original wrapper so we can get the apps implementation. if that doesn't
          // exist, then return nil
        guard let wrapper  = pUnk.bindMemory(to: InterfaceWrapperBase.ComObject.self, capacity: 1).pointee.wrapper else {   return I.from(abi: pointer) }
        return wrapper.takeRetainedValue().swiftObj
    }

    override public func to_abi<ResultType>(_ body: (UnsafeMutablePointer<I.c_ABI>) throws -> ResultType)
        throws -> ResultType {
        // If this is an implementation then we're holding onto a WinRT object pointer, get that pointer
        // and return that.
        if let internalImpl = swiftObj as? I {
            let abi: UnsafeMutablePointer<I.c_ABI>? = RawPointer(internalImpl._default)
            return try body(abi!)
        } else {
            return try super.to_abi(body)
        }
    }
}


open class IReferenceWrapperBase<I: ReferenceImpl> : WinRTWrapperBase2<I> {
    override public class var IID: IID { I.ValueType.IID }
    public init?(_ value: I.ValueType?) {
        guard let value = value else { return nil }
        let abi = I.makeAbi()
        super.init(abi, __IMPL_Windows_Foundation.IPropertyValueImpl(value: value) as! I.swift_Projection)
    }

    public static func unwrap_from(abi pointer: UnsafeMutablePointer<I.c_ABI>?) -> I.ValueType? {
        return .init(ref: pointer)
    }
}
