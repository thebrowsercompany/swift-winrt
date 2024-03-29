import WinSDK
import XCTest
@_spi(WinRTInternal) import test_component
import Ctest_component
import Foundation

class WeakReferenceTests : XCTestCase {
  class Target: IReferenceTarget {
    func method() {}
  }

  public func testCreateAndResolve() throws {
    let original = Target()
    let weakReferencer = WeakReferencer(original)
    let roundtrippedAny = try XCTUnwrap(weakReferencer.resolve())
    let roundtripped = try XCTUnwrap(roundtrippedAny as? Target)
    XCTAssertIdentical(roundtripped, original)
  }

  public func testNotStrong() throws {
    var original: Target! = Target()
    let weakReferencer = WeakReferencer(original)
    original = nil
    XCTAssertNil(try weakReferencer.resolve())
  }
}

var weakReferenceTests: [XCTestCaseEntry] = [
  testCase([
    ("testCreateAndResolve", WeakReferenceTests.testCreateAndResolve),
    ("testNotStrong", WeakReferenceTests.testNotStrong)
  ])
]
