// Swift Dictionary-like extensions to IMap[View]
extension IMap {
    public var count: Int { Int(Size) }
    public var underestimatedCount: Int { Int(Size) }
    public var isEmpty: Bool { Size == 0 }

    public subscript(key: Key) -> Value? {
        get { HasKey(key) ? Lookup(key) : nil }
        set(newValue) {
            if let value = newValue {
                Insert(key, value)
            }
            else {
                Remove(key)
            }
        }
    }

    public subscript(key: Key, default defaultValue: @autoclosure () -> Value) -> Value {
        get { HasKey(key) ? Lookup(key) : defaultValue() }
    }

    @discardableResult
    public func updateValue(_ value: Value, forKey key: Key) -> Value? {
        let oldValue = HasKey(key) ? Lookup(key) : nil
        Insert(key, value)
        return oldValue
    }

    @discardableResult
    public func removeValue(forKey key: Key) -> Value? {
        let oldValue = HasKey(key) ? Lookup(key) : nil
        Remove(key)
        return oldValue
    }

    public func removeAll() { Clear() }
}

extension IMapView {
    public var count: Int { Int(Size) }
    public var underestimatedCount: Int { Int(Size) }
    public var isEmpty: Bool { Size == 0 }
    
    public subscript(key: Key) -> Value? {
        HasKey(key) ? Lookup(key) : nil
    }

    public subscript(key: Key, default defaultValue: @autoclosure () -> Value) -> Value {
        get { HasKey(key) ? Lookup(key) : defaultValue() }
    }
}