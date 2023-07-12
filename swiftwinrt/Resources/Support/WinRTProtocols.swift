import C_BINDINGS_MODULE

// Protocols for WinRT types that are used by public APIs

public protocol WinRTStruct {}
public protocol WinRTEnum {}

public protocol IWinRTObject: AnyObject {
  var thisPtr: SUPPORT_MODULE.IInspectable { get }
}

// ABI pointers for interfaces are made at runtime, which is why
// this is a seperate protocol with a function instead of a property
public protocol WinRTInterface: AnyObject {
  @_spi(WinRTInternal)
  func getAbiMaker() -> () -> UnsafeMutablePointer<C_BINDINGS_MODULE.IInspectable>
}

public protocol WinRTClass : IWinRTObject, Equatable {
    func _getABI<T>() -> UnsafeMutablePointer<T>?
}
public typealias AnyWinRTClass = any WinRTClass

public protocol UnsealedWinRTClass : WinRTClass {
    var _inner: UnsafeMutablePointer<C_BINDINGS_MODULE.IInspectable>? { get }
    // rather than require an initializer, expose a type which can create this class.
    // we do this so app derived types don't have to implement initializers that will
    // never be called
    static var _makeFromAbi: any MakeFromAbi.Type { get }
}

public extension WinRTClass {
    fileprivate func _getDefaultAsIInspectable() -> SUPPORT_MODULE.IInspectable {
        // Every WinRT interface is binary compatible with IInspectable. asking this class for
        // the iinspectable will ensure we get the default implementation from whichever derived
        // class it actually is. 
        let cDefault: UnsafeMutablePointer<C_BINDINGS_MODULE.IInspectable> = _getABI()!
        return IInspectable(cDefault)
    }
}

public func ==<T: WinRTClass>(_ lhs: T, _ rhs: T) -> Bool {
  return lhs._getDefaultAsIInspectable() == rhs._getDefaultAsIInspectable()
}

extension WinRTClass {
    public var thisPtr: SUPPORT_MODULE.IInspectable { _getDefaultAsIInspectable() }
}

@_spi(WinRTInternal)
extension WinRTClass {
  public func copyTo<Type>(_ ptr: UnsafeMutablePointer<UnsafeMutablePointer<Type>?>?) {
    guard let ptr else { return }
    let result: UnsafeMutablePointer<Type> = _getABI()!
    result.withMemoryRebound(to: C_BINDINGS_MODULE.IInspectable.self, capacity: 1) { 
      _ = $0.pointee.lpVtbl.pointee.AddRef($0)
    }
    ptr.initialize(to: result)
  }
}