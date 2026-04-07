import CWinRT
import Foundation
import WinSDK

// Not strongly typed, we lose the type safety of the associatedtype anyways
// when we cast to `any MakeFromAbi`, plus that requires a lot more exported
// simples than we want
public protocol MakeFromAbi {
    associatedtype SwiftType
    static func from(abi: WindowsFoundation.IInspectable) -> SwiftType
}

func make(typeName: SwiftTypeName, from abi: WindowsFoundation.IInspectable) -> Any? {
    guard let makerType = NSClassFromString("\(typeName.module).\(typeName.typeName)Maker") as? any MakeFromAbi.Type else {
        return nil
    }
    return makerType.from(abi: abi)
}

/// Dictionary key that wraps a raw HSTRING, hashing and comparing on the UTF-16 content without
/// converting to a Swift String.
///
/// Keys stored in the dictionary duplicate the HSTRING and intentionally never free it; the cache
/// lives for the process lifetime and contains at most one entry per WinRT class name. Transient
/// lookup keys borrow the caller's HSTRING without duplicating.
///
private struct FactoryCacheKey: Hashable {
    let hstring: HSTRING

    /// Transient key for dictionary lookups. Does not duplicate the HSTRING.
    ///
    /// - Important: Not generally safe to do, but in this narrow case we know the associated
    /// `HSTRING` will have a non-zero reference count while it's being used here.
    ///
    init(_unsafeBorrowing hstring: HSTRING) {
        self.hstring = hstring
    }

    /// Persistent key for dictionary storage. Duplicates the HSTRING (never freed).
    init(duplicating hstring: HSTRING) {
        var duplicate: HSTRING?
        _ = WindowsDuplicateString(hstring, &duplicate)
        guard let duplicate else { fatalError("Out of memory") }
        self.hstring = duplicate
    }

    func hash(into hasher: inout Hasher) {
        var length: UINT32 = 0
        let buffer = WindowsGetStringRawBuffer(hstring, &length)
        hasher.combine(bytes: UnsafeRawBufferPointer(
            start: buffer,
            count: Int(length) * MemoryLayout<WCHAR>.size
        ))
    }

    static func == (lhs: FactoryCacheKey, rhs: FactoryCacheKey) -> Bool {
        var result: Int32 = 0
        _ = WindowsCompareStringOrdinal(lhs.hstring, rhs.hstring, &result)
        return result == 0
    }
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

private var factoryCache: [FactoryCacheKey: FactoryCacheEntry] = [:]
private var factoryCacheLock = SRWLock()

func makeFrom(abi: WindowsFoundation.IInspectable) -> Any? {
    // When creating a swift class which represents this type, we want to get the class name that we're trying to create
    // via GetRuntimeClassName so that we can create the proper derived type. For example, the API may return UIElement,
    // but we want to return a Button type.

    // Note that we'll *never* be trying to create an app implemented object at this point

    // Get the raw HSTRING directly, avoiding the String conversion.
    var rawClassName: HSTRING?
    _ = try? abi.GetRuntimeClassName(&rawClassName)
    guard let rawClassName else { return nil }
    defer { WindowsDeleteString(rawClassName) }

    let lookupKey = FactoryCacheKey(_unsafeBorrowing: rawClassName)
    let cached = factoryCacheLock.withLock(.shared) { factoryCache[lookupKey] }

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

    let className = String(from: rawClassName)

    let factory: (any MakeFromAbi.Type)?
    if let typeName = IInspectable.GetSwiftTypeName(from: className) {
        factory = NSClassFromString("\(typeName.module).\(typeName.typeName)Maker") as? any MakeFromAbi.Type
    } else {
        factory = nil
    }

    let storedKey = FactoryCacheKey(duplicating: rawClassName)
    let entry: FactoryCacheEntry = factory.map { .factory($0) } ?? .noFactory
    factoryCacheLock.withLock(.exclusive) { factoryCache[storedKey] = entry }

    return factory?.from(abi: abi)
}

func make<T:AnyObject>(type: T.Type, from abi: WindowsFoundation.IInspectable) -> T? {
    let classString = NSStringFromClass(type).split(separator: ".", maxSplits: 2)
    return make(typeName: SwiftTypeName(module: String(classString[0]), typeName: String(classString[1])), from: abi) as? T
}
