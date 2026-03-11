import WinSDK

/// Enables QueryInterface for Swift-implemented generic interfaces.
///
/// When a Swift collection (e.g. `ArrayVector<Any?>`) is passed to WinRT as `IInspectable`,
/// the C++ side may QI for `IVector<IInspectable>` or `IIterable<IInspectable>`. The
/// type-erased `AnyWrapper` has no knowledge of these generic interfaces, so this registry
/// maps interface IIDs to factory closures that can wrap the Swift object appropriately.
///
@_spi(WinRTInternal)
public enum GenericInterfaceRegistry {
    private static var factories: [WindowsFoundation.IID: (AnyObject) -> IUnknownRef?] = [:]
    private static var lock = SRWLock()

    public struct Registrar: ~Copyable {
        fileprivate init() {}

        public func register(_ iid: WindowsFoundation.IID, _ factory: @escaping (AnyObject) -> IUnknownRef?) {
            GenericInterfaceRegistry.factories[iid] = factory
        }
    }

    /// Called by generated per-module code that looks like:
    ///
    ///     GenericInterfaceRegistry.registerBatch(count: 3) {
    ///         $0.register(IID___x_ABI_C__FIVector_1_IInspectable,
    ///             __x_ABI_C__FIVector_1_IInspectableBridge.factory)
    ///         ...
    ///     }
    ///
    public static func registerBatch(count: Int, _ body: (borrowing Registrar) -> Void) {
        lock.withLock(.exclusive) {
            factories.reserveCapacity(factories.count + count)
            body(Registrar())
        }
    }

    static func queryInterface(_ obj: AnyObject, _ iid: WindowsFoundation.IID) -> IUnknownRef? {
        lock.withLock(.shared) {
            factories[iid]?(obj)
        }
    }
}

extension AbiInterfaceBridge {
    /// Referenced as `ConcreteBridge.factory` in generated registration code.
    /// Using a static method ensures this is a fully-specialized function pointer
    /// with no closure context or heap allocation.
    @_spi(WinRTInternal)
    public static func factory(_ obj: AnyObject) -> IUnknownRef? {
        guard let typed = obj as? SwiftProjection else { return nil }
        return InterfaceWrapperBase<Self>(swiftImpl: typed)?.queryInterface(SwiftABI.IID)
    }
}
