import WinSDK
import C_BINDINGS_MODULE

// This protocol defines the storage mechanism for the vector. The vector and it's content could 
// either be created in Swift and passed to the WinRT ABI, or it's created from the WinRT ABI
// and manipulated from Swift. For code simplicity, we model this the same way we would model
// a WinRT interface definition of IVector (i.e. non-throwing and using the WinRT API names).
public protocol IVector<Element> : IWinRTObject, Collection where Index == Int {
    associatedtype Element
    // MARK: WinRT APIs
    func GetAt(_ index: UInt32) -> Element 
    var Size: UInt32 { get }
    func IndexOf(_ item: Element, _ index: inout UInt32) -> Bool
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