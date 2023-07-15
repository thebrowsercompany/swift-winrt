import Ctest_component
@_spi(WinRTInternal) import test_component
import Foundation

extension DoubleDelegate {
    @_spi(WinRTInternal)
    public func queryInterface(_ riid: REFIID, _ ppvObj: UnsafeMutablePointer<LPVOID?>?) -> HRESULT {
        if riid.pointee == __ABI_test_component.IBasic.IID {
            guard let iBasicWrapper = __ABI_test_component.IBasicWrapper(self) else { fatalError("failed to create wrapper") }
            let thisAsIBasic = try! iBasicWrapper.toABI { $0 }
            return thisAsIBasic.pointee.lpVtbl.pointee.QueryInterface(thisAsIBasic, riid, ppvObj)
        } else if riid.pointee == __ABI_test_component.ISimpleDelegate.IID {
            guard let iSimpleDelegateWrapper = __ABI_test_component.ISimpleDelegateWrapper(self) else { fatalError("failed to create wrapper") }
            let thisAsISimpleDelegate = try! iSimpleDelegateWrapper.toABI { $0 }
            return thisAsISimpleDelegate.pointee.lpVtbl.pointee.QueryInterface(thisAsISimpleDelegate, riid, ppvObj)
        } else {
            return E_NOINTERFACE
        }
    }
}

