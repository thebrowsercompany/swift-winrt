// A simple DependencyObject that has two properties. This would reflect the codegenerated to handle
// this type and properties associated with it

public class MyDO2: MyDO {
    
    override init(fromABi: DOAbi) {
        let abi = fromABi as! MyDO2ABI
        self._abi = abi

        // We have to initialize the _myProperty PropertyWrapper in the initializer since it needs a reference to the ABI.
        // This can't be done in the 
        self._myBoolProperty = .init(MyDO2Properties.myBoolPropertyPublisher(abi))
        super.init(fromABi: abi)
    }

    public convenience init() {
        let abi = MyDO2ABI()
        self.init(fromABi: abi)
    }
  
    private let _abi: MyDO2ABI

    @DependencyProperty var myBoolProperty: Bool
}

fileprivate extension MyDO2 {
    struct MyDO2Properties {
        static let myBoolPropertyProperty = DependencyPropertyHandle()
        static let myBoolPropertyPublisher = { (owner: MyDO2ABI) in DependencyPropertyChangedProxyImpl(
            property: myBoolPropertyProperty,
            owner: owner,
            set: { $0.setMyBoolProperty($1) }, 
            get: { $0.getMyBoolProperty() }
        )}
    }
}

public extension DependencyProperties where Type: MyDO2 {
    static var myBoolProperty: DependencyProperties<MyDO2, Bool> { .init(handle: MyDO2.MyDO2Properties.myBoolPropertyProperty) }
}
