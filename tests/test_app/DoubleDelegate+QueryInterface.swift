import Ctest_component
@_spi(WinRTImplements) import test_component
import Foundation

extension DoubleDelegate {
    @_spi(WinRTImplements)
    public func queryInterface(_ iid: test_component.IID) -> IUnknownRef? {
        switch iid {
            case __ABI_test_component.IBasic.IID:
                let iBasicWrapper = __ABI_test_component.IBasicWrapper(self)
                return iBasicWrapper!.queryInterface(iid)
            case __ABI_test_component.ISimpleDelegate.IID:
                let iSimpleDelegateWrapper = __ABI_test_component.ISimpleDelegateWrapper(self)
                return iSimpleDelegateWrapper!.queryInterface(iid)
            default: return nil
        }
    }
}
