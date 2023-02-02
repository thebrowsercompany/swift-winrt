// Default Swift Collection protocol implementation for IVector
public extension IVector {
    var startIndex: Int { 0 }
    var endIndex: Int { count }
    var count: Int { Int(Size) }
    
    subscript(position: Int) -> Element {
        get { GetAt(UInt32(position)) }
        set(newValue) { SetAt(UInt32(position), newValue) }
    }

    func index(after i: Int) -> Int { i+1 }
    func append(_ item: Element) { Append(item) }
    func removeLast() { RemoveAtEnd()}
    func clear() { Clear() }

    func index(of: Element) -> Int? { 
        var index: UInt32 = 0
        let result = IndexOf(of, &index)
        guard result else { return nil }
        return Int(index)
    }

    func remove(at: Int) -> Element {
        let item = self[at]
        RemoveAt(UInt32(at))
        return item   
    }
}

public extension IVectorView {
    var startIndex: Int { 0 }
    var endIndex: Int { count }
    var count: Int { Int(Size) }

    func index(after i: Int) -> Int { i+1}
    subscript(position: Int) -> Element { GetAt(UInt32(position)) }

    func index(of: Element) -> Int? { 
        var index: UInt32 = 0
        let result = IndexOf(of, &index)
        guard result else { return nil }
        return Int(index)
    }
}