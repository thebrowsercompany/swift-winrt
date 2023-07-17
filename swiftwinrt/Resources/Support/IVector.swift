import WinSDK
import C_BINDINGS_MODULE

// This protocol defines the storage mechanism for the vector. The vector and it's content could 
// either be created in Swift and passed to the WinRT ABI, or it's created from the WinRT ABI
// and manipulated from Swift. For code simplicity, we model this the same way we would model
// a WinRT interface definition of IVector (i.e. non-throwing and using the WinRT API names).
public protocol IVector<Element> : Collection where Index == Int {
    associatedtype Element
    // MARK: WinRT APIs
    func getAt(_ index: UInt32) -> Element 
    var size: UInt32 { get }
    func indexOf(_ item: Element, _ index: inout UInt32) -> Bool
    func append(_ item: Element)
    func setAt(_ index: UInt32, _ item: Element)
    func insertAt(_ index: UInt32, _ item: Element)
    func removeAt(_ index: UInt32)
    func removeAtEnd()
    func clear()
    func getView() -> AnyIVectorView<Element>?
}

public protocol IVectorView<Element> : Collection where Index == Int {
    associatedtype Element
    // MARK: WinRT APIs
    func getAt(_ index: UInt32) -> Element 
    var size: UInt32 { get }
    func indexOf(_ item: Element, _ index: inout UInt32) -> Bool
}

public typealias AnyIVector<Element> = any IVector<Element>
public typealias AnyIVectorView<Element> = any IVectorView<Element>
