import WinSDK
import Ctest_component

public protocol IMap<Key, Value> : IWinRTObject {
    associatedtype Key
    associatedtype Value

    var Size : UInt32 { get }
    func HasKey(_ key: Key) -> Bool
    func Lookup(_ key: Key) -> Value
    func GetView() -> any IMapView<Key, Value>

    @discardableResult func Insert(_ key: Key, _ value: Value) -> Bool
    func Remove(_ key: Key)
    func Clear()
}   

public protocol IMapView<Key, Value> : IWinRTObject {
    associatedtype Key
    associatedtype Value
    
    var Size : UInt32 { get }
    func HasKey(_ key: Key) -> Bool 
    func Lookup(_ key: Key) -> Value 
    func Split(
        _ first: inout (any IMapView<Key, Value>)?,
        _ second: inout (any IMapView<Key, Value>)?)
}