import test_component

protocol BindableVectorBase: IBindableVector, INotifyCollectionChanged {}


public class BindableVector<Element> : IVector, BindableVectorBase {
    private var storage: Array<Element>

    public init(_ elements: Array<Element>){
        self.storage = elements
    }

    public init() {
        self.storage = []
    }

    // MARK: Collection
    public var startIndex: Int { 0 }
    public var endIndex: Int { Int(size) }
    public func index(after i: Int) -> Int {
        i+1
    }
    public func index(of: Element) -> Int? { 
        var index: UInt32 = 0
        let result = indexOf(of, &index)
        guard result else { return nil }
        return Int(index)
    }
    public var count: Int { Int(size) }

    public subscript(position: Int) -> Element {
        get {
            getAt(UInt32(position))
        }
        set(newValue) {
            setAt(UInt32(position), newValue)
        }
    }
    // MARK: IVector
    public func getAt(_ index: UInt32) -> Element { storage[Int(index)] }
    public var size : UInt32 { UInt32(storage.count) }
    public func indexOf(_ item: Element, _ index: inout UInt32) -> Bool  { return false }
    public func append(_ item: Element) { 
        storage.append(item)
        _collectionChanged.invoke(self, argsFor(.add(index: size - 1, newItem: item)))
    }

    public func setAt(_ index: UInt32, _ item: Element) {
        storage[Int(index)] = item

    }
    public func insertAt(_ index: UInt32, _ item: Element) { 
        storage.insert(item, at: Int(index))
        _collectionChanged.invoke(self, argsFor(.add(index: index, newItem: item)))
    }
    public func removeAt(_ index: UInt32) {
        storage.remove(at: Int(index) )
    }
    public func removeAtEnd() { 
        storage.removeLast()
    }
    public func clear() { 
        let oldItems = storage
        storage.removeAll()
    }
    public func getView() -> AnyIVectorView<Element>? { return nil }

    // MARK: IBindableVector
    public func getAt(_ index: UInt32) throws -> Any! {
        return (self as AnyIVector<Element>).getAt(index)
    }

    // MARK: INotifyCollectionChanged
    @EventSource<NotifyCollectionChangedEventHandler> public var collectionChanged
}

public extension BindableVector where Element: Equatable {
    func indexOf(_ item: Element, _ index: inout UInt32) throws -> Bool  {
        guard let foundIndex = storage.firstIndex(of: item) else { return false }
        index = UInt32(foundIndex)
        return true
    }
}


enum CollectionChangedAction<T> {
    case add(index: UInt32, newItem: T)
}
extension BindableVector {
    func argsFor(_ action: CollectionChangedAction<Element>) -> NotifyCollectionChangedEventArgs {
        switch action {
        case .add(let index, let newItem):
            return NotifyCollectionChangedEventArgs(.add, BindableVector([newItem]), Int32(index))
        }
    }
}
