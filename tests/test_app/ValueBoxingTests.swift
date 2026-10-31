import WinSDK
import XCTest
import test_component
import Foundation
import WindowsFoundation

struct Person : Equatable
{
  var firstName: String
  var lastName: String
  var age: Int
}

class ValueBoxingTests : XCTestCase {
  public func testInInt() {
    let value: Int = 2
    let classy = Class()
    let result = try! classy.inObject(value)
    XCTAssertEqual("\(value)", result)
  }

  public func testInString() {
    let words = "hello world"
    let classy = Class()
    let result = try! classy.inObject(words)
    XCTAssertEqual(words, result)
  }

  /*
  https://linear.app/the-browser-company/issue/WIN-650/some-unicode-conversions-dont-work
  public func testInChar() {
    let classy = Class()
    // regionalIndicatorForUS is 🇺🇸
    let regionalIndicatorForUS: Character = "\u{1F1FA}\u{1F1F8}"
    let result = try! classy.inObject(regionalIndicatorForUS)
    XCTAssertEqual(String(regionalIndicatorForUS), result)
  }
  */

  public func testInWinRTClass() {
    // Verify that we return the class name of the swift type
    let derived = Derived()
    let classy = Class()
    let result = try! classy.inObject(derived)
    XCTAssertEqual(NSStringFromClass(Derived.self), result)

    // and the class name of the WinRT base for a Swift class derived from one without overrides
    XCTAssertEqual("test_component.BaseNoOverrides", try! classy.inObject(AppDerivedNoOverrides()))
  }

  public func testInWinRTClassKeepsIdentity() throws {
    // The same projected object passed as two Objects reaches WinRT as one COM object
    let classy = Class()
    XCTAssertTrue(try StaticClass.isSameObject(classy, classy))
    XCTAssertFalse(try StaticClass.isSameObject(classy, Class()))

    // as does a Swift class derived from a projected one
    let appDerived = AppDerived()
    XCTAssertTrue(try StaticClass.isSameObject(appDerived, appDerived))
  }

  public func testInSwiftStruct() {
    // Swift structs are wrapped and boxed by the runtime in a __SwiftValue class:
    // https://github.com/apple/swift-corelibs-foundation/blob/4eacf8848d0a5a6bfb7a6fd52ad4ba0405dea5b2/Sources/Foundation/Bridging.swift#L73
    let classy = Class()
    let person = Person(firstName: "John", lastName: "Doe", age: 32)
    let result = try! classy.inObject(person)
    XCTAssertEqual("Foundation.__SwiftValue", result)
  }

  public func testOutInterface() {
    var anyObj: Any?
    let classy = Class()
    try! classy.outObject(&anyObj)
    let obj = anyObj as! IStringable
    XCTAssertEqual("123", try! obj.toString())
  }

  public func testInAppImplemented() {
    let classy = Class()
    let delegate = MySimpleDelegate()
    let result = try! classy.inObject(delegate)
    XCTAssertEqual("simple", result)
  }

  public func testInAppImplementedMultipleInterfaces() {
    let classy = Class()
    let doubleDelegate = DoubleDelegate()
    let result = try! classy.inObject(doubleDelegate)
    XCTAssertEqual("simply basic", result)
  }

  public func testRoundTripping() {
    // Add a swift implementation so this will test the roundtripping of swift
    // objects
    let impl = MyImplementableDelegate()
    let classy = Class("with delegate", .orange, impl)
    let person = Person(firstName: "John", lastName: "Doe", age: 32)
    let result = try! classy.inObject(person)
    XCTAssertEqual(result, String(describing: person))

    var anyObj: Any?

    try! classy.outObject(&anyObj)
    XCTAssertEqual(anyObj as! Person, person)

    anyObj = try! classy.returnObject()
    XCTAssertEqual(anyObj as! Person, person)

    // A Swift class derived from a projected one comes back as itself
    let appDerived = AppDerived()
    _ = try! classy.inObject(appDerived)
    try! classy.outObject(&anyObj)
    XCTAssertIdentical(anyObj as? AppDerived, appDerived)
    anyObj = try! classy.returnObject()
    XCTAssertIdentical(anyObj as? AppDerived, appDerived)
  }
}


var valueBoxingTests: [XCTestCaseEntry] = [
  testCase([
    ("InInt", ValueBoxingTests.testInInt),
    ("InString", ValueBoxingTests.testInString),
    ("InWinRTClass", ValueBoxingTests.testInWinRTClass),
    ("InWinRTClassKeepsIdentity", ValueBoxingTests.testInWinRTClassKeepsIdentity),
    ("InSwiftStruct", ValueBoxingTests.testInSwiftStruct),
    ("OutInterface", ValueBoxingTests.testOutInterface),
    ("testInAppImplemented", ValueBoxingTests.testInAppImplemented),
    ("testInAppImplementedMultipleInterfaces", ValueBoxingTests.testInAppImplementedMultipleInterfaces),
    ("RoundTripping", ValueBoxingTests.testRoundTripping)
  ])
]