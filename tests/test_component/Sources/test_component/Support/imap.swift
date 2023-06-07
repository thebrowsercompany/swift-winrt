import WinSDK
import Ctest_component

public protocol IMap<Key, Value> : WinRTInterface {
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

public protocol IMapView<Key, Value> : WinRTInterface {
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

// No default implementation for WinRTInterface, this means that
// app implemented vectors can't be passed to an API that expects
// Any yet
extension IMap {
  public func makeAbi() -> test_component.IInspectable { 
    fatalError("not implemented")
  }
}

extension IMapView {
  public func makeAbi() -> test_component.IInspectable { 
    fatalError("not implemented")
  }
}