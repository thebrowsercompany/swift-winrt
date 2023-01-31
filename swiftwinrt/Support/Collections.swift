import WinSDK
import C_BINDINGS_MODULE

public protocol WinRTVector : IVector, AbiInterfaceImpl where swift_Projection == any IVector<Element> {}
public protocol WinRTVectorView : IVectorView, AbiInterfaceImpl where swift_Projection == any IVectorView<Element> {}

// This protocol defines the storage mechanism for the vector. The vector and it's content could 
// either be created in Swift and passed to the WinRT ABI, or it's created from the WinRT ABI
// and manipulated from Swift. For code simplicity, we model this the same way we would model
// a WinRT interface definition of IVector (i.e. non-throwing and using the WinRT API names).
public protocol IVector<Element> : IVectorView {
    associatedtype Element
    // MARK: WinRT APIs
    func Append(_ item: Element)
    func SetAt(_ index: UInt32, _ item: Element)
    func InsertAt(_ index: UInt32, _ item: Element)
    func RemoveAt(_ index: UInt32)
    func RemoveAtEnd()
    func Clear()
    func GetView() -> any IVectorView<Element>
}   

public protocol IVectorView<Element> : IWinRTObject, Collection where Index == Int {
    associatedtype Element
    // MARK: WinRT APIs
    func GetAt(_ index: UInt32) -> Element 
    var Size: UInt32 { get }
    func IndexOf(_ item: Element, _ index: inout UInt32) -> Bool
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
    func GetView() -> any IVectorView<Element> { return self }
}

extension ArrayVector where Element: Equatable {
    func IndexOf(_ item: Element, _ index: inout UInt32) throws -> Bool  {
        guard let foundIndex = storage.firstIndex(of: item) else { return false }
        index = UInt32(foundIndex)
        return true
    }
}

extension IVector {
    public func append(_ item: Element) {
        Append(item)
    }

    public subscript(position: Index) -> Element {
        get {
            GetAt(UInt32(position))
         }
         set(newValue) {
             SetAt(UInt32(position), newValue)
         }
    }

    public func removeLast() {
        RemoveAtEnd()
    }

    public func clear() {
        Clear()
    }
}

extension IVectorView {
    public var startIndex: Index { 0 }
    public var endIndex: Index { Index(Size) }
    public func index(after i: Index) -> Index {
        i+1
    }

    public subscript(position: Index) -> Element {
        get {
            GetAt(UInt32(position))
         }
    }

    public func index(of: Element) -> Int? { 
        var index: UInt32 = 0
        let result = IndexOf(of, &index)
        guard result else { return nil }
        return Int(index)
    }
    public var count: Int { Int(Size) }
}

extension Array {
    public func toVector() -> any IVector<Element> {
        ArrayVector(self)
    }
}

