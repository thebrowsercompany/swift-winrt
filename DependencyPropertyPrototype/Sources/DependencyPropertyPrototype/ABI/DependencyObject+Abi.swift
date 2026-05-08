// DOAbi is the 
public class DOAbi {
     func registerPropertyChanged(_ dp: DependencyPropertyHandle, _ handler: @escaping (Any?, Any?) -> Void) {
        print("abi :registerPropertyChanged")
        self.handler = handler
    }

    var handler: ((Any, Any) -> Void)?
}