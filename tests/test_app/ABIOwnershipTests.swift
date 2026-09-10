import CWinRT
@_spi(WinRTInternal) import WindowsFoundation
import XCTest
import test_component

private enum ConversionFailure: Swift.Error {
    case expected
}

private final class AllocationCounter {
    let pointer = UnsafeMutablePointer<Int>.allocate(capacity: 1)
    var live: Int {
        get { pointer.pointee }
        set { pointer.pointee = newValue }
    }
    init() { pointer.initialize(to: 0) }
    deinit {
        pointer.deinitialize(count: 1)
        pointer.deallocate()
    }
}

private struct CountedABI: ToAbi {
    let counter: AllocationCounter
    var fails = false

    func toABI() throws -> UnsafeMutablePointer<Int> {
        if fails { throw ConversionFailure.expected }
        counter.live += 1
        return counter.pointer
    }

    static func release(abi: UnsafeMutablePointer<Int>) {
        abi.pointee -= 1
    }
}

final class ABIOwnershipTests: XCTestCase {
    func testNativeBoxedValuesReleaseStrings() throws {
        let native = Simple()
        native.stringProperty = "owned string"
        for _ in 0..<10 {
            XCTAssertEqual(try native.boxedStruct()?.first, "owned string")
            var value: NonBlittableStruct?
            try native.outBoxedStruct(&value)
            XCTAssertEqual(value?.first, "owned string")
            XCTAssertEqual(try native.boxedNestedStruct()?.value.first, "owned string")
            XCTAssertEqual(native.storedStringReferences, 1)
        }
    }

    func testNativeStructReturnsReleaseStrings() throws {
        let native = Simple()
        native.stringProperty = "nested string"
        for _ in 0..<10 {
            XCTAssertEqual(try native.nestedStruct().value.first, "nested string")
            XCTAssertEqual(native.storedStringReferences, 1)
        }
    }

    func testInputArraysReleaseStrings() throws {
        let native = Simple()
        try native.storeStrings(["input string"])
        XCTAssertEqual(native.stringProperty, "input string")
        XCTAssertEqual(native.storedStringReferences, 1)
        try native.storeStructs([.init(first: "struct string", second: "", third: 0, fourth: "")])
        XCTAssertEqual(native.stringProperty, "struct string")
        XCTAssertEqual(native.storedStringReferences, 1)
        try native.storeNestedStruct(.init(value: .init(first: "nested string", second: "", third: 0, fourth: "")))
        XCTAssertEqual(native.stringProperty, "nested string")
        XCTAssertEqual(native.storedStringReferences, 1)
    }

    func testOutputArraysReleaseStrings() throws {
        let native = Simple()
        native.stringProperty = "output string"
        for _ in 0..<10 {
            XCTAssertEqual(try native.storedStrings(), ["output string", "output string"])
            XCTAssertEqual(try native.storedStructs().map(\.first), ["output string", "output string"])
            var values = ["previous value", "previous value"]
            try native.outStoredStrings(&values)
            XCTAssertEqual(values, ["output string", "output string"])
            try native.fillStoredStrings(&values)
            XCTAssertEqual(values, ["output string", "output string"])
            XCTAssertEqual(native.storedStringReferences, 1)
        }
    }

    func testGetManyReleasesStrings() throws {
        let native = Simple()
        native.stringProperty = "vector string"
        let vector = try XCTUnwrap(native.storedStringVector())
        let baseline = native.storedStringReferences
        for _ in 0..<10 {
            var values = Array(repeating: "previous value", count: 3)
            XCTAssertEqual(try vector.getMany(0, &values), 2)
            XCTAssertEqual(Array(values.prefix(2)), ["vector string", "vector string"])
            XCTAssertEqual(native.storedStringReferences, baseline)
        }
    }

    func testArrayCleanupOnSuccessAndFailure() throws {
        let counter = AllocationCounter()
        let values = [CountedABI(counter: counter), CountedABI(counter: counter)]
        try values.toABI { _ in XCTAssertEqual(counter.live, 2) }
        XCTAssertEqual(counter.live, 0)
        XCTAssertThrowsError(try values.toABI { _ in throw ConversionFailure.expected })
        XCTAssertEqual(counter.live, 0)
        let failing = [CountedABI(counter: counter), CountedABI(counter: counter, fails: true)]
        XCTAssertThrowsError(try failing.toABI { _ in XCTFail("Conversion should fail first") })
        XCTAssertEqual(counter.live, 0)
        var output: UnsafeMutablePointer<UnsafeMutablePointer<Int>>?
        XCTAssertThrowsError(try failing.fill(abi: &output))
        XCTAssertNil(output)
        XCTAssertEqual(counter.live, 0)
    }
}

let abiOwnershipTests = [testCase([
    ("testNativeBoxedValuesReleaseStrings", ABIOwnershipTests.testNativeBoxedValuesReleaseStrings),
    ("testNativeStructReturnsReleaseStrings", ABIOwnershipTests.testNativeStructReturnsReleaseStrings),
    ("testInputArraysReleaseStrings", ABIOwnershipTests.testInputArraysReleaseStrings),
    ("testOutputArraysReleaseStrings", ABIOwnershipTests.testOutputArraysReleaseStrings),
    ("testGetManyReleasesStrings", ABIOwnershipTests.testGetManyReleasesStrings),
    ("testArrayCleanupOnSuccessAndFailure", ABIOwnershipTests.testArrayCleanupOnSuccessAndFailure)
])]
