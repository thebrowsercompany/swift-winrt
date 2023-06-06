// Swift Array to IVector Adaptor
extension Array {
    public func toVector() -> AnyIVector<Element> {
        ArrayVector(self)
    }
}

internal class ArrayVector<Element> : IVector {
    private var storage: Array<Element>
    internal init(_ storage: Array<Element>){
        self.storage = storage
    }

    // MARK: WinRT APIs
    func getAt(_ index: UInt32) -> Element { storage[Int(index)] }
    var size : UInt32 { UInt32(storage.count) }
    func indexOf(_ item: Element, _ index: inout UInt32) -> Bool  { return false }
    func append(_ item: Element) { storage.append(item ) }
    func setAt(_ index: UInt32, _ item: Element) { storage[Int(index)] = item }
    func insertAt(_ index: UInt32, _ item: Element) { storage.insert(item, at: Int(index)) }
    func removeAt(_ index: UInt32) { storage.remove(at: Int(index) )}
    func removeAtEnd() { storage.removeLast() }
    func clear() { storage.removeAll() }
    func getView() -> AnyIVectorView<Element>? { return ArrayVectorView(storage) }
}

extension ArrayVector where Element: Equatable {
    func indexOf(_ item: Element, _ index: inout UInt32) throws -> Bool  {
        guard let foundIndex = storage.firstIndex(of: item) else { return false }
        index = UInt32(foundIndex)
        return true
    }
}

extension ArrayVector {
  var thisPtr: SUPPORT_MODULE.IInspectable { 
    get {
      fatalError("not implemented")
    }
  }
}

internal class ArrayVectorView<Element> : IVectorView {
    private var storage: Array<Element>
    internal init(_ storage: Array<Element>){
        self.storage = storage
    }
    func getAt(_ index: UInt32) -> Element { storage[Int(index)] }
    var size : UInt32 { UInt32(storage.count) }
    func indexOf(_ item: Element, _ index: inout UInt32) -> Bool  { return false }
}

extension ArrayVectorView where Element: Equatable {
    func indexOf(_ item: Element, _ index: inout UInt32) throws -> Bool  {
        guard let foundIndex = storage.firstIndex(of: item) else { return false }
        index = UInt32(foundIndex)
        return true
    }
}

extension ArrayVectorView {
  var thisPtr: SUPPORT_MODULE.IInspectable { 
    get {
      fatalError("not implemented")
    }
  }
}