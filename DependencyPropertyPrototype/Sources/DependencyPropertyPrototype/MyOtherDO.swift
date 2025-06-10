// Another DO implementation to show how styles are enforced by the type system. This uses
// the same underlying ABI type for simplicity of the example.
public class MyOtherDO: DependencyObject {
    
    override init(fromABi: DOAbi) {
        let abi = fromABi as! MyDOABI
        self._abi = abi

        // We have to initialize the _myProperty PropertyWrapper in the initializer since it needs a reference to the ABI.
        // This can't be done in the 
        self._myStringProperty = .init(MyOtherDOProperties.myPropertyPublisher(abi))
        self._myOtherIntProperty = .init(MyOtherDOProperties.myIntPropertyPublisher(abi))
        super.init(fromABi: abi)
    }

    public convenience init() {
        let abi = MyDOABI()
        self.init(fromABi: abi)
    }
  
    private let _abi: MyDOABI

    @DependencyProperty var myStringProperty: String
    @DependencyProperty var myOtherIntProperty: Int
}

fileprivate extension MyOtherDO {
    struct MyOtherDOProperties {
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

public extension DependencyProperties where Type: MyOtherDO {
    static var myStringProperty: DependencyProperties<MyOtherDO, String> { .init(handle: MyOtherDO.MyOtherDOProperties.myPropertyProperty) }
    static var myOtherIntProperty: DependencyProperties<MyOtherDO, Int> { .init(handle: MyOtherDO.MyOtherDOProperties.myIntPropertyProperty) }
}