import C_BINDINGS_MODULE
import Foundation

public protocol MakeFromAbi {
    associatedtype CABI
    associatedtype SwiftProjection
    static func from(abi: UnsafeMutableRawPointer?) -> SwiftProjection?
}

public protocol MakeComposedAbi : MakeFromAbi where SwiftProjection: UnsealedWinRTClass {
    associatedtype SwiftABI : SUPPORT_MODULE.IInspectable
}

public protocol ComposableImpl : AbiInterface where SwiftABI: IInspectable  {
    associatedtype Default : MakeComposedAbi
    static func makeAbi() -> CABI
}

extension UnsealedWinRTClass {
    public func GetRuntimeClassName() -> HString {
        if let inner = _inner {
            // if there is an inner object, get the runtime class from there, very rarely do
            // we want to get the swift name in this sense. the winui runtime will query for
            // class names and if it isn't recognized, it will call out to IXamlMetadataProvider (IXMP)
            // to get the associated XamlType. We aren't using Xaml for swift, so we don't actually
            // need or want the framework to think it's dealing with custom types.
            var name: HSTRING?
            try! inner.borrow.withMemoryRebound(to: C_IInspectable.self, capacity: 1) {
                _ = try CHECKED($0.pointee.lpVtbl.pointee.GetRuntimeClassName($0, &name))
            }
            return .init(consuming: name)
        } else {
            let string = NSStringFromClass(type(of: self))
            return try! HString(string)
        }
    }
}

public protocol ComposableActivationFactory {
    associatedtype Composable : ComposableImpl

    func CreateInstanceImpl(
            _ base: UnsafeMutablePointer<C_IInspectable>?,
            _ inner: inout UnsafeMutablePointer<C_IInspectable>?) throws -> UnsafeMutablePointer<Composable.Default.CABI>?
}


// The composition types are unsealed but the app can't override them, we shouldn't generate these constructors. See:
// https://linear.app/the-browser-company/issue/WIN-110/swiftwinrt-dont-allow-app-to-derive-from-types-without-a-constructor
public extension ComposableActivationFactory {
    func CreateInstanceImpl(
            _ base: UnsafeMutablePointer<C_IInspectable>?,
            _ inner: inout UnsafeMutablePointer<C_IInspectable>?) throws -> UnsafeMutablePointer<Composable.Default.CABI>? {
        throw Error(hr: E_NOTIMPL)
    }
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
public func MakeComposed<Factory: ComposableActivationFactory>(_ factory: Factory,  _ inner: inout IUnknownRef?, _ this: Factory.Composable.Default.SwiftProjection) -> Factory.Composable.Default.SwiftABI {
    let aggregated = type(of: this) != Factory.Composable.Default.SwiftProjection.self
    let wrapper:UnsealedWinRTClassWrapper<Factory.Composable>? = .init(aggregated ? this : nil)

    let abi = try! wrapper?.toABI { $0 }
    let baseInsp = abi?.withMemoryRebound(to: C_IInspectable.self, capacity: 1) { $0 }
    var innerInsp: UnsafeMutablePointer<C_IInspectable>? = nil
    let base = try! factory.CreateInstanceImpl(baseInsp, &innerInsp)
    guard let innerInsp, let base else {
        fatalError("Unexpected nil returned after successful creation")
    }

    let baseRef = IUnknownRef(consuming: base)
    let innerRef = IUnknownRef(consuming: innerInsp)
    if aggregated {
        inner = innerRef
    } else {
        inner = baseRef
    }
    return Factory.Composable.Default.SwiftABI(base)
}

public class UnsealedWinRTClassWrapper<Composable: ComposableImpl> : WinRTWrapperBase<Composable.CABI, Composable.Default.SwiftProjection> {
    override public class var IID: SUPPORT_MODULE.IID { Composable.SwiftABI.IID }

    public init?(_ impl: Composable.Default.SwiftProjection?) {
        guard let impl = impl else { return nil }
        let abi = Composable.makeAbi()
        super.init(abi, impl)
    }

    public static func unwrapFrom(base: UnsafeMutablePointer<Composable.Default.CABI>) -> Composable.Default.SwiftProjection {
        let baseInsp = IInspectable(base)
        let overrides: Composable.SwiftABI = try! baseInsp.QueryInterface()

        // Try to unwrap an app implemented object. If one doesn't exist then we'll create the proper WinRT type below
        if let instance = tryUnwrapFrom(abi: RawPointer(overrides)) {
            return instance
        }

        // When creating a swift class which represents this type, we want to get the class name that we're trying to create
        // via GetRuntimeClassName so that we can create the proper derived type. For example, the API may return UIElement,
        // but we want to return a Button type.
        // Note that we'll *never* be trying to create an app implemented object at this point
        let className = try? overrides.GetSwiftClassName()
        guard let className, let baseType = NSClassFromString(className) as? Composable.Default.SwiftProjection.Type else {
          // the derived class doesn't exist, which is fine, just return the type the API specifies.
          return Composable.Default.from(abi: base)!
        }
        return baseType._makeFromAbi.from(abi: base) as! Composable.Default.SwiftProjection
    }
}
