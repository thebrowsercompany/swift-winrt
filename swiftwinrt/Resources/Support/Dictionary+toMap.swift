// Swift Dictionary to IMap Adaptor
extension Dictionary {
    public func toMap() -> any IMap<Key, Value> {
        DictionaryMap(self)
    }
}

internal class DictionaryMap<Key, Value> : IMap where Key : Hashable {
    private var storage: Dictionary<Key, Value>
    internal init(_ storage: Dictionary<Key, Value>){
        self.storage = storage
    }
    
    var Size : UInt32 { UInt32(storage.count) }
    func HasKey(_ key: Key) -> Bool { storage[key] != nil }
    func Lookup(_ key: Key) -> Value { storage[key]! }
    func GetView() -> (any IMapView<Key, Value>)? { DictionaryMapView(storage) }

    @discardableResult func Insert(_ key: Key, _ value: Value) -> Bool {
        // WinRT returns true if replacing
        storage.updateValue(value, forKey: key) != nil
    }
    func Remove(_ key: Key) { storage.removeValue(forKey: key) }
    func Clear() { storage.removeAll(keepingCapacity: true) }
}

internal class DictionaryMapView<Key, Value> : IMapView where Key : Hashable {
    private var storage: Dictionary<Key, Value>
    internal init(_ storage: Dictionary<Key, Value>){
        self.storage = storage
    }
    
    var Size : UInt32 { UInt32(storage.count) }
    func HasKey(_ key: Key) -> Bool { storage[key] != nil }
    func Lookup(_ key: Key) -> Value { storage[key]! }
    func Split(
        _ first: inout (any IMapView<Key, Value>)?,
        _ second: inout (any IMapView<Key, Value>)?) {
        fatalError("Not implemented: DictionaryMapView.Split")
    }
}