// WARNING: Please don't edit this file. It was generated by Swift/WinRT v0.0.1
import Ctest_component

public typealias NotifyCollectionChangedAction = __x_ABI_CMicrosoft_CUI_CXaml_CInterop_CNotifyCollectionChangedAction
public final class NotifyCollectionChangedEventArgs : WinRTClass {
    private typealias SwiftABI = __ABI_Microsoft_UI_Xaml_Interop.INotifyCollectionChangedEventArgs
    private typealias CABI = __x_ABI_CMicrosoft_CUI_CXaml_CInterop_CINotifyCollectionChangedEventArgs
    private var _default: SwiftABI!
    public func _getABI<T>() -> UnsafeMutablePointer<T>? {
        if T.self == CABI.self {
            return RawPointer(_default)
        }   
        if T.self == Ctest_component.IInspectable.self {
            return RawPointer(_default)
        }
        return nil
    }

    public var thisPtr: test_component.IInspectable { _default }

    public static func from(abi: UnsafeMutablePointer<__x_ABI_CMicrosoft_CUI_CXaml_CInterop_CINotifyCollectionChangedEventArgs>?) -> NotifyCollectionChangedEventArgs? {
        guard let abi = abi else { return nil }
        return .init(fromAbi: .init(abi))
    }

    public init(fromAbi: test_component.IInspectable) {
        _default = try! fromAbi.QueryInterface()
    }

    private static let _INotifyCollectionChangedEventArgsFactory: __ABI_Microsoft_UI_Xaml_Interop.INotifyCollectionChangedEventArgsFactory = try! RoGetActivationFactory(HString("Microsoft.UI.Xaml.Interop.NotifyCollectionChangedEventArgs"))
    public init(_ action: NotifyCollectionChangedAction, _ newItems: AnyIBindableVector!, _ newStartingIndex: Int32) {
        let newItemsWrapper = __ABI_Microsoft_UI_Xaml_Interop.IBindableVectorWrapper(newItems)
        let _newItems = try! newItemsWrapper?.toABI { $0 }
        let value = try! Self._INotifyCollectionChangedEventArgsFactory.CreateInstanceImpl(action, _newItems, newStartingIndex)
        _default = __ABI_Microsoft_UI_Xaml_Interop.INotifyCollectionChangedEventArgs(consuming: value!)
    }

    public var action : NotifyCollectionChangedAction {
        get {
            let value = try! _default.get_ActionImpl()
            return value
        }

    }

    public var newItems : AnyIBindableVector! {
        get {
            let value = try! _default.get_NewItemsImpl()
            return __ABI_Microsoft_UI_Xaml_Interop.IBindableVectorWrapper.unwrapFrom(abi: value)
        }

    }

    public var newStartingIndex : Int32 {
        get {
            let value = try! _default.get_NewStartingIndexImpl()
            return value
        }

    }

}

public typealias NotifyCollectionChangedEventHandler = (Any?, NotifyCollectionChangedEventArgs?) -> ()
public protocol IBindableVector : WinRTInterface {
    func getAt(_ index: UInt32) throws -> Any!
    var size: UInt32 { get }
}

extension IBindableVector {
    public func getAbiMaker() -> () -> UnsafeMutablePointer<Ctest_component.IInspectable> {
        let wrapper = __ABI_Microsoft_UI_Xaml_Interop.IBindableVectorWrapper(self)
        return { try! wrapper!.toABI { $0.withMemoryRebound(to: Ctest_component.IInspectable.self, capacity: 1) { $0 } } } 
    }
}
public typealias AnyIBindableVector = any IBindableVector

public protocol INotifyCollectionChanged : WinRTInterface {
    var collectionChanged: Event<NotifyCollectionChangedEventHandler> { get }
}

public extension EventSource where Handler == NotifyCollectionChangedEventHandler {
    func invoke(_ sender: Any!, _ args: NotifyCollectionChangedEventArgs!) {
        for handler in getInvocationList() {
            handler(sender, args)
        }
    }
}

extension INotifyCollectionChanged {
    public func getAbiMaker() -> () -> UnsafeMutablePointer<Ctest_component.IInspectable> {
        let wrapper = __ABI_Microsoft_UI_Xaml_Interop.INotifyCollectionChangedWrapper(self)
        return { try! wrapper!.toABI { $0.withMemoryRebound(to: Ctest_component.IInspectable.self, capacity: 1) { $0 } } } 
    }
}
public typealias AnyINotifyCollectionChanged = any INotifyCollectionChanged

extension test_component.NotifyCollectionChangedAction {
    public static var add : test_component.NotifyCollectionChangedAction {
        __x_ABI_CMicrosoft_CUI_CXaml_CInterop_CNotifyCollectionChangedAction_Add
    }
}
extension test_component.NotifyCollectionChangedAction: Hashable, Codable {}
