import C_BINDINGS_MODULE
import Foundation

public protocol WinRTClass : IWinRTObject, Equatable {
    func _get_abi<T>() -> UnsafeMutablePointer<T>?
}

public protocol MakeFromAbi {
    associatedtype c_ABI
    associatedtype swift_Projection
    static func from(abi: UnsafeMutableRawPointer?) -> swift_Projection
}

public protocol MakeComposedAbi : MakeFromAbi where swift_Projection: UnsealedWinRTClass {
    associatedtype swift_ABI : SUPPORT_MODULE.IInspectable
}

public protocol ComposableImpl : AbiInterface where swift_ABI: IInspectable  {
    associatedtype Default : MakeComposedAbi
    static func makeAbi() -> c_ABI
}

public protocol UnsealedWinRTClass : WinRTClass {
    var _inner: UnsafeMutablePointer<C_BINDINGS_MODULE.IInspectable>? { get }
    // rather than require an initializer, expose a type which can create this class.
    // we do this so app derived types don't have to implement initializers that will
    // never be called
    static var _makeFromAbi: any MakeFromAbi.Type { get }
}

public extension WinRTClass {

    func getDefault() -> SUPPORT_MODULE.IInspectable {
        // Every WinRT interface is binary compatible with IInspectable. asking this class for
        // the iinspectable will ensure we get the default implementation from whichever derived
        // class it actually is. 
        let cDefault: UnsafeMutablePointer<C_BINDINGS_MODULE.IInspectable>? = _get_abi()
        return IInspectable(cDefault)
    }
    func `as`<Interface: SUPPORT_MODULE.IInspectable>() throws -> Interface {
        try getDefault().QueryInterface()
    }

    func try_as<Interface: SUPPORT_MODULE.IInspectable>() -> Interface? {
        return try? getDefault().QueryInterface()
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
  return lhs.getDefault() == rhs.getDefault()
}

public protocol ComposableActivationFactory {
    associatedtype Composable : ComposableImpl

    func CreateInstanceImpl(
            _ base: UnsafeMutablePointer<C_BINDINGS_MODULE.IInspectable>?,
            _ inner: inout UnsafeMutablePointer<C_BINDINGS_MODULE.IInspectable>?)  throws -> UnsafeMutablePointer<Composable.Default.c_ABI>? 
}


// The composition types are unsealed but the app can't override them, we shouldn't generate these constructors. See:
// https://linear.app/the-browser-company/issue/WIN-110/swiftwinrt-dont-allow-app-to-derive-from-types-without-a-constructor
public extension ComposableActivationFactory {
    func CreateInstanceImpl(
            _ base: UnsafeMutablePointer<C_BINDINGS_MODULE.IInspectable>?,
            _ inner: inout UnsafeMutablePointer<C_BINDINGS_MODULE.IInspectable>?)  throws -> UnsafeMutablePointer<Composable.Default.c_ABI>? {
        throw Error(hr: E_NOTIMPL)
    }
}

// When composing WinRT types, we need to use the composable factory of the most derived type. For types with a deep hierarchy
// structure (i.e. WinUI) there are many base classes, but we only need to call CreateInstance on the most derived. This will 
// give us back the "inner" object, which represents a non-delegating pointer to the base type. We need to hold onto this pointer for QI
// calls so that we can send calls to the inner object when the app doesn't override methods. In COM terms, the swift object is
// considered the "controlling unknown", meaning all QI calls for IUnknown and IInspectable should come to the swift object and we
// forward other calls to the inner object

public func MakeComposed<Factory: ComposableActivationFactory>(_ factory: Factory, _ inner: inout UnsafeMutablePointer<C_BINDINGS_MODULE.IInspectable>?, _ this: Factory.Composable.Default.swift_Projection) -> Factory.Composable.Default.swift_ABI {
    let wrapper:UnsealedWinRTClassWrapper<Factory.Composable>? = .init(this)

    let abi = try! wrapper?.to_abi { $0 }
    let baseInsp = abi?.withMemoryRebound(to: C_BINDINGS_MODULE.IInspectable.self, capacity: 1) { $0 }
    let base = try! factory.CreateInstanceImpl(baseInsp, &inner)
    return Factory.Composable.Default.swift_ABI(base)
}

public class UnsealedWinRTClassWrapper<Composable: ComposableImpl> : WinRTWrapperBase<Composable.c_ABI, Composable.Default.swift_Projection> {

    override public class var IID: IID { Composable.swift_ABI.IID }
    public init?(_ impl: Composable.Default.swift_Projection?) {
        guard let impl = impl else { return nil }
        let abi = Composable.makeAbi()
        super.init(abi, impl)
    }

    public static func unwrap_from(base: UnsafeMutablePointer<Composable.Default.c_ABI>) -> Composable.Default.swift_Projection {
        let baseInsp = IInspectable(base)
        let overrides: Composable.swift_ABI = try! baseInsp.QueryInterface()

        // Try to unwrap an app implemented object. If one doesn't exist then we'll create the proper WinRT type below
        if let instance = try_unwrap_from(abi: RawPointer(overrides)) {
            return instance
        }

        // We don't use the `Composable` type here because we have to get the actual implementation of this base 
        // class and then get *that types* composing creator. This allows us to be able to properly create a derived type.
        // Note that we'll *never* be trying to create an app implemented object at this point
        let className = try! overrides.GetSwiftClassName() 
        let baseType = NSClassFromString(className) as! Composable.Default.swift_Projection.Type
        return baseType._makeFromAbi.from(abi: base) as! Composable.Default.swift_Projection
    }
}