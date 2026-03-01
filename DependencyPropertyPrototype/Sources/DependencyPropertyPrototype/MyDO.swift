// A simple DependencyObject that has two properties. This would reflect the codegenerated to handle
// this type and properties associated with it

public class MyDO: FrameworkElement {
    
    override init(fromABi: DOAbi) {
        let abi = fromABi as! MyDOABI
        self._abi = abi

        // We have to initialize the _myProperty PropertyWrapper in the initializer since it needs a reference to the ABI.
        // This can't be done in the 
        self._myProperty = .init(MyDOProperties.myPropertyPublisher(abi))
        self._myIntProperty = .init(MyDOProperties.myIntPropertyPublisher(abi))
        super.init(fromABi: abi)
    }

    public convenience init() {
        let abi = MyDOABI()
        self.init(fromABi: abi)
    }
  
    private let _abi: MyDOABI

    @DependencyProperty var myProperty: String
    @DependencyProperty var myIntProperty: Int
}

fileprivate extension MyDO {
    struct MyDOProperties {
        static let myPropertyProperty = DependencyPropertyHandle()
        static let myPropertyPublisher = { (owner: MyDOABI) in DependencyPropertyChangedProxyImpl(
            property: myPropertyProperty,
            owner: owner,
            set: { $0.setMyProperty($1) }, 
            get: { $0.getMyProperty() }
        )}

        static let myIntPropertyProperty = DependencyPropertyHandle()
        static let myIntPropertyPublisher = { (owner: MyDOABI) in DependencyPropertyChangedProxyImpl(
            property: myPropertyProperty,
            owner: owner,
            set: { $0.setMyIntProperty($1) }, 
            get: { $0.getMyIntProperty() }
        )}
    }
}

public extension DependencyProperties where Type: MyDO {
    static var myProperty: DependencyProperties<MyDO, String> { .init(handle: MyDO.MyDOProperties.myPropertyProperty) }
    static var myIntProperty: DependencyProperties<MyDO, Int> { .init(handle: MyDO.MyDOProperties.myIntPropertyProperty) }
}
