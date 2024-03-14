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

  public func testIdentityCopyTo() throws {
    let obj = MyObj()
    let wrapper = try XCTUnwrap(__ABI_.AnyWrapper(obj))
    let wrapper2 = try XCTUnwrap(__ABI_.AnyWrapper(obj))
    var copy: UnsafeMutablePointer<C_IInspectable>?
    var copy2: UnsafeMutablePointer<C_IInspectable>?
    defer {
      _ = copy?.pointee.lpVtbl.pointee.Release(copy)
      _ = copy2?.pointee.lpVtbl.pointee.Release(copy2)
    }

    wrapper.copyTo(&copy)
    wrapper2.copyTo(&copy2)
    XCTAssertEqual(copy, copy2)
  }

  // This test verifies that even after releasing the last reference
  // from WinRT, that the original identity still persists and is valid.
  // This is common in collection accessors, where the object is retrieved
  // and then immediately released.
  public func testIdentityAfterRelease() throws {
    let obj = MyObj()
    var copy2: UnsafeMutablePointer<C_IInspectable>?

    var wrapper:__ABI_.AnyWrapper? = try XCTUnwrap(__ABI_.AnyWrapper(obj))
    var copy: UnsafeMutablePointer<C_IInspectable>?

    wrapper?.copyTo(&copy)
    _ = copy?.pointee.lpVtbl.pointee.Release(copy)
    wrapper = nil


    let wrapper2 = try XCTUnwrap(__ABI_.AnyWrapper(obj))
    wrapper2.copyTo(&copy2)
    XCTAssertEqual(copy, copy2)
    _ = copy2?.pointee.lpVtbl.pointee.Release(copy2)
  }
}

var identityTests: [XCTestCaseEntry] = [
  testCase([
    ("testIdentity", IdentityTests.testIdentity),
    ("testIdentityCopyTo", IdentityTests.testIdentityCopyTo),
    ("testIdentityAfterRelease", IdentityTests.testIdentityAfterRelease)
  ])
]