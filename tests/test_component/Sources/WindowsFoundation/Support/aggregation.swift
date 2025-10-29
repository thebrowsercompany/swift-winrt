import CWinRT
import Foundation

// The WinRTClassWeakReference class is a proxy for properly managing the reference count of
// the WinRTClass that is being aggregated. The aggregated object holds a weak reference to
// the outer IInspectable (swift) that is passed in during construction. In general, the
// swift wrappers we create hold strong references to the objects they are wrapping, as we
// expect an AddRef from WinRT to keep the object alive. Since this doesn't happen for aggregated
// objects, we need a proxy which sits in the middle. The WinRTClassWeakReference object doesn't
// keep a strong ref to the swift object, but it forwards all AddRef/Release calls from WinRT
// to the swift object, to ensure it doesn't get cleaned up. The Swift object in turn holds a strong
// reference to this object so that it stays alive.
@_spi(WinRTInternal)
public final class WinRTClassWeakReference<Class: WinRTClass> {
    fileprivate weak var instance: Class?
    public init(_ instance: Class){
        self.instance = instance
    }
}

extension WinRTClassWeakReference: CustomQueryInterface {
    @_spi(WinRTImplements)
    public func queryInterface(_ iid: WindowsFoundation.IID) -> IUnknownRef? {
        guard let instance else { return nil }
        return instance.queryInterface(iid)
    }
}

extension WinRTClassWeakReference: CustomAddRef {
    func addRef() {
        guard let instance else { return }
        let unmanaged = Unmanaged.passUnretained(instance)
        _ = unmanaged.retain()
    }

    func release() {
        guard let instance else { return }
        let unmanaged = Unmanaged.passUnretained(instance)
        unmanaged.release()
    }
}

extension WinRTClassWeakReference: AnyObjectWrapper {
    var obj: AnyObject? { instance }
}

@_spi(WinRTInternal)
public protocol ComposableImpl<Class> : AbiInterfaceBridge where SwiftABI: IInspectable, SwiftProjection: WinRTClassWeakReference<Class>  {
    associatedtype Class: WinRTClass
    associatedtype Default : AbiInterface where Default.SwiftABI: WindowsFoundation.IInspectable
    static func makeAbi() -> CABI
}

@_spi(WinRTInternal)
public protocol ComposableBridge<SwiftProjection>: AbiBridge where SwiftProjection: WinRTClass  {
    associatedtype Composable: ComposableImpl<SwiftProjection>
}

// At a high level, aggregation simply requires the WinRT object to have a pointer back to the Swift world, so that it can call
// overridable methods on the class. This Swift pointer is given to the WinRT object during construction. The construction of the
// WinRT object returns us two different pointers:

// 1. A non-delegating "inner" pointer. A non-delegating pointer means that any QueryInterface calls won't "delegate" back into the Swift world
// 2. A pointer to the default interface.

// Below is a table which shows what the input parameters to CreateInstance is, and what we should do with the
// output parameters in order to properly aggregate a type. For reference, a constructor for a winrt object (without any parameters)
// looks like this:

// CreateInstance(IInspectable* baseInsp, IInspectable** innerInsp, IInspectable** result)

// |  Aggregating? |  baseInsp (Swift pointer) | innerInsp (C++ pointer) | result (C++)             |
// |---------------|---------------------------|-------------------------|--------------------------|
// |  Yes          |  self                     |  stored on swift object |  ignored or stored       |
// |  No           |  nil                      |  ignored                |  stored on swift object  |
@_spi(WinRTInternal)
public func MakeComposed<Composable: ComposableImpl>(
    composing: Composable.Type,
    _ this: Composable.Class,
    _ createCallback: (UnsealedWinRTClassWrapper<Composable>?, inout WindowsFoundation.IInspectable?) -> Composable.Default.SwiftABI) {
    let aggregated = type(of: this) != Composable.Class.self
    let wrapper:UnsealedWinRTClassWrapper<Composable>? = .init(aggregated ? this : nil)

    var innerInsp: WindowsFoundation.IInspectable? = nil
    let base = createCallback(wrapper, &innerInsp)
    guard let innerInsp else {
        fatalError("Unexpected nil returned after successful creation")
    }

    if let wrapper {
        this.identity = ComPtr(wrapper.toIInspectableABI { $0 })
        // Storing a strong ref to the wrapper adds a ref to ourselves, remove the
        // reference
        wrapper.swiftObj.release()
    }
    this._inner = aggregated ? innerInsp : base
}

@_spi(WinRTInternal)
public class UnsealedWinRTClassWrapper<Composable: ComposableImpl> : WinRTAbiBridgeWrapper<Composable> {
    override public class var IID: WindowsFoundation.IID { Composable.SwiftABI.IID }
    public init?(_ impl: Composable.Class?) {
        guard let impl = impl else { return nil }
        let abi = Composable.makeAbi()
        super.init(abi, Composable.SwiftProjection(impl))
    }

    public static func unwrapFrom(base: ComPtr<Composable.Default.CABI>) -> Composable.Class? {
        let overrides: Composable.SwiftABI = try! base.queryInterface()
        if let weakRef = tryUnwrapFrom(abi: RawPointer(overrides)) { return weakRef.instance }
        guard let instance = makeFrom(abi: overrides) else {
            // the derived class doesn't exist, which is fine, just return the type the API specifies.
            return make(type: Composable.Class.self, from: overrides)
        }
        return instance as? Composable.Class
    }

    public static func tryUnwrapFrom(raw pUnk: UnsafeMutableRawPointer?) -> Composable.Class? {
        tryUnwrapFromBase(raw: pUnk)?.instance
    }

    public func toIInspectableABI<ResultType>(_ body: (UnsafeMutablePointer<C_IInspectable>) throws -> ResultType)
        rethrows -> ResultType {
        let abi = try! toABI { $0 }
        return try abi.withMemoryRebound(to: C_IInspectable.self, capacity: 1) { try body($0) }
    }
}

public extension ComposableImpl {
    static func from(abi: ComPtr<CABI>?) -> SwiftProjection? {
        return nil
    }
}
