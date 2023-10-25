import C_BINDINGS_MODULE
import WinSDK

public protocol CustomQueryInterface {
    @_spi(WinRTImplements)
    func queryInterface(_ iid: SUPPORT_MODULE.IID) -> IUnknownRef?
}

extension IUnknownRef {
    func queryInterface(_ iid: SUPPORT_MODULE.IID) -> IUnknownRef? {
        var iid = iid
        var result: UnsafeMutableRawPointer?
        guard borrow.pointee.lpVtbl.pointee.QueryInterface(borrow, &iid, &result) == S_OK, let result else { return nil }
        return IUnknownRef(consuming: result)
    }
}

@_spi(WinRTInternal)
public func queryInterface(_ obj: AnyWinRTClass, _ iid: SUPPORT_MODULE.IID) -> IUnknownRef? {
    obj._inner.pUnk.queryInterface(iid)
}

extension WinRTClass {
    @_spi(WinRTInternal)
    public func queryInterface(_ iid: SUPPORT_MODULE.IID) -> IUnknownRef? {
        SUPPORT_MODULE.queryInterface(self, iid)
    }
}
