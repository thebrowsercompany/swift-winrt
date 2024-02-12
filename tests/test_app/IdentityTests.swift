import test_component
import XCTest

class MyObj: WinRTObject {
  override public init() {
    super.init()
  }
}

class ABIHelper {
    static func toABI(_ obj1: __ABI_.AnyWrapper, _ obj2: __ABI_.AnyWrapper, _ body: (UnsafeMutablePointer<C_IInspectable>, UnsafeMutablePointer<C_IInspectable>) -> Void) throws {
        try obj1.toABI { abi1 in
            try obj2.toABI { abi2 in
                body(abi1, abi2)
            }
        }
    }
}

class IdentityTests: XCTestCase {
  public func testIdentity() throws {
    let obj = MyObj()
    let wrapper = try XCTUnwrap(__ABI_.AnyWrapper(obj))
    let wrapper2 = try XCTUnwrap(__ABI_.AnyWrapper(obj))

    // Since `identity` is internal and we don't have access to internals we just check that pointers from different wrappers
    // are the same
    try ABIHelper.toABI(wrapper, wrapper2) { (abi1, abi2) in
        XCTAssertEqual(abi1, abi2)
    }
  }
}

var identityTests: [XCTestCaseEntry] = [
  testCase([
    ("testIdentity", IdentityTests.testIdentity)
  ])
]