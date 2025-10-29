// WinRTDelegateBridge specifies the contract for bridging between WinRT and Swift for event handlers and d
public protocol WinRTDelegateBridge<Handler>: AbiInterfaceBridge where SwiftProjection == Handler {
    associatedtype Handler
}
