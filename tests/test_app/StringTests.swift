import CWinRT
import XCTest
import test_component
@_spi(WinRTInternal) import WindowsFoundation

class StringTests : XCTestCase {

    // MARK: - String.init(from:)

    // WinRT APIs return null HSTRING to represent empty strings.
    // String.init(from:) must handle nil by producing "".
    public func testStringFromNilHString() {
        let result = String(from: nil as HSTRING?)
        XCTAssertEqual(result, "")
    }

    // MARK: - String.withHStringRef

    // ASCII strings have equal grapheme cluster and UTF-16 code unit counts,
    // so this validates the basic withHStringRef round-trip path.
    public func testWithHStringRefAscii() throws {
        let s = "Hello, World!"
        s.withHStringRef { hstring in
            let result = String(from: hstring)
            XCTAssertEqual(result, s)
        }
    }

    // Emoji like 😀 (U+1F600) encode as a UTF-16 surrogate pair (2 code units)
    // but are a single grapheme cluster. This verifies that withHStringRef passes
    // the UTF-16 code unit count, not the grapheme cluster count, to
    // WindowsCreateStringReference.
    public func testWithHStringRefEmoji() throws {
        let s = "Hello 😀"
        s.withHStringRef { hstring in
            let result = String(from: hstring)
            XCTAssertEqual(result, s)
        }
    }

    // "e\u{0301}" (e + combining acute accent) is a single grapheme cluster but
    // 2 UTF-16 code units. This is a BMP-only case where grapheme cluster count
    // diverges from UTF-16 code unit count, distinct from the surrogate pair case.
    public func testWithHStringRefCombiningCharacter() throws {
        let s = "e\u{0301}vent"
        s.withHStringRef { hstring in
            let result = String(from: hstring)
            XCTAssertEqual(result, s)
        }
    }

    // Verify withHStringRef handles zero-length strings.
    public func testWithHStringRefEmpty() throws {
        let s = ""
        s.withHStringRef { hstring in
            let result = String(from: hstring)
            XCTAssertEqual(result, s)
        }
    }

    // MARK: - StaticString.withHStringRef

    // ASCII static strings have equal UTF-8 byte and UTF-16 code unit counts,
    // so this validates the basic StaticString.withHStringRef round-trip through
    // MultiByteToWideChar.
    public func testStaticStringWithHStringRefAscii() throws {
        let s: StaticString = "Hello"
        s.withHStringRef { hstring in
            let result = String(from: hstring)
            XCTAssertEqual(result, "Hello")
        }
    }

    // "café" has 5 UTF-8 bytes (é is 2 bytes) but only 4 UTF-16 code units.
    // This verifies that StaticString.withHStringRef uses the actual
    // MultiByteToWideChar conversion result as the HSTRING length, not the
    // UTF-8 byte count.
    public func testStaticStringWithHStringRefNonAscii() throws {
        let s: StaticString = "café"
        s.withHStringRef { hstring in
            let result = String(from: hstring)
            XCTAssertEqual(result, "café")
        }
    }

    // Verify StaticString.withHStringRef handles zero-length strings through
    // the MultiByteToWideChar path.
    public func testStaticStringWithHStringRefEmpty() throws {
        let s: StaticString = ""
        s.withHStringRef { hstring in
            let result = String(from: hstring)
            XCTAssertEqual(result, "")
        }
    }

    // MARK: - String.toABI / String.from round-trip

    // Verify toABI/from round-trip for a basic ASCII string.
    public func testToAbiRoundTripAscii() throws {
        let s = "Hello, World!"
        let hstring = try s.toABI()
        defer { WindowsDeleteString(hstring) }
        let result = String.from(abi: hstring)
        XCTAssertEqual(result, s)
    }

    // Verify toABI/from round-trip for a string containing emoji (surrogate
    // pairs in UTF-16).
    public func testToAbiRoundTripEmoji() throws {
        let s = "Hello 😀 World"
        let hstring = try s.toABI()
        defer { WindowsDeleteString(hstring) }
        let result = String.from(abi: hstring)
        XCTAssertEqual(result, s)
    }

    // Verify toABI/from round-trip for an empty string.
    public func testToAbiRoundTripEmpty() throws {
        let s = ""
        let hstring = try s.toABI()
        defer { WindowsDeleteString(hstring) }
        let result = String.from(abi: hstring)
        XCTAssertEqual(result, s)
    }
}

var stringTests: [XCTestCaseEntry] = [
    testCase([
        ("testStringFromNilHString", StringTests.testStringFromNilHString),
        ("testWithHStringRefAscii", StringTests.testWithHStringRefAscii),
        ("testWithHStringRefEmoji", StringTests.testWithHStringRefEmoji),
        ("testWithHStringRefCombiningCharacter", StringTests.testWithHStringRefCombiningCharacter),
        ("testWithHStringRefEmpty", StringTests.testWithHStringRefEmpty),
        ("testStaticStringWithHStringRefAscii", StringTests.testStaticStringWithHStringRefAscii),
        ("testStaticStringWithHStringRefNonAscii", StringTests.testStaticStringWithHStringRefNonAscii),
        ("testStaticStringWithHStringRefEmpty", StringTests.testStaticStringWithHStringRefEmpty),
        ("testToAbiRoundTripAscii", StringTests.testToAbiRoundTripAscii),
        ("testToAbiRoundTripEmoji", StringTests.testToAbiRoundTripEmoji),
        ("testToAbiRoundTripEmpty", StringTests.testToAbiRoundTripEmpty),
    ])
]
