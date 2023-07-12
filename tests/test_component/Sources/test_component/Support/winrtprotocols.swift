import Ctest_component

// Protocols for WinRT types that are used by public APIs

public protocol WinRTStruct {}
public protocol WinRTEnum {}

public protocol IWinRTObject: AnyObject {
  var thisPtr: test_component.IInspectable { get }
}

// ABI pointers for interfaces are made at runtime, which is why
// this is a seperate protocol with a function instead of a property
public protocol WinRTInterface: AnyObject {
  @_spi(WinRTInternal)
  func getAbiMaker() -> () -> UnsafeMutablePointer<Ctest_component.IInspectable>
}

public protocol WinRTClass : IWinRTObject, Equatable {
    func _getABI<T>() -> UnsafeMutablePointer<T>?
}
public typealias AnyWinRTClass = any WinRTClass

public protocol UnsealedWinRTClass : WinRTClass {
    var _inner: UnsafeMutablePointer<Ctest_component.IInspectable>? { get }
    // rather than require an initializer, expose a type which can create this class.
    // we do this so app derived types don't have to implement initializers that will
    // never be called
    static var _makeFromAbi: any MakeFromAbi.Type { get }
}

public extension WinRTClass {
    fileprivate func _getDefaultAsIInspectable() -> test_component.IInspectable {
        // Every WinRT interface is binary compatible with IInspectable. asking this class for
        // the iinspectable will ensure we get the default implementation from whichever derived
        // class it actually is. 
        let cDefault: UnsafeMutablePointer<Ctest_component.IInspectable> = _getABI()!
        return IInspectable(cDefault)
    }
}

public func ==<T: WinRTClass>(_ lhs: T, _ rhs: T) -> Bool {
  return lhs._getDefaultAsIInspectable() == rhs._getDefaultAsIInspectable()
}

extension WinRTClass {
    public var thisPtr: test_component.IInspectable { _getDefaultAsIInspectable() }
}
