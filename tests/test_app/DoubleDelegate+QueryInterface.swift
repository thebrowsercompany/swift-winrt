import Ctest_component
@_spi(WinRTImplements) import test_component
import Foundation

extension DoubleDelegate {
    @_spi(WinRTImplements)
    public func queryInterface(_ riid: REFIID, _ ppvObj: UnsafeMutablePointer<LPVOID?>?) -> HRESULT {
        if riid.pointee == __ABI_test_component.IBasic.IID {
            guard let iBasicWrapper = __ABI_test_component.IBasicWrapper(self) else { fatalError("failed to create wrapper") }
            return iBasicWrapper.queryInterface(riid, ppvObj)
        } else if riid.pointee == __ABI_test_component.ISimpleDelegate.IID {
            guard let iSimpleDelegateWrapper = __ABI_test_component.ISimpleDelegateWrapper(self) else { fatalError("failed to create wrapper") }
            return iSimpleDelegateWrapper.queryInterface(riid, ppvObj)
        } else {
            return E_NOINTERFACE
        }
    }
}

