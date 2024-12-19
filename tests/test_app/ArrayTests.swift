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
        let result = try ArrayMethods.inInt32Array(input)
        XCTAssertEqual("12345", result)
    }

    public func testInStringArray() throws {
        let input: [String] = ["1", "2", "3", "4", "5"]
        let result = try ArrayMethods.inStringArray(input)
        XCTAssertEqual("12345", result)
    }

    public func testInObjectArray() throws {
        let input: [StringableInt] = [.init(value: 1), .init(value: 2), .init(value: 3), .init(value: 4), .init(value: 5)]
        let result = try ArrayMethods.inObjectArray(input)
        XCTAssertEqual("12345", result)
    }

    public func testInStringableArray() throws {
        let input: [StringableInt] = [.init(value: 1), .init(value: 2), .init(value: 3), .init(value: 4), .init(value: 5)]
        let result = try ArrayMethods.inStringableArray(input)
        XCTAssertEqual("12345", result)
    }

    public func testInStructArray() throws {
        let input: [BlittableStruct] = [BlittableStruct(first: 1, second: 2), BlittableStruct(first: 3, second: 4) ]
        let result = try ArrayMethods.inStructArray(input)
        XCTAssertEqual("1234", result)
    }

    public func testInNonBlittableStructArray() throws {
        let input: [NonBlittableStruct] = [NonBlittableStruct(first: "1", second: "2", third: 3, fourth: "4"), NonBlittableStruct(first: "5", second: "6", third: 7, fourth: "8")]
        let result = try ArrayMethods.inNonBlittableStructArray(input)
        XCTAssertEqual("12345678", result)
    }

    public func testInEnumArray() throws {
        let input: [Signed] = [.first, .second, .third]
        let result = try ArrayMethods.inEnumArray(input)
        XCTAssertEqual("FirstSecondThird", result)
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
