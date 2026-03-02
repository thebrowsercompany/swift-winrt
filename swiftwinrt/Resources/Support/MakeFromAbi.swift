import Foundation
import WinSDK

// Not strongly typed, we lose the type safety of the associatedtype anyways
// when we cast to `any MakeFromAbi`, plus that requires a lot more exported
// simples than we want
public protocol MakeFromAbi {
    associatedtype SwiftType
    static func from(abi: SUPPORT_MODULE.IInspectable) -> SwiftType
}

func make(typeName: SwiftTypeName, from abi: SUPPORT_MODULE.IInspectable) -> Any? {
    guard let makerType = NSClassFromString("\(typeName.module).\(typeName.typeName)Maker") as? any MakeFromAbi.Type else {
        return nil
    }
    return makerType.from(abi: abi)
}

/// Cached result of resolving a WinRT runtime class name to its Swift factory type.
///
/// We cache both positive and negative results so that class names with no corresponding
/// Swift projection (e.g. internal WinRT types) don't repeatedly pay the cost of
/// namespace parsing, string interpolation, and `NSClassFromString` on every crossing.
///
private enum FactoryCacheEntry {
    case factory(any MakeFromAbi.Type)

    /// The class name has no corresponding Swift factory type.
    ///
    /// This is normal for:
    /// - Exclusive-to interfaces: private to a specific class (e.g. `IButtonFactory`), not projected
    /// - Filtered-out types: exist in metadata but excluded by the projection's type filters
    /// - Generic interfaces: the code generator skips `MakeFromAbi` for open generic types
    /// - Aggregated derived types: app-implemented subclasses whose runtime class name has no Maker
    ///
    case noFactory
}

private var factoryCache: [String: FactoryCacheEntry] = [:]
private var factoryCacheLock = SRWLock()

func makeFrom(abi: SUPPORT_MODULE.IInspectable) -> Any? {
    // When creating a swift class which represents this type, we want to get the class name that we're trying to create
    // via GetRuntimeClassName so that we can create the proper derived type. For example, the API may return UIElement,
    // but we want to return a Button type.

    // Note that we'll *never* be trying to create an app implemented object at this point
    guard let className = try? String(hString: abi.GetRuntimeClassName()) else { return nil }

    let cached = factoryCacheLock.withLock(.shared) { factoryCache[className] }

    if let cached {
        switch cached {
        case .factory(let factory):
            return factory.from(abi: abi)
        case .noFactory:
            return nil
        }
    }

    // Note: Because we read-then-write without holding the lock across both, concurrent threads
    // may redundantly resolve the same type. This is harmless (the result is identical) and
    // lets us use shared reader locking on the hot path.

    let factory: (any MakeFromAbi.Type)?
    if let typeName = IInspectable.GetSwiftTypeName(from: className) {
        factory = NSClassFromString("\(typeName.module).\(typeName.typeName)Maker") as? any MakeFromAbi.Type
    } else {
        factory = nil
    }

    let entry: FactoryCacheEntry = factory.map { .factory($0) } ?? .noFactory
    factoryCacheLock.withLock(.exclusive) { factoryCache[className] = entry }

    return factory?.from(abi: abi)
}

func make<T:AnyObject>(type: T.Type, from abi: SUPPORT_MODULE.IInspectable) -> T? {
    let classString = NSStringFromClass(type).split(separator: ".", maxSplits: 2)
    return make(typeName: SwiftTypeName(module: String(classString[0]), typeName: String(classString[1])), from: abi) as? T
}
