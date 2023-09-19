// Swift Array to IVector Adaptor
import WinSDK

extension Array {
    public func toVector() -> AnyIVector<Element> {
        ArrayVector(self)
    }
}

internal class ArrayVector<T> : IVector {
    typealias Element = T
    private var storage: Array<T>
    internal init(_ storage: Array<T>){
        self.storage = storage
    }

    // MARK: WinRT APIs
    func getAt(_ index: UInt32) -> T { storage[Int(index)] }
    var size : UInt32 { UInt32(storage.count) }
    func indexOf(_ item: T, _ index: inout UInt32) -> Bool  { return false }
    func append(_ item: T) { storage.append(item ) }
    func setAt(_ index: UInt32, _ item: T) { storage[Int(index)] = item }
    func insertAt(_ index: UInt32, _ item: T) { storage.insert(item, at: Int(index)) }
    func removeAt(_ index: UInt32) { storage.remove(at: Int(index) )}
    func removeAtEnd() { storage.removeLast() }
    func clear() { storage.removeAll() }
    func getView() -> AnyIVectorView<T>? { return ArrayVectorView(storage) }

    func first() -> AnyIIterator<T>? { fatalError("Not implemented: ArrayVector.First") }
}

extension ArrayVector where T: Equatable {
    func indexOf(_ item: T, _ index: inout UInt32) throws -> Bool  {
        guard let foundIndex = storage.firstIndex(of: item) else { return false }
        index = UInt32(foundIndex)
        return true
    }
}

extension ArrayVector {
    public func queryInterface(_ iid: test_component.IID) -> IUnknownRef? { nil }
}

internal class ArrayVectorView<T> : IVectorView {
    typealias Element = T
    private var storage: Array<T>
    internal init(_ storage: Array<T>){
        self.storage = storage
    }
    func getAt(_ index: UInt32) -> T { storage[Int(index)] }
    var size : UInt32 { UInt32(storage.count) }
    func indexOf(_ item: T, _ index: inout UInt32) -> Bool  { return false }

    func first() -> AnyIIterator<T>? { fatalError("Not implemented: ArrayVector.First") }
}

extension ArrayVectorView where T: Equatable {
    func indexOf(_ item: T, _ index: inout UInt32) throws -> Bool  {
        guard let foundIndex = storage.firstIndex(of: item) else { return false }
        index = UInt32(foundIndex)
        return true
    }
}

extension ArrayVectorView {
    public func queryInterface(_ iid: test_component.IID) -> IUnknownRef? { nil }
}