import C_BINDINGS_MODULE
import Foundation

public protocol WinRTClass : IWinRTObject, Equatable {
    func _getABI<T>() -> UnsafeMutablePointer<T>?
}

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

public protocol UnsealedWinRTClass : WinRTClass {
    var _inner: UnsafeMutablePointer<C_BINDINGS_MODULE.IInspectable>? { get }
    // rather than require an initializer, expose a type which can create this class.
    // we do this so app derived types don't have to implement initializers that will
    // never be called
    static var _makeFromAbi: any MakeFromAbi.Type { get }
}

public extension WinRTClass {
    func _getDefaultAsIInspectable() -> SUPPORT_MODULE.IInspectable {
        // Every WinRT interface is binary compatible with IInspectable. asking this class for
        // the iinspectable will ensure we get the default implementation from whichever derived
        // class it actually is. 
        let cDefault: UnsafeMutablePointer<C_BINDINGS_MODULE.IInspectable> = _getABI()!
        return IInspectable(cDefault)
    }

    func `as`<Interface: SUPPORT_MODULE.IInspectable>() throws -> Interface {
        try _getDefaultAsIInspectable().QueryInterface()
    }
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
            try! CHECKED(inner.pointee.lpVtbl.pointee.GetRuntimeClassName(inner, &name)) 
            return .init(consuming: name)
        } else {
            let string = NSStringFromClass(type(of: self))
            return try! HString(string)
        }
    }
}

public func ==<T: WinRTClass>(_ lhs: T, _ rhs: T) -> Bool {
  return lhs._getDefaultAsIInspectable() == rhs._getDefaultAsIInspectable()
}

public protocol ComposableActivationFactory {
    associatedtype Composable : ComposableImpl

    func CreateInstanceImpl(
            _ base: UnsafeMutablePointer<C_BINDINGS_MODULE.IInspectable>?,
            _ inner: inout UnsafeMutablePointer<C_BINDINGS_MODULE.IInspectable>?) throws -> UnsafeMutablePointer<Composable.Default.CABI>? 
}


// The composition types are unsealed but the app can't override them, we shouldn't generate these constructors. See:
// https://linear.app/the-browser-company/issue/WIN-110/swiftwinrt-dont-allow-app-to-derive-from-types-without-a-constructor
public extension ComposableActivationFactory {
    func CreateInstanceImpl(
            _ base: UnsafeMutablePointer<C_BINDINGS_MODULE.IInspectable>?,
            _ inner: inout UnsafeMutablePointer<C_BINDINGS_MODULE.IInspectable>?) throws -> UnsafeMutablePointer<Composable.Default.CABI>? {
        throw Error(hr: E_NOTIMPL)
    }
}

// When composing WinRT types, we need to use the composable factory of the most derived type. For types with a deep hierarchy
// structure (i.e. WinUI) there are many base classes, but we only need to call CreateInstance on the most derived. This will 
// give us back the "inner" object, which represents a non-delegating pointer to the base type. We need to hold onto this pointer for QI
// calls so that we can send calls to the inner object when the app doesn't override methods. In COM terms, the swift object is
// considered the "controlling unknown", meaning all QI calls for IUnknown and IInspectable should come to the swift object and we
// forward other calls to the inner object

public func MakeComposed<Factory: ComposableActivationFactory>(_ factory: Factory, _ inner: inout UnsafeMutablePointer<C_BINDINGS_MODULE.IInspectable>?, _ this: Factory.Composable.Default.SwiftProjection) -> Factory.Composable.Default.SwiftABI {
    let wrapper:UnsealedWinRTClassWrapper<Factory.Composable>? = .init(this)

    let abi = try! wrapper?.toABI { $0 }
    let baseInsp = abi?.withMemoryRebound(to: C_BINDINGS_MODULE.IInspectable.self, capacity: 1) { $0 }
    let base = try! factory.CreateInstanceImpl(baseInsp, &inner)
    return Factory.Composable.Default.SwiftABI(base!)
}

public class UnsealedWinRTClassWrapper<Composable: ComposableImpl> : WinRTWrapperBase<Composable.CABI, Composable.Default.SwiftProjection> {
    override public class var IID: IID { Composable.SwiftABI.IID }

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
        let className = try! overrides.GetSwiftClassName() 
        guard let baseType = NSClassFromString(className) as? Composable.Default.SwiftProjection.Type else {
          // the derived class doesn't exist, which is fine, just return the type the API specifies.
          return Composable.Default.from(abi: base)!
        }
        return baseType._makeFromAbi.from(abi: base) as! Composable.Default.SwiftProjection
    }
}
