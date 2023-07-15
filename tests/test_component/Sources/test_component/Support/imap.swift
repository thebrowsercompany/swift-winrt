import WinSDK
import Ctest_component

public protocol IMap<Key, Value> {
    associatedtype Key
    associatedtype Value

    var size : UInt32 { get }
    func hasKey(_ key: Key) -> Bool
    func lookup(_ key: Key) -> Value
    func getView() -> AnyIMapView<Key, Value>?

    @discardableResult func insert(_ key: Key, _ value: Value) -> Bool
    func remove(_ key: Key)
    func clear()
}

public protocol IMapView<Key, Value> {
    associatedtype Key
    associatedtype Value
    
    var size : UInt32 { get }
    func hasKey(_ key: Key) -> Bool 
    func lookup(_ key: Key) -> Value 
    func split(
        _ first: inout AnyIMapView<Key, Value>?,
        _ second: inout AnyIMapView<Key, Value>?)
}

public typealias AnyIMap<Key, Value> = any IMap<Key, Value>
public typealias AnyIMapView<Key, Value> = any IMapView<Key, Value>
