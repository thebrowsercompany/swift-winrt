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

class ArrayScenarios: IArrayScenarios {
    var array: [Int32] = []
    func inArray(_ value: [Int32]) throws {
        array = value
    }

    func outArray(_ value: inout [Int32]) throws {
        value = array
    }

    func refArray(_ value: inout [Int32]) throws {
        for i in 0..<value.count-1 {
            value[i] = Int32(i)
        }
    }
    func returnArray() throws -> [Int32] {
        array
    }

    func doubleIn(_ value1: [Int32], _ value2: [Int32]) throws {}
    func inAndOut(_ value: [Int32], _ results: inout [Int32]) throws {}
    func inAndRef(_ value: [Int32], _ results: inout [Int32]) throws {}
    func inAndReturn(_ value: [Int32]) throws -> [Int32] { value }
    func inAndRefNonBlittable(_ value: [Int32], _ results: inout [Bool]) throws {}
    var arrayProperty: [Int32] = []
}

class ArrayInputTests : XCTestCase {
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

    public func testThroughSwiftImplementation() throws {
        let arrayScenario = ArrayScenarios()
        try ArrayMethods.testInArrayThroughSwiftImplementation(arrayScenario, [1, 2, 3])
        XCTAssertEqual([1, 2, 3], arrayScenario.array)
    }
}

public class ArrayOutputTests: XCTestCase {
    public func testOutIntArray() throws {
        var intArray = [Int32]()
        try ArrayMethods.outInt32Array(&intArray)
        XCTAssertEqual([1, 2, 3], intArray)
    }

    public func testOutStringArray() throws {
        var stringArray = [String]()
        try ArrayMethods.outStringArray(&stringArray)
        XCTAssertEqual(["1", "2", "3"], stringArray)
    }

    public func testOutObjectArray() throws {
        var objectArray = [Any?]()
        try ArrayMethods.outObjectArray(&objectArray)
        let mapped = try objectArray.compactMap { try XCTUnwrap($0 as? IStringable).toString() }
        XCTAssertEqual(["1", "2", "3"], mapped)
    }

    public func testOutStringableArray() throws {
        var stringableArray = [AnyIStringable?]()
        try ArrayMethods.outStringableArray(&stringableArray)
        let mapped = try stringableArray.compactMap { try $0?.toString() }
        XCTAssertEqual(["1", "2", "3"], mapped)
    }

    public func testOutStructArray() throws {
        var structArray = [BlittableStruct]()
        try ArrayMethods.outStructArray(&structArray)
        XCTAssertEqual([
            BlittableStruct(first: 1, second: 2),
            BlittableStruct(first: 3, second: 4),
            BlittableStruct(first: 5, second: 6)
        ], structArray)
    }

    public func testOutNonBlittableStructArray() throws {
        var structArray = [NonBlittableStruct]()
        try ArrayMethods.outNonBlittableStructArray(&structArray)
        XCTAssertEqual([
            NonBlittableStruct(first: "1", second: "2", third: 3, fourth: "4"),
            NonBlittableStruct(first: "5", second: "6", third: 7, fourth: "8"),
            NonBlittableStruct(first: "9", second: "10", third: 11, fourth: "12")
        ], structArray)
    }

    public func testOutEnumArry() throws {
        var enumArray = [Signed]()
        try ArrayMethods.outEnumArray(&enumArray)
        XCTAssertEqual([.first, .second], enumArray)
    }

    public func testThroughSwiftImplementation() throws {
        let arrayScenario = ArrayScenarios()
        arrayScenario.array = [1, 3, 1, 10]
        try ArrayMethods.testOutArrayThroughSwiftImplementation(arrayScenario) { array in
            XCTAssertEqual([1, 3, 1, 10], array)
        }
    }
}

public class ArrayByReferenceTests: XCTestCase {
    public func testInIntArrayByReference() throws {
        var input = [Int32](repeating: 8675309, count: 5)
        try ArrayMethods.refInt32Array(&input)
        XCTAssertEqual([1, 2, 3, 4, 8675309], input)
    }

