import WinSDK

// Swift Dictionary to IMap Adaptor
extension Dictionary {
    public func toMap() -> AnyIMap<Key, Value> {
        DictionaryMap(self)
    }
}

internal class DictionaryMap<K, V> : IMap where K : Hashable {
    typealias T = AnyIKeyValuePair<Key, Value>?
    private var storage: Dictionary<K, V>
    internal init(_ storage: Dictionary<K, V>){
        self.storage = storage
    }
    
    var size : UInt32 { UInt32(storage.count) }
    func hasKey(_ K: K) -> Bool { storage[K] != nil }
    func lookup(_ K: K) -> V { storage[K]! }
    func getView() -> AnyIMapView<K, V>? { DictionaryMapView(storage) }

    @discardableResult func insert(_ K: K, _ V: V) -> Bool {
        // WinRT returns true if replacing
        storage.updateValue(V, forKey: K) != nil
    }
    func remove(_ K: K) { storage.removeValue(forKey: K) }
    func clear() { storage.removeAll(keepingCapacity: true) }
    
    func first() -> AnyIIterator<T>? { fatalError("Not implemented: ArrayVector.First") }
}

extension DictionaryMap {
    public func queryInterface(_ iid: WindowsFoundation.IID) -> IUnknownRef? { nil }
}

internal class DictionaryMapView<K, V> : IMapView where K : Hashable {
    typealias T = AnyIKeyValuePair<Key, Value>?

    private var storage: Dictionary<K, V>
    internal init(_ storage: Dictionary<K, V>){
        self.storage = storage
    }
    
    var size : UInt32 { UInt32(storage.count) }
    func hasKey(_ key: K) -> Bool { storage[key] != nil }
    func lookup(_ key: K) -> V { storage[key]! }
    func split(
        _ first: inout AnyIMapView<K, V>?,
        _ second: inout AnyIMapView<K, V>?) {
        fatalError("Not implemented: DictionaryMapView.Split")
    }

    func first() -> AnyIIterator<T>? { fatalError("Not implemented: ArrayVector.First") }
}

extension DictionaryMapView {
    public func queryInterface(_ iid: WindowsFoundation.IID) -> IUnknownRef? { nil }
}