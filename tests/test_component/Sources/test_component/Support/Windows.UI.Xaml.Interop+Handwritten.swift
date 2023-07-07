// This is put into the "Support" folder so that it is picked up by the cmake build

import Ctest_component
// swiftlint:disable all
// swiftformat:disable:all

// https://linear.app/the-browser-company/issue/WIN-115/swiftwinrt-support-constructor-parameters-for-composable-types
class NotifyCollectionChangedEventArgsComposableActivationFactoryWithArgs: ComposableActivationFactory {
    typealias Composable = __ABI_Windows_UI_Xaml_Interop.INotifyCollectionChangedEventArgsFactory.Composable
    let _INotifyCollectionChangedEventArgsFactory: __ABI_Windows_UI_Xaml_Interop.INotifyCollectionChangedEventArgsFactory = try! RoGetActivationFactory(HString("Windows.UI.Xaml.Interop.NotifyCollectionChangedEventArgs"))
    private let action: NotifyCollectionChangedAction
    private let newItems: AnyIBindableVector?
    private let oldItems: AnyIBindableVector?
    private let newIndex: Int32
    private let oldIndex: Int32
    init(_ action: NotifyCollectionChangedAction, _ newItems: AnyIBindableVector?, _ oldItems: AnyIBindableVector?, _ newIndex: Int32, _ oldIndex: Int32) {
        self.action = action
        self.newItems = newItems
        self.oldItems = oldItems
        self.newIndex = newIndex
        self.oldIndex = oldIndex
    }

    internal func CreateInstanceImpl(
        _ baseInterface: UnsafeMutablePointer<Ctest_component.IInspectable>?,
        _ innerInterface: inout UnsafeMutablePointer<Ctest_component.IInspectable>?
    ) throws -> UnsafeMutablePointer<__x_ABI_CWindows_CUI_CXaml_CInterop_CINotifyCollectionChangedEventArgs>? {
        let newItemsWrapper = __ABI_Windows_UI_Xaml_Interop.IBindableVectorWrapper(newItems)
        let oldItemsWrapper = __ABI_Windows_UI_Xaml_Interop.IBindableVectorWrapper(oldItems)

        let newItemsAbi = try! newItemsWrapper?.toABI() { $0 }
        let oldItemsAbi = try! oldItemsWrapper?.toABI() { $0 }

        var value: UnsafeMutablePointer<__x_ABI_CWindows_CUI_CXaml_CInterop_CINotifyCollectionChangedEventArgs>?
        _ = try _INotifyCollectionChangedEventArgsFactory.perform(as: __x_ABI_CWindows_CUI_CXaml_CInterop_CINotifyCollectionChangedEventArgsFactory.self) { pThis in
            try CHECKED(pThis.pointee.lpVtbl.pointee.CreateInstanceWithAllParameters(pThis, self.action, newItemsAbi, oldItemsAbi, self.newIndex, self.oldIndex, baseInterface, &innerInterface, &value))
        }
        return value
    }
}

extension NotifyCollectionChangedEventArgs {
    public convenience init(_ action: NotifyCollectionChangedAction, newItems: AnyIBindableVector?, oldItems: AnyIBindableVector?, newIndex: Int32, oldIndex: Int32) {
        let factory = NotifyCollectionChangedEventArgsComposableActivationFactoryWithArgs(action, newItems, oldItems, newIndex, oldIndex)
        self.init(factory)
    }
}
// swiftlint:enable all
// swiftformat:enable:all
