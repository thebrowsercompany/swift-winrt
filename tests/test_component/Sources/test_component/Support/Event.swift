import WinSDK
import Ctest_component

public protocol Disposable {
 func dispose()
}

public class Event<Data, Return> {
    @discardableResult open func addHandler(_ handler: @escaping (Data) -> Return) -> Disposable { fatalError("not implemented") }
}
