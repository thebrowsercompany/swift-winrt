import Ctest_component
import WinSDK

public typealias IID = WinSDK.IID

public protocol CustomQueryInterface {
    @_spi(WinRTImplements)
    func queryInterface(_ iid: IID) -> IUnknownRef?
}

@_spi(WinRTInternal)
public func queryInterface(sealed obj: AnyWinRTClass, _ iid: IID) -> IUnknownRef? {
    guard let cDefault: UnsafeMutablePointer<Ctest_component.IInspectable> = obj._getABI() else { return nil }

    var iid = iid
    var result: UnsafeMutableRawPointer?
    guard cDefault.pointee.lpVtbl.pointee.QueryInterface(cDefault, &iid, &result) == S_OK, let result else { return nil }
    return IUnknownRef(consuming: result)
}

extension WinRTClass {
    @_spi(WinRTInternal)
    public func queryInterface(_ iid: IID) -> IUnknownRef? {
        test_component.queryInterface(sealed: self, iid)
    }
}

extension UnsealedWinRTClass {
    @_spi(WinRTInternal)
    public func queryInterface(_ iid: IID) -> IUnknownRef? {
        test_component.queryInterface(unsealed: self, iid)
    }
}

@_spi(WinRTInternal)
public func queryInterface(unsealed obj: AnyUnsealedWinRTClass, _ iid: IID) -> IUnknownRef? {
    guard let inner = obj._inner ?? obj._getABI() else { return nil }
    var iid = iid
    var result: UnsafeMutableRawPointer?
    guard inner.pointee.lpVtbl.pointee.QueryInterface(inner, &iid, &result) == S_OK, let result else { return nil }
    return IUnknownRef(consuming: result)
}

