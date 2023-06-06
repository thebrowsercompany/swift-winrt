// Swift Dictionary to IMap Adaptor
extension Dictionary {
    public func toMap() -> AnyIMap<Key, Value> {
        DictionaryMap(self)
    }
}

internal class DictionaryMap<Key, Value> : IMap where Key : Hashable {
    private var storage: Dictionary<Key, Value>
    internal init(_ storage: Dictionary<Key, Value>){
        self.storage = storage
    }
    
    var size : UInt32 { UInt32(storage.count) }
    func hasKey(_ key: Key) -> Bool { storage[key] != nil }
    func lookup(_ key: Key) -> Value { storage[key]! }
    func getView() -> AnyIMapView<Key, Value>? { DictionaryMapView(storage) }

    @discardableResult func insert(_ key: Key, _ value: Value) -> Bool {
        // WinRT returns true if replacing
        storage.updateValue(value, forKey: key) != nil
    }
    func remove(_ key: Key) { storage.removeValue(forKey: key) }
    func clear() { storage.removeAll(keepingCapacity: true) }
}

extension DictionaryMap {
  var thisPtr: SUPPORT_MODULE.IInspectable { 
    get {
      fatalError("not implemented")
    }
  }
}

internal class DictionaryMapView<Key, Value> : IMapView where Key : Hashable {
    private var storage: Dictionary<Key, Value>
    internal init(_ storage: Dictionary<Key, Value>){
        self.storage = storage
    }
    
    var size : UInt32 { UInt32(storage.count) }
    func hasKey(_ key: Key) -> Bool { storage[key] != nil }
    func lookup(_ key: Key) -> Value { storage[key]! }
    func split(
        _ first: inout AnyIMapView<Key, Value>?,
        _ second: inout AnyIMapView<Key, Value>?) {
        fatalError("Not implemented: DictionaryMapView.Split")
    }
}

extension DictionaryMapView {
  var thisPtr: SUPPORT_MODULE.IInspectable { 
    get {
      fatalError("not implemented")
    }
  }
}