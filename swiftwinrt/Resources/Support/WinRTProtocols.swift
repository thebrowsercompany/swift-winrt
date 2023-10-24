import C_BINDINGS_MODULE
import Foundation

// Protocols for WinRT types that are used by public APIs

public protocol WinRTStruct {}
public protocol WinRTEnum {}

public protocol IWinRTObject: AnyObject {
  var thisPtr: SUPPORT_MODULE.IInspectable { get }
}

public protocol WinRTInterface: AnyObject, CustomQueryInterface {
}

public protocol WinRTClass : IWinRTObject, CustomQueryInterface, Equatable {
    @_spi(WinRTInternal)
    func _getABI<T>() -> UnsafeMutablePointer<T>?
    @_spi(WinRTInternal)
    var _inner: SUPPORT_MODULE.IInspectable! { get }
}
public typealias AnyWinRTClass = any WinRTClass

public func ==<T: WinRTClass>(_ lhs: T, _ rhs: T) -> Bool {
  return lhs.thisPtr == rhs.thisPtr
}

extension WinRTClass {
    public var thisPtr: SUPPORT_MODULE.IInspectable { try! _inner.QueryInterface() }
}

@_spi(WinRTInternal)
extension WinRTClass {
  public func copyTo<Type>(_ ptr: UnsafeMutablePointer<UnsafeMutablePointer<Type>?>?) {
    guard let ptr else { return }
    let result: UnsafeMutablePointer<Type> = _getABI()!
    result.withMemoryRebound(to: C_IInspectable.self, capacity: 1) {
      _ = $0.pointee.lpVtbl.pointee.AddRef($0)
    }
    ptr.initialize(to: result)
  }

  public func GetRuntimeClassName() -> HString {
      let string = NSStringFromClass(type(of: self))
      return try! HString(string)
  }
}
