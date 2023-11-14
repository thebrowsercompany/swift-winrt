import XCTest

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
}

var memoryManagementTests: [XCTestCaseEntry] = [
  testCase([
    ("testAggregatedObject", MemoryManagementTests.testAggregatedObject),
  ])
]
