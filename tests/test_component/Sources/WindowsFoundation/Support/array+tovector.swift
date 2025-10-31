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

    func first() -> AnyIIterator<T>? { ArrayIterator(storage) }

    func replaceAll(_ items: [T]) {
        storage = items
    }

    func getMany(_ startIndex: UInt32, _ items: inout [T]) -> UInt32 {
        let count = Swift.min(UInt32(storage.count) - startIndex, UInt32(items.count))
        for i in 0..<count {
            items[Int(i)] = storage[Int(startIndex + i)]
        }
        return count
    }
}

extension ArrayVector where T: Equatable {
    func indexOf(_ item: T, _ index: inout UInt32) throws -> Bool  {
        guard let foundIndex = storage.firstIndex(of: item) else { return false }
        index = UInt32(foundIndex)
        return true
    }
}

extension ArrayVector {
    public func queryInterface(_ iid: WindowsFoundation.IID) -> IUnknownRef? { nil }
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

    func first() -> AnyIIterator<T>? { ArrayIterator(storage) }

    func getMany(_ startIndex: UInt32, _ items: inout [T]) -> UInt32 {
        let count = Swift.min(UInt32(storage.count) - startIndex, UInt32(items.count))
        for i in 0..<count {
            items[Int(i)] = storage[Int(startIndex + i)]
        }
        return count
    }
}

extension ArrayVectorView where T: Equatable {
    func indexOf(_ item: T, _ index: inout UInt32) throws -> Bool  {
        guard let foundIndex = storage.firstIndex(of: item) else { return false }
        index = UInt32(foundIndex)
        return true
    }
}

extension ArrayVectorView {
    public func queryInterface(_ iid: WindowsFoundation.IID) -> IUnknownRef? { nil }
}

class ArrayIterator<T>: IIterator {
    typealias Element = T
    private let storage: Array<T>
    private var index: Int = 0
    init(_ storage: Array<T>){
        self.storage = storage
    }
    func moveNext() -> Bool { index += 1; return index < storage.count }
    var current: T { storage[index] }
    var hasCurrent: Bool { index < storage.count }

    func queryInterface(_ iid: IID) -> IUnknownRef? { nil }

    func getMany(_ items: inout [T]) -> UInt32 {
        let count = Swift.min(storage.count - index, items.count)
        for i in 0..<count {
            items[i] = storage[index + i]
        }
        return UInt32(count)
    }
}
