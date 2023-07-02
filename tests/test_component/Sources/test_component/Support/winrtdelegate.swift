// WinRTDelegateBridge specifies the contract for bridging between WinRT and Swift for event handlers and d
public protocol WinRTDelegateBridge<Handler>: AbiInterfaceImpl where SwiftProjection == Handler {
    associatedtype Handler
}

// The WinRTDelegateBridge doesn't actually hold a pointer to the SwiftABI, 
// rather the handler it creates and gives to the Swift holder keeps a strong
// reference to the SwiftABI object, which keeps the handler alive. 
public extension WinRTDelegateBridge {
    var _default: SwiftABI { fatalError("_default should not be accessed on a WinRTDelegateBridge") }
}
