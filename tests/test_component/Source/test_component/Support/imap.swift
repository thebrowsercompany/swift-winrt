import WinSDK
import Ctest_component

public protocol IMap<Key, Value> : IWinRTObject {
    associatedtype Key
    associatedtype Value

    var size : UInt32 { get }
    func hasKey(_ key: Key) -> Bool
    func lookup(_ key: Key) -> Value
    func getView() -> (any IMapView<Key, Value>)?

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
        _ first: inout (any IMapView<Key, Value>)?,
        _ second: inout (any IMapView<Key, Value>)?)
}