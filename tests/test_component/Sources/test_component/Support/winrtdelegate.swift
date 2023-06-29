// WinRTDelegate defines the object which holds onto a swift event handler when it is passed to a WinRT object.
// This object is wrapped via the DelegateWrapperBase class.
public protocol WinRTDelegate : AnyObject {
    associatedtype Data
    associatedtype Return
    init(handler: @escaping (Data) -> Return)
    var handler: (Data) -> Return { get }
}
public typealias AnyWinRTDelegate = any WinRTDelegate

// WinRTDelegateBridge specifies the contract for bridging between WinRT and Swift for event handlers and d
public protocol WinRTDelegateBridge<Data, Return>: AbiInterfaceImpl, WinRTDelegate where SwiftProjection == (Data) -> Return {
}

// The WinRTDelegateBridge doesn't actually hold a pointer to the SwiftABI, 
// rather the handler it creates and gives to the Swift holder keeps a strong
// reference to the SwiftABI object, which keeps the handler alive. 
public extension WinRTDelegateBridge {
    var _default: SwiftABI { fatalError("_default should not be accessed on a WinRTDelegateBridge") }
}
