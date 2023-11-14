import C_BINDINGS_MODULE
import WinSDK

public protocol CustomQueryInterface {
    @_spi(WinRTImplements)
    func queryInterface(_ iid: SUPPORT_MODULE.IID) -> IUnknownRef?
}

extension IUnknownRef {
    func queryInterface(_ iid: SUPPORT_MODULE.IID) -> IUnknownRef? {
        var iid = iid
        let (ptr) = try? ComPtrs.initialize(to: C_IUnknown.self) { result in
            try CHECKED(borrow.pointee.lpVtbl.pointee.QueryInterface(borrow, &iid, &result))
        }
        guard let ptr else { return nil}
        return IUnknownRef(ptr)
    }
}

@_spi(WinRTInternal)
public func queryInterface(_ obj: WinRTClass, _ iid: SUPPORT_MODULE.IID) -> IUnknownRef? {
    obj._inner.pUnk.queryInterface(iid)
}
