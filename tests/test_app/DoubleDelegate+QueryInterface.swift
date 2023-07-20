import Ctest_component
@_spi(WinRTImplements) import test_component
import Foundation

extension DoubleDelegate {
    @_spi(WinRTImplements)
    public func queryInterface(_ iid: IID, _ result: inout QueryInterfaceResult?) -> HResult {
        if iid == __ABI_test_component.IBasic.IID {
            guard let iBasicWrapper = __ABI_test_component.IBasicWrapper(self) else { fatalError("failed to create wrapper") }
            return iBasicWrapper.queryInterface(iid, &result)
        } else if iid == __ABI_test_component.ISimpleDelegate.IID {
            guard let iSimpleDelegateWrapper = __ABI_test_component.ISimpleDelegateWrapper(self) else { fatalError("failed to create wrapper") }
            return iSimpleDelegateWrapper.queryInterface(iid, &result)
        } else {
            return E_NOINTERFACE
        }
    }
}

