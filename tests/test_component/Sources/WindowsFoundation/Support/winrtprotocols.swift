import CWinRT
import Foundation

// Protocols for WinRT types that are used by public APIs

public protocol WinRTStruct {}
public protocol WinRTEnum {}

public protocol IWinRTObject: AnyObject {
  var thisPtr: WindowsFoundation.IInspectable { get }
}

public protocol WinRTInterface: AnyObject, CustomQueryInterface {
}

open class WinRTClass : CustomQueryInterface, Equatable {
    public init() {}

    @_spi(WinRTInternal)
    public init(_ ptr: WindowsFoundation.IInspectable) {
      _inner = ptr
    }

    @_spi(WinRTInternal)
    open func _getABI<T>() -> UnsafeMutablePointer<T>? {
        if T.self == C_IInspectable.self {
            return UnsafeMutableRawPointer(identity?.get())?.bindMemory(to: T.self, capacity: 1) ?? RawPointer(_inner)
        }
        if T.self == C_IUnknown.self {
            return UnsafeMutableRawPointer(identity?.get())?.bindMemory(to: T.self, capacity: 1) ?? RawPointer(_inner)
        }
        return nil
    }

    @_spi(WinRTInternal)
    public internal(set) var _inner: WindowsFoundation.IInspectable!

    var identity: ComPtr<C_IInspectable>?

    @_spi(WinRTImplements)
    open func queryInterface(_ iid: WindowsFoundation.IID) -> IUnknownRef? {
        WindowsFoundation.queryInterface(self, iid)
    }

    deinit {
      // ensure we release the _inner pointer before releasing identity. releasing the _inner
      // cleans up the underlying COM object, which might be holding a reference to the identity pointer.

      _inner = nil
      identity = nil
    }
}

public func ==<T: WinRTClass>(_ lhs: T, _ rhs: T) -> Bool {
  return lhs.thisPtr == rhs.thisPtr
}

extension WinRTClass: IWinRTObject {
    public var thisPtr: WindowsFoundation.IInspectable { try! _inner.QueryInterface() }
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
    // always use the runtime class name of the inner WinRT object. the winui runtime will query for
    // class names and if it isn't recognized, it will call out to IXamlMetadataProvider (IXMP)
    // to get the associated XamlType. We aren't using Xaml for swift, so we don't actually
    // need or want the framework to think it's dealing with custom types.
    return try! _inner.GetRuntimeClassName()
  }

  fileprivate func aggregated() -> Bool { identity != nil }

  // Get an interface for caching on a class. This method properly handles
  // reference counting via releasing the reference added on the Swift object
  // in the case of being aggregated. The wrapper still has the +1 ref on it,
  // which will be released when the object is destroyed. We can safely let the
  // objects be destroyed since _inner is destroyed last. Releasing _inner is what
  // cleans up the underlying COM object.
  public func getInterfaceForCaching<T: IUnknown>() -> T {
    let ptr:T = try! _inner.QueryInterface()
    if aggregated() {
      Unmanaged.passUnretained(self).release()
    }
    return ptr
  }
}
