import test_component

class BindableVectorView<Element> : IVectorView, IBindableVectorView {
    private var storage: Array<Element>
    internal init(_ storage: Array<Element>){
        self.storage = storage
    }
    func getAt(_ index: UInt32) -> Element { storage[Int(index)] }
    var size : UInt32 { UInt32(storage.count) }
    func indexOf(_ item: Element, _ index: inout UInt32) -> Bool  { return false }

    // MARK: Collection
    var startIndex: Int { 0 }
    var endIndex: Int { Int(size) }
    func index(after i: Int) -> Int {
        i+1
    }

    func index(of: Element) -> Int? { 
        var index: UInt32 = 0
        let result = indexOf(of, &index)
        guard result else { return nil }
        return Int(index)
    }
    var count: Int { Int(size) }

    subscript(position: Int) -> Element {
        get {
            getAt(UInt32(position))
        }
    }

     // MARK: IBindableVectorView
    func getAt(_ index: UInt32) throws -> Any! {
        return storage[Int(index)]
    }

    func indexOf(_ value: Any!, _ index: inout UInt32) throws -> Bool {
        return (self as AnyIVectorView<Element>).indexOf(value as! Element, &index)   
    }
    
    func first() throws -> test_component.AnyIBindableIterator! {
        BindableIterator(self)
    }
}

extension BindableVectorView where Element: Equatable {
    func indexOf(_ item: Element, _ index: inout UInt32) throws -> Bool  {
        guard let foundIndex = storage.firstIndex(of: item) else { return false }
        index = UInt32(foundIndex)
        return true
    }
}

extension BindableVectorView {
  func makeAbi() -> test_component.IInspectable { 
    let wrapper = __ABI_Windows_UI_Xaml_Interop.IBindableVectorViewWrapper(self)
    let abi = try! wrapper?.toABI { $0 }
    return .init(abi!)
  }
}


