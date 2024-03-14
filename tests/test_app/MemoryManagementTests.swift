import XCTest
import test_component

class MemoryManagementTests : XCTestCase {

    public func testAggregatedObject() throws {
        weak var weakDerived: AppDerived? = nil
        try {
            let appDerived = AppDerived()
            // invoke a method so that _inner object is QI'ed
            try appDerived.doTheThing()
            weakDerived = appDerived
        }()
        XCTAssertNil(weakDerived)
    }

    public func testReturningAggregatedObject() throws {
       weak var weakDerived: AppDerived? = nil
        try {
          let classy = Class()
          let appDerived = AppDerived()
          weakDerived = appDerived
          classy.baseProperty = appDerived

          let base_returned = try XCTUnwrap(classy.baseProperty)
          XCTAssertNotNil(base_returned)
        }()
        XCTAssertNil(weakDerived)
    }

    public func testNonAggregatedObject() throws {
        weak var weakDerived: Derived? = nil
        try {
            let derived = Derived()
            // invoke a method so that _inner object is QI'ed
            try derived.doTheThing()
            weakDerived = derived
        }()
        XCTAssertNil(weakDerived)
    }
    public func testReturningNonAggregatedObject() throws {
       weak var weakDerived: Derived? = nil
        try {
          let classy = Class()
          let derived = Derived()
          weakDerived = derived
          classy.baseProperty = derived

          let base_returned = try XCTUnwrap(classy.baseProperty)
          XCTAssertNotNil(base_returned)
        }()
        XCTAssertNil(weakDerived)
    }

    func testWinRTObject() throws {
        weak var weakDerived: MyObj? = nil
        try {
            let obj = MyObj()
            weakDerived = obj

            // wrapping MyObj will create the identity pointer. make sure
            // this doesn't cause a leak
            let wrapper = try XCTUnwrap(__ABI_.AnyWrapper(obj))
            var copy: UnsafeMutablePointer<C_IInspectable>?
            defer {
              _ = copy?.pointee.lpVtbl.pointee.Release(copy)
            }
            wrapper.copyTo(&copy)
        }()
        XCTAssertNil(weakDerived)
    }
}

var memoryManagementTests: [XCTestCaseEntry] = [
  testCase([
    ("testAggregatedObject", MemoryManagementTests.testAggregatedObject),
    ("testNonAggregatedObject", MemoryManagementTests.testNonAggregatedObject),
    ("testReturningAggregatedObject", MemoryManagementTests.testReturningAggregatedObject),
    ("testReturningNonAggregatedObject", MemoryManagementTests.testReturningNonAggregatedObject),
    ("testWinRTObject", MemoryManagementTests.testWinRTObject)
  ])
]
