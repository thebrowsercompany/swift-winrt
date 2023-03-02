// Swift Array to IVector Adaptor
extension Array {
    public func toVector() -> any IVector<Element> {
        ArrayVector(self)
    }
}

internal class ArrayVector<Element> : IVector {
    private var storage: Array<Element>
    internal init(_ storage: Array<Element>){
        self.storage = storage
    }

    // MARK: WinRT APIs
    func GetAt(_ index: UInt32) -> Element { storage[Int(index)] }
    var Size : UInt32 { UInt32(storage.count) }
    func IndexOf(_ item: Element, _ index: inout UInt32) -> Bool  { return false }
    func Append(_ item: Element) { storage.append(item ) }
    func SetAt(_ index: UInt32, _ item: Element) { storage[Int(index)] = item }
    func InsertAt(_ index: UInt32, _ item: Element) { storage.insert(item, at: Int(index)) }
    func RemoveAt(_ index: UInt32) { storage.remove(at: Int(index) )}
    func RemoveAtEnd() { storage.removeLast() }
    func Clear() { storage.removeAll() }
    func GetView() -> (any IVectorView<Element>)? { return ArrayVectorView(storage) }
}

extension ArrayVector where Element: Equatable {
    func IndexOf(_ item: Element, _ index: inout UInt32) throws -> Bool  {
        guard let foundIndex = storage.firstIndex(of: item) else { return false }
        index = UInt32(foundIndex)
        return true
    }
}

internal class ArrayVectorView<Element> : IVectorView {
    private var storage: Array<Element>
    internal init(_ storage: Array<Element>){
        self.storage = storage
    }
    func GetAt(_ index: UInt32) -> Element { storage[Int(index)] }
    var Size : UInt32 { UInt32(storage.count) }
    func IndexOf(_ item: Element, _ index: inout UInt32) -> Bool  { return false }
}

extension ArrayVectorView where Element: Equatable {
    func IndexOf(_ item: Element, _ index: inout UInt32) throws -> Bool  {
        guard let foundIndex = storage.firstIndex(of: item) else { return false }
        index = UInt32(foundIndex)
        return true
    }
}