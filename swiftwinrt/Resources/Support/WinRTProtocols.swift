import C_BINDINGS_MODULE

// Protocols for WinRT types that are used by public APIs

public protocol WinRTStruct {}
public protocol WinRTEnum {}

public protocol IWinRTObject: AnyObject {
  var thisPtr: SUPPORT_MODULE.IInspectable { get }
}

public protocol WinRTInterface: AnyObject, CustomQueryInterface {
}

public protocol CustomQueryInterface {
  @_spi(WinRTInternal)
  func queryInterface(_ iid: REFIID, _ ppvObj: UnsafeMutablePointer<LPVOID?>?) -> HRESULT
}

public protocol WinRTClass : IWinRTObject, CustomQueryInterface, Equatable {
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
public typealias AnyUnsealedWinRTClass = any UnsealedWinRTClass

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

extension WinRTClass {
    @_spi(WinRTInternal)
    public func queryInterface(_ riid: REFIID, _ ppvObj: UnsafeMutablePointer<LPVOID?>?) -> HRESULT {
        queryInterfaceImpl(riid, ppvObj)
    }

    fileprivate func queryInterfaceImpl(_ riid: REFIID, _ ppvObj: UnsafeMutablePointer<LPVOID?>?) -> HRESULT {
      guard let cDefault: UnsafeMutablePointer<C_BINDINGS_MODULE.IInspectable> = _getABI() else { return E_NOINTERFACE }
      return cDefault.pointee.lpVtbl.pointee.QueryInterface(cDefault, riid, ppvObj) 
    }
}

extension UnsealedWinRTClass {
    @_spi(WinRTInternal)
    public func queryInterface(_ riid: REFIID, _ ppvObj: UnsafeMutablePointer<LPVOID?>?) -> HRESULT {
        guard let inner = _inner else { return queryInterfaceImpl(riid, ppvObj) }
        return inner.pointee.lpVtbl.pointee.QueryInterface(inner, riid, ppvObj)
    }
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
