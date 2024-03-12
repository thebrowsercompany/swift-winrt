import XCTest
import test_component
import Foundation

func AssertThrows(_ expression: @autoclosure () throws -> ()) {
  do {
    try expression()
    XCTFail("No error was thrown")
  } catch {
    // expected
  }
}

class ErrorHandlingTests : XCTestCase {
  public func testErrorInfo() {
    let message = "You are doing a bad thing"
    do {
      let classy = Class()
      try classy.fail(message)
    } catch {
      XCTAssertEqual("\(error)", message)
    }
  }
  public func testNoExcept() throws {
    let classy = Class()
    classy.noexceptVoid()
    classy.method()
    XCTAssertEqual(classy.noexceptInt32(), 123)
    XCTAssertEqual(classy.noexceptString(), "123")
  }

  public func testHandleFailedPropertyGet() throws {
    let failure = Failure()
    AssertThrows(_ = try withFailingCall(failure.failedProperty))
  }

  public func testHandleFailedPropertySet() throws {
    let failure = Failure()
    AssertThrows(try withFailingCall(failure.failedProperty = "123"))
  }

  public func testHandleFailedStaticPropertyGet() throws {
    AssertThrows(_ = try withFailingCall(Failure.failedStaticProperty))
  }

  public func testHandleFailedStaticMethod() throws {
    AssertThrows(try withFailingCall(Failure.failedStaticMethod()))
  }
}

var errorHandlingTests: [XCTestCaseEntry] = [
  testCase([
    ("testErrorInfo", ErrorHandlingTests.testErrorInfo),
    ("testNoExcept", ErrorHandlingTests.testNoExcept),
    ("testHandleFailedPropertyGet", ErrorHandlingTests.testHandleFailedPropertyGet),
    ("testHandleFailedPropertySet", ErrorHandlingTests.testHandleFailedPropertySet),
    ("testHandleFailedStaticPropertyGet", ErrorHandlingTests.testHandleFailedStaticPropertyGet),
    ("testHandleFailedStaticMethod", ErrorHandlingTests.testHandleFailedStaticMethod)
  ])
]