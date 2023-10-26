import C_BINDINGS_MODULE
import WinSDK

public protocol CustomQueryInterface {
    @_spi(WinRTImplements)
    func queryInterface(_ iid: SUPPORT_MODULE.IID) -> IUnknownRef?
}

@_spi(WinRTInternal)
public func queryInterface(_ obj: AnyWinRTClass, _ iid: SUPPORT_MODULE.IID) -> IUnknownRef? {
    guard let cDefault: UnsafeMutablePointer<C_IInspectable> = obj._getABI() else { return nil }

    var iid = iid
    var result: UnsafeMutableRawPointer?
    guard cDefault.pointee.lpVtbl.pointee.QueryInterface(cDefault, &iid, &result) == S_OK, let result else { return nil }
    return IUnknownRef(consuming: result)
}

extension WinRTClass {
    @_spi(WinRTInternal)
    public func queryInterface(_ iid: SUPPORT_MODULE.IID) -> IUnknownRef? {
        SUPPORT_MODULE.queryInterface(self, iid)
    }
}
