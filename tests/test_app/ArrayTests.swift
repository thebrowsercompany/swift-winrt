import WinSDK
import XCTest
import test_component
import Foundation

class StringableInt: IStringable {
    let value: Int32
    init(value: Int32) {
        self.value = value
    }
    public func toString() -> String {
        return "\(value)"
    }
}

class ArrayTests : XCTestCase {
    public func testInputIntArray() throws {
        let input: [Int32] = [1, 2, 3, 4, 5]
        let classy = Class()
        let result = try classy.inInt32Array(input)
        XCTAssertEqual("12345", result)
    }

    public func testInStringArray() throws {
        let input: [String] = ["1", "2", "3", "4", "5"]
        let classy = Class()
        let result = try classy.inStringArray(input)
        XCTAssertEqual("12345", result)
    }

    public func testInObjectArray() throws {
        let input: [StringableInt] = [.init(value: 1), .init(value: 2), .init(value: 3), .init(value: 4), .init(value: 5)]
        let classy = Class()
        let result = try classy.inObjectArray(input)
        XCTAssertEqual("12345", result)
    }

    public func testInStringableArray() throws {
        let input: [StringableInt] = [.init(value: 1), .init(value: 2), .init(value: 3), .init(value: 4), .init(value: 5)]
        let classy = Class()
        let result = try classy.inStringableArray(input)
        XCTAssertEqual("12345", result)
    }

    public func testInStructArray() throws {
        let input: [BlittableStruct] = [BlittableStruct(first: 1, second: 2), BlittableStruct(first: 3, second: 4) ]
        let classy = Class()
        let result = try classy.inStructArray(input)
        XCTAssertEqual("1234", result)
    }

    public func testInNonBlittableStructArray() throws {
        let input: [NonBlittableStruct] = [NonBlittableStruct(first: "1", second: "2", third: 3, fourth: "3"), NonBlittableStruct(first: "4", second: "5", third: 3, fourth: "6")]
        let classy = Class()
        let result = try classy.inNonBlittableStructArray(input)
        XCTAssertEqual("123456", result)
    }

    public func testInEnumArray() throws {
        let input: [Signed] = [.first, .second, .third]
        let classy = Class()
        let result = try classy.inEnumArray(input)
        XCTAssertEqual("0", result)
    }
}

var arrayTests: [XCTestCaseEntry] = [
  testCase([
    ("testInputIntArray", ArrayTests.testInputIntArray),
    ("testInStringArray", ArrayTests.testInStringArray),
    ("testInObjectArray", ArrayTests.testInObjectArray),
    ("testInStringableArray", ArrayTests.testInStringableArray),
    ("testInStructArray", ArrayTests.testInStructArray),
    ("testInNonBlittableStructArray", ArrayTests.testInNonBlittableStructArray),
    ("testInEnumArray", ArrayTests.testInEnumArray)
  ])
]
