// A "DependencyPropertyHandle" is the "true" DependencyProperty type that
// the WinUI system understands and uses
public class DependencyPropertyHandle {}

// DependencyProperties are the "public" properties for a given DependencyObject
// and are strongly typed to the actual type of the property and who owns them
// this allows us to use the swift compiler to enforce type safety
public struct DependencyProperties<Type: DependencyObject, Value>  {
    let handle: DependencyPropertyHandle
}

public protocol DependencyPropertyPublisher<Value> {
    associatedtype Value
    func sink(_ handler: @escaping (Value, Value) -> Void)
    var property: DependencyPropertyHandle { get }
}

public protocol DependencyPropertyProxy<Value> {
    associatedtype Value
    var set: (Value) -> Void { get }
    var get: () -> Value { get }
}

public protocol DependencyPropertyChangedProxy<Value>: DependencyPropertyPublisher, DependencyPropertyProxy{
}

// The DepdnencyProperty wrapper is given setters/getters since while it is possible to access the owning
// instance of the wrapper in the wrappedValue setter/getter, it is not possible to access the instance
// in the projectedValue. This means we can't properly subscribe to the property changed event on the
// object. It's for this reason that we have the DependencyPropertyProxy protocol instead of using KeyPaths as
// done here: https://www.swiftbysundell.com/articles/accessing-a-swift-property-wrappers-enclosing-instance/
@propertyWrapper
public struct DependencyProperty<Value> {
    public var wrappedValue: Value {
        get { _get() }
        set { _set(newValue) }
    }

    private var _get: () -> Value
    private var _set: (Value) -> Void

    init<Proxy: DependencyPropertyChangedProxy<Value>>(_ proxy: Proxy) {
        self._get = proxy.get
        self._set = proxy.set
        self.projectedValue = proxy
    }

   public let projectedValue: any DependencyPropertyPublisher<Value>
}
