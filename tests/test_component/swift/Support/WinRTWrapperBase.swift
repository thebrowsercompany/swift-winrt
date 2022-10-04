import Ctest_component
import WinSDK

public protocol IWinRTObject: AnyObject {

}

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
