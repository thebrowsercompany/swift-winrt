import WinSDK
import C_BINDINGS_MODULE

public protocol IMap<Key, Value> : IWinRTObject {
    associatedtype Key
    associatedtype Value

    var size : UInt32 { get }
    func hasKey(_ key: Key) -> Bool
    func lookup(_ key: Key) -> Value
    func getView() -> AnyMapView<Key, Value>?

    @discardableResult func insert(_ key: Key, _ value: Value) -> Bool
    func remove(_ key: Key)
    func clear()
}

public protocol IMapView<Key, Value> : IWinRTObject {
    associatedtype Key
    associatedtype Value
    
    var size : UInt32 { get }
    func hasKey(_ key: Key) -> Bool 
    func lookup(_ key: Key) -> Value 
    func split(
        _ first: inout AnyMapView<Key, Value>?,
        _ second: inout AnyMapView<Key, Value>?)
}

public typealias AnyMap<Key, Value> = any IMap<Key, Value>
public typealias AnyMapView<Key, Value> = any IMapView<Key, Value>