    public func testInStringArrayByReference() throws {
        var input = [String](repeating: "SWIFTRULES", count: 5)
        try ArrayMethods.refStringArray(&input)
        XCTAssertEqual(["1", "2", "3", "4", ""], input)
    }

    public func testInObjectArrayByReference() throws {
        var input = [Any?](repeating: StringableInt(value: 42), count: 5)
        try ArrayMethods.refObjectArray(&input)
        let mapped = try input.map { try ($0 as? IStringable)?.toString() }
        XCTAssertEqual(["1", "2", "3", "4", nil], mapped)
    }

    public func testInStringableArrayByReference() throws {
        var input = [AnyIStringable?](repeating: StringableInt(value: 42), count: 5)
        try ArrayMethods.refStringableArray(&input)
        let mapped = try input.map { try $0?.toString() }
        XCTAssertEqual(["1", "2", "3", "4", nil], mapped)
    }

    public func testInStructArrayByReference() throws {
        var input = [BlittableStruct](repeating: BlittableStruct(first: 10, second: 10), count: 3)
        try ArrayMethods.refStructArray(&input)
        XCTAssertEqual([
            BlittableStruct(first: 1, second: 2),
            BlittableStruct(first: 3, second: 4),
            BlittableStruct(first: 10, second: 10)], input)

    }

    public func testInNonBlittableStructArrayByReference() throws {
        var input = [NonBlittableStruct](repeating: NonBlittableStruct(first: "H", second: "E", third: 1, fourth: "P"), count: 3)
        try ArrayMethods.refNonBlittableStructArray(&input)
        XCTAssertEqual([
            NonBlittableStruct(first: "1", second: "2", third: 3, fourth: "4"),
            NonBlittableStruct(first: "5", second: "6", third: 7, fourth: "8"),
            NonBlittableStruct(first: "", second: "", third: 0, fourth: "")], input)
    }

    public func testInEnumArrayByReference() throws {
        var input = [Signed](repeating: .third, count: 3)
        try ArrayMethods.refEnumArray(&input)
        XCTAssertEqual([Signed.first, Signed.second, Signed.third], input)
    }

     public func testThroughSwiftImplementation() throws {
        let arrayScenario = ArrayScenarios()
        var testArray: [Int32] = [5, 4, 3, 2, 1]
        try ArrayMethods.testRefArrayThroughSwiftImplementation(arrayScenario, &testArray) { array in
            XCTAssertEqual([0, 1, 2, 3, 1], array)
        }
        XCTAssertEqual([0, 1, 2, 3, 1], testArray)
    }
}

public class ReturnArrayInputTests: XCTestCase {
    public func testReturnIntArray() throws {
        let result = try ArrayMethods.returnInt32Array()
        XCTAssertEqual([1, 2, 3], result)
    }
    public func testReturnStringArray() throws {
        let result = try ArrayMethods.returnStringArray()
        XCTAssertEqual(["1", "2", "3"], result)
    }
    public func testReturnObjectArray() throws {
        let result = try ArrayMethods.returnObjectArray()
        let mapped = try result.compactMap { try XCTUnwrap($0 as? IStringable).toString() }
        XCTAssertEqual(["1", "2", "3"], mapped)
    }
    public func testReturnStringableArray() throws {
        let result = try ArrayMethods.returnStringableArray()
        let mapped = try result.compactMap { try $0?.toString() }
        XCTAssertEqual(["1", "2", "3"], mapped)
    }
    public func testReturnStructArray() throws {
        let result = try ArrayMethods.returnStructArray()
        XCTAssertEqual([
            BlittableStruct(first: 1, second: 2),
            BlittableStruct(first: 3, second: 4),
            BlittableStruct(first: 5, second: 6)
        ], result)
    }
    public func testReturnNonBlittableStructArray() throws {
        let result = try ArrayMethods.returnNonBlittableStructArray()
        XCTAssertEqual([
            NonBlittableStruct(first: "1", second: "2", third: 3, fourth: "4"),
            NonBlittableStruct(first: "5", second: "6", third: 7, fourth: "8"),
            NonBlittableStruct(first: "9", second: "10", third: 11, fourth: "12")
        ], result)
    }
    public func testReturnEnumArray() throws {
        let result = try ArrayMethods.returnEnumArray()
        XCTAssertEqual([Signed.first, Signed.second], result)
    }

