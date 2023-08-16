// Default Swift Collection protocol implementation for IVector
public extension IVector {
    var startIndex: Int { 0 }
    var endIndex: Int { count }
    var count: Int { Int(size) }
    
    subscript(position: Int) -> Element {
        get { getAt(UInt32(position)) }
        set(newValue) { setAt(UInt32(position), newValue) }
    }

    func index(after i: Int) -> Int { i+1 }
    func removeLast() { removeAtEnd()}

    func index(of: Element) -> Int? { 
        var index: UInt32 = 0
        let result = indexOf(of, &index)
        guard result else { return nil }
        return Int(index)
    }

    func remove(at: Int) -> Element {
        let item = self[at]
        removeAt(UInt32(at))
        return item   
    }
}

public extension IVectorView {
    var startIndex: Int { 0 }
    var endIndex: Int { count }
    var count: Int { Int(size) }

    func index(after i: Int) -> Int { i+1}
    subscript(position: Int) -> Element { getAt(UInt32(position)) }

    func index(of: Element) -> Int? { 
        var index: UInt32 = 0
        let result = indexOf(of, &index)
        guard result else { return nil }
        return Int(index)
    }
}