import C_BINDINGS_MODULE
import WinSDK

// Redefine types so authors don't need to add WinSDK to their imports
public typealias IID = WinSDK.IID
public typealias HResult = WinSDK.HRESULT
public typealias QueryInterfaceResult = UnsafeMutableRawPointer

public protocol CustomQueryInterface {
  @_spi(WinRTImplements)
  func queryInterface(_ iid: IID, _ result: inout QueryInterfaceResult?) -> HResult
}

@_spi(WinRTInternal)
public func queryInterface(sealed obj: AnyWinRTClass, _ iid: IID, _ result: inout QueryInterfaceResult?) -> HResult {
    guard let cDefault: UnsafeMutablePointer<C_BINDINGS_MODULE.IInspectable> = obj._getABI() else { return E_NOINTERFACE }
    var iid = iid
    return cDefault.pointee.lpVtbl.pointee.QueryInterface(cDefault, &iid, &result) 
}

extension WinRTClass {
    @_spi(WinRTInternal)
    public func queryInterface(_ iid: IID, _ result: inout QueryInterfaceResult?) -> HResult {
        SUPPORT_MODULE.queryInterface(sealed: self, iid, &result)
    }
}

extension UnsealedWinRTClass {
    @_spi(WinRTInternal)
    public func queryInterface(_ iid: IID, _ result: inout QueryInterfaceResult?) -> HResult {
        SUPPORT_MODULE.queryInterface(unsealed: self, iid, &result)
    }
}

@_spi(WinRTInternal)
public func queryInterface(unsealed obj: AnyUnsealedWinRTClass, _ iid: IID, _ result: inout QueryInterfaceResult?) -> HResult {
  guard let inner = obj._inner ?? obj._getABI() else { return E_NOINTERFACE }
  var iid = iid
  return inner.pointee.lpVtbl.pointee.QueryInterface(inner, &iid, &result)
}