    public func testThroughSwiftImplementation() throws {
        let arrayScenario = ArrayScenarios()
        arrayScenario.array = [1, 3, 1, 10]

        try ArrayMethods.testReturnArrayThroughSwiftImplementation(arrayScenario) { array in
            XCTAssertEqual([1, 3, 1, 10], array)
        }
    }
}

private let inputArrayTests: [XCTestCaseEntry] = [
  testCase([
    ("testInputIntArray", ArrayInputTests.testInputIntArray),
    ("testInStringArray", ArrayInputTests.testInStringArray),
    ("testInObjectArray", ArrayInputTests.testInObjectArray),
    ("testInStringableArray", ArrayInputTests.testInStringableArray),
    ("testInStructArray", ArrayInputTests.testInStructArray),
    ("testInNonBlittableStructArray", ArrayInputTests.testInNonBlittableStructArray),
    ("testInEnumArray", ArrayInputTests.testInEnumArray),
    ("testThroughSwiftImplementation", ArrayInputTests.testThroughSwiftImplementation)
    ])
]
private let outputArrayTests: [XCTestCaseEntry] = [
  testCase([
    ("testOutIntArray", ArrayOutputTests.testOutIntArray),
    ("testOutStringArray", ArrayOutputTests.testOutStringArray),
    ("testOutObjectArray", ArrayOutputTests.testOutObjectArray),
    ("testOutStringableArray", ArrayOutputTests.testOutStringableArray),
    ("testOutStructArray", ArrayOutputTests.testOutStructArray),
    ("testOutNonBlittableStructArray", ArrayOutputTests.testOutNonBlittableStructArray),
    ("testOutEnumArry", ArrayOutputTests.testOutEnumArry),
    ("testThroughSwiftImplementation", ArrayOutputTests.testThroughSwiftImplementation)
  ])
]

private let referenceArrayTests: [XCTestCaseEntry] = [
  testCase([
    ("testInIntArrayByReference", ArrayByReferenceTests.testInIntArrayByReference),
    ("testInStringArrayByReference", ArrayByReferenceTests.testInStringArrayByReference),
    ("testInObjectArrayByReference", ArrayByReferenceTests.testInObjectArrayByReference),
    ("testInStringableArrayByReference", ArrayByReferenceTests.testInStringableArrayByReference),
    ("testInStructArrayByReference", ArrayByReferenceTests.testInStructArrayByReference),
    ("testInNonBlittableStructArrayByReference", ArrayByReferenceTests.testInNonBlittableStructArrayByReference),
    ("testInEnumArrayByReference", ArrayByReferenceTests.testInEnumArrayByReference),
    ("testThroughSwiftImplementation", ArrayByReferenceTests.testThroughSwiftImplementation)
  ])
]

private let returnArrayTests: [XCTestCaseEntry] = [
  testCase([
    ("testReturnIntArray", ReturnArrayInputTests.testReturnIntArray),
    ("testReturnStringArray", ReturnArrayInputTests.testReturnStringArray),
    ("testReturnObjectArray", ReturnArrayInputTests.testReturnObjectArray),
    ("testReturnStringableArray", ReturnArrayInputTests.testReturnStringableArray),
    ("testReturnStructArray", ReturnArrayInputTests.testReturnStructArray),
    ("testReturnNonBlittableStructArray", ReturnArrayInputTests.testReturnNonBlittableStructArray),
    ("testReturnEnumArray", ReturnArrayInputTests.testReturnEnumArray),
    ("testThroughSwiftImplementation", ReturnArrayInputTests.testThroughSwiftImplementation)
  ])
]

let arrayTests = inputArrayTests + outputArrayTests + referenceArrayTests + returnArrayTests