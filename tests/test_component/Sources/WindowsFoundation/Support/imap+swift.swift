// Swift Dictionary-like extensions to IMap[View]
extension IMap {
    public typealias Key = K
    public typealias Value = V

    public var count: Int { Int(size) }
    public var underestimatedCount: Int { Int(size) }
    public var isEmpty: Bool { size == 0 }

    public subscript(key: Key) -> Value? {
        get { hasKey(key) ? lookup(key) : nil }
        set(newValue) {
            if let value = newValue {
                _ = insert(key, value)
            }
            else {
                remove(key)
            }
        }
    }

    public subscript(key: Key, default defaultValue: @autoclosure () -> Value) -> Value {
        get { hasKey(key) ? lookup(key) : defaultValue() }
    }

    @discardableResult
    public func updateValue(_ value: Value, forKey key: Key) -> Value? {
        let oldValue = hasKey(key) ? lookup(key) : nil
        _ = insert(key, value)
        return oldValue
    }

    @discardableResult
    public func removeValue(forKey key: Key) -> Value? {
        let oldValue = hasKey(key) ? lookup(key) : nil
        remove(key)
        return oldValue
    }

    public func removeAll() { clear() }
}

extension IMapView {
    public typealias Key = K
    public typealias Value = V

    public var count: Int { Int(size) }
    public var underestimatedCount: Int { Int(size) }
    public var isEmpty: Bool { size == 0 }
    
    public subscript(key: Key) -> Value? {
        hasKey(key) ? lookup(key) : nil
    }

    public subscript(key: Key, default defaultValue: @autoclosure () -> Value) -> Value {
        get { hasKey(key) ? lookup(key) : defaultValue() }
    }
}