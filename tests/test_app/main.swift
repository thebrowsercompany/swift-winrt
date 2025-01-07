import WinSDK
import XCTest
import test_component
import Ctest_component
import Foundation

class SwiftWinRTTests : XCTestCase {
  public func testBlittableStruct() throws {
    let simple = Simple()
    var blittableStruct = try simple.returnBlittableStruct()
    print("first:", blittableStruct.first)
    print("second: ", blittableStruct.second)

    XCTAssertEqual(blittableStruct.first, 123, "not copied correctly")
    XCTAssertEqual(blittableStruct.second, 456, "not copied correctly")

    // TakeBlittableStruct requires the struct to have these values
    let putBlittableStruct = BlittableStruct(first: 654, second: 321)

    try simple.takeBlittableStruct(putBlittableStruct)

    blittableStruct = simple.blittableStructProperty
    print("first:", blittableStruct.first)
    print("second: ", blittableStruct.second)

    XCTAssertEqual(blittableStruct.first, 0, "not copied correctly")
    XCTAssertEqual(blittableStruct.second, 0, "not copied correctly")

    simple.blittableStructProperty = putBlittableStruct

    blittableStruct = simple.blittableStructProperty
    print("first:", blittableStruct.first)
    print("second: ", blittableStruct.second)

    XCTAssertEqual(blittableStruct.first, 654, "not copied correctly")
    XCTAssertEqual(blittableStruct.second, 321, "not copied correctly")
  }

  public func testNonBlittableStruct() throws {
    let simple = Simple()
    var nonBlittableStruct = try simple.returnNonBlittableStruct()
    print("first:", nonBlittableStruct.first)
    print("second: ", nonBlittableStruct.second)

    XCTAssertEqual(nonBlittableStruct.first, "Hello", "not copied correctly")
    XCTAssertEqual(nonBlittableStruct.second, "World", "not copied correctly")

    // TakeBlittableStruct requires the struct to have these values
    let putNonBlittableStruct = NonBlittableStruct(first: "From", second: "Swift!", third: 3, fourth: "Yay!")

    try simple.takeNonBlittableStruct(putNonBlittableStruct)

    nonBlittableStruct = simple.nonBlittableStructProperty
    print("first:", nonBlittableStruct.first)
    print("second: ", nonBlittableStruct.second)

    XCTAssertEqual(nonBlittableStruct.first, "", "not copied correctly")
    XCTAssertEqual(nonBlittableStruct.second, "", "not copied correctly")

    simple.nonBlittableStructProperty = putNonBlittableStruct

    nonBlittableStruct = simple.nonBlittableStructProperty
    print("first:", nonBlittableStruct.first)
    print("second: ", nonBlittableStruct.second)
    print("third:", nonBlittableStruct.third)
    print("fourth: ", nonBlittableStruct.fourth)

    XCTAssertEqual(nonBlittableStruct.first, "From", "not copied correctly")
    XCTAssertEqual(nonBlittableStruct.second, "Swift!", "not copied correctly")
    XCTAssertEqual(nonBlittableStruct.third, 3, "not copied correctly")
    XCTAssertEqual(nonBlittableStruct.fourth, "Yay!", "not copied correctly")
  }

  public func testStructWithIReference() throws {
    let simple = Simple()
    var structWithIReference = try simple.returnStructWithReference()

    XCTAssertEqual(structWithIReference.value1, 4, "not copied correctly")
    XCTAssertEqual(structWithIReference.value2, 2, "not copied correctly")
    // takeStructWithReference requires the struct to have these values
    try simple.takeStructWithReference(structWithIReference)

    structWithIReference = simple.structWithReferenceProperty

    XCTAssertNil(structWithIReference.value1, "not copied correctly")
    XCTAssertNil(structWithIReference.value2, "not copied correctly")

    simple.structWithReferenceProperty = StructWithIReference(value1: 4, value2: nil)
    structWithIReference = simple.structWithReferenceProperty
    XCTAssertEqual(structWithIReference.value1, 4, "not copied correctly")
    XCTAssertEqual(structWithIReference.value2, nil, "not copied correctly")

    simple.structWithReferenceProperty = StructWithIReference(value1: 4, value2: 2)
    structWithIReference = simple.structWithReferenceProperty
    XCTAssertEqual(structWithIReference.value1, 4, "not copied correctly")
    XCTAssertEqual(structWithIReference.value2, 2, "not copied correctly")
  }

  public func testEnums() throws {
    let classy = Class()

    var enumVal = Signed.second
    let returned = try classy.inEnum(enumVal);
    XCTAssertEqual(returned, "Second", "improper value returned")

    enumVal = try classy.returnEnum()
    XCTAssertEqual(enumVal, Signed.first, "improper value returned")

    var enumProp = classy.enumProperty
    print("class has: ", enumProp)
    XCTAssertEqual(enumProp, Fruit.banana, "fruit should be b-a-n-a-n-a-s")

    print("setting enum to Apple")
    classy.enumProperty = .apple
    enumProp = classy.enumProperty
    print("an", enumProp, "a day keeps the bugs away")
    XCTAssertEqual(enumProp, Fruit.apple, "fruit should be apple")
  }

  public func testCustomConstructors() throws {
    var classy = Class("hello")
    XCTAssertEqual(classy.enumProperty, Fruit.banana, "fruit should be bananas")

    classy = Class("world", Fruit.pineapple)
    XCTAssertEqual(classy.enumProperty, Fruit.pineapple, "fruit should be Pineapple")
  }

  public func testStaticMethods() throws {
    try Class.staticTest()

    var result = try Class.staticTestReturn()
    print("result: ", result);
    XCTAssertEqual(result, 42);

    result = Class.staticProperty
    print("result: ", result);
    XCTAssertEqual(result, 18);

    print("calling static class methods")
    let output = try StaticClass.inEnum(Signed.first)
    print("result: ", output)
    XCTAssertEqual(output, "First")

    let fruit = try StaticClass.enumProperty
    print("result: ", fruit)
    XCTAssertEqual(fruit, Fruit.orange, "expected Orange!")

    StaticClass.enumProperty = Fruit.banana
    XCTAssertEqual(StaticClass.enumProperty, Fruit.banana, "expected Banana!")

    let putNonBlittableStruct = NonBlittableStruct(first: "From", second: "Swift!", third: 1, fourth: "Yay!")

    let putStructResult = try StaticClass.inNonBlittableStruct(putNonBlittableStruct)
    print("result: ", putStructResult)
    XCTAssertEqual(putStructResult, "FromSwift!Yay!")
  }

  public func testOutParams() throws {
    let classy = Class()
    print(">> testing OutInt32")
    var outInt: Int32 = 0
    try classy.outInt32(&outInt)
    print("     result: ", outInt)
    XCTAssertEqual(outInt, 123)

    print(">> testing OutString")
    var outString: String = ""
    try classy.outString(&outString)
    print("     result: ", outString)
    XCTAssertEqual(outString, "123")

    print(">> testing OutEnum")
    var outEnum: Signed = .second
    try classy.outEnum(&outEnum)
    print("     result: ", outEnum)
    XCTAssertEqual(outEnum, .first)

    print(">> testing OutBlittableStruct")
    var outBlittableStruct = BlittableStruct()
    try classy.outBlittableStruct(&outBlittableStruct)
    print("     result: ", outBlittableStruct)
    XCTAssertEqual(outBlittableStruct.first, 867)
    XCTAssertEqual(outBlittableStruct.second, 5309)

    print(">> testing OutNonBlittableStruct")
    var outNonBlittableStruct: NonBlittableStruct = .init()
    try classy.outNonBlittableStruct(&outNonBlittableStruct)
    print("     result: ", outNonBlittableStruct)
    XCTAssertEqual(outNonBlittableStruct.first, "please")
    XCTAssertEqual(outNonBlittableStruct.second, "vote")
    XCTAssertEqual(outNonBlittableStruct.third, 4)
    XCTAssertEqual(outNonBlittableStruct.fourth, "pedro")
  }

  public func testDelegate() throws {
    var classy = Class()
    do
    {
      print("TESTING ISimpleDelegate")

      let mySwiftDelegate = MySimpleDelegate()

      try classy.setDelegate(mySwiftDelegate)
      XCTAssertEqual(mySwiftDelegate.getThisCount(), 1)
      XCTAssertEqual(mySwiftDelegate.getThat(), 3)

      mySwiftDelegate.doThis()
      mySwiftDelegate.doThat(6)
      XCTAssertEqual(mySwiftDelegate.getThisCount(), 2)
      XCTAssertEqual(mySwiftDelegate.getThat(), 6)

      // Get the delegate back and make sure it works
      // TODO: WIN-78 retrieving the delegate causes a memory
      // leak. We should verify the delegate is fully cleaned up
      var retrievedDelegate = try classy.getDelegate()!
      try retrievedDelegate.doThis()
      try retrievedDelegate.doThat(9)

      XCTAssertEqual(mySwiftDelegate.getThisCount(), 3)
      XCTAssertEqual(mySwiftDelegate.getThat(), 9)

      // Null out the delegate from the C++ and retrieve a new one. Note that
      // we're initializing the swift object with `nil`, which is similar to
      // how in C++/WinRT this works.

      try classy.setDelegate(nil)

      // This will hand us a new C++ class which implements the interface.
      retrievedDelegate = try classy.getDelegate()!
      try retrievedDelegate.doThis()
      try retrievedDelegate.doThat(15)
      XCTAssertEqual(mySwiftDelegate.getThisCount(), 3)
      XCTAssertEqual(mySwiftDelegate.getThat(), 9)

      try classy.setDelegate(retrievedDelegate)
    }

    do
    {
      print("TESTING IIAmImplementable")

      let impl = MyImplementableDelegate()
      classy = Class("with delegate", .orange, impl)

      var enumVal = Signed.second
      let returned = try classy.inEnum(enumVal);
      XCTAssertEqual(returned, impl.inEnum(enumVal), "improper value returned")

      enumVal = try classy.returnEnum()
      XCTAssertEqual(enumVal, impl.returnEnum(), "improper value returned")

      var enumProp = classy.enumProperty
      print("class has: ", enumProp)
      XCTAssertEqual(enumProp, impl.enumProperty, "i'd be apple'd if this was wrong")

      print("setting enum to Pineapple")
      classy.enumProperty = .pineapple
      enumProp = classy.enumProperty
      print("a", enumProp, "is prickly")
      XCTAssertEqual(enumProp, impl.enumProperty, "fruit should be prickly")

      print(">> testing OutInt32")
      var outInt: Int32 = 0
      try classy.outInt32(&outInt)
      print("     result: ", outInt)

      var expectedOutInt: Int32 = 0
      impl.outInt32(&expectedOutInt)
      XCTAssertEqual(outInt, expectedOutInt)

      print(">> testing OutString")
      var outString: String = ""
      try classy.outString(&outString)

      var expectedOutString: String = ""
      impl.outString(&expectedOutString)
      print("     result: ", outString)
      XCTAssertEqual(outString, expectedOutString)

      print(">> testing OutEnum")
      var outEnum: Signed = .second
      try classy.outEnum(&outEnum)
      print("     result: ", outEnum)

      var expectedOutEnum: Signed = .second
      impl.outEnum(&expectedOutEnum)
      XCTAssertEqual(outEnum, expectedOutEnum)

      print(">> testing OutBlittableStruct")
      var outBlittableStruct = BlittableStruct()
      try classy.outBlittableStruct(&outBlittableStruct)
      print("     result: ", outBlittableStruct)

      var outBlittableStructExpected = BlittableStruct()
      impl.outBlittableStruct(&outBlittableStructExpected)
      // TODO: Implement equatable on structs
      //XCTAssertEqual(outBlittableStruct, outBlittableStructExpected)
      XCTAssertEqual(outBlittableStruct.first, outBlittableStructExpected.first)
      XCTAssertEqual(outBlittableStruct.second, outBlittableStructExpected.second)

      print(">> testing OutNonBlittableStruct")
      var outNonBlittableStruct: NonBlittableStruct = .init()
      try classy.outNonBlittableStruct(&outNonBlittableStruct)
      print("     result: ", outNonBlittableStruct)

      var outNonBlittableStructExected = NonBlittableStruct()
      impl.outNonBlittableStruct(&outNonBlittableStructExected)
      //XCTAssertEqual(outNonBlittableStruct, outNonBlittableStructExected)
      XCTAssertEqual(outNonBlittableStruct.first, outNonBlittableStructExected.first)
      XCTAssertEqual(outNonBlittableStruct.second, outNonBlittableStructExected.second)
      XCTAssertEqual(outNonBlittableStruct.third, outNonBlittableStructExected.third)
      XCTAssertEqual(outNonBlittableStruct.fourth, outNonBlittableStructExected.fourth)
    }
  }

  public func testNonDefaultMethods() throws {
    Class.staticPropertyFloat = 4.0
    XCTAssertEqual(Class.staticPropertyFloat, 4.0)
    XCTAssertEqual(try Class.staticTestReturnFloat(), 42.24)
    let classy = Class()
    classy.method()
  }

  public func testChar() throws {
    let classy = Class()
    print("classy ReturnChar: ", try classy.returnChar())
    XCTAssertEqual(try classy.returnChar(), "d")

    let result = try classy.inChar("x")
    print("classy InChar: ", result)
    XCTAssertEqual(result, "x")

    var out: Character = "_"
    try classy.outChar(&out)
    print("classy OutChar: ", out)
    XCTAssertEqual(out, "z")
  }

  class DoTheNewBase : IBasic {
    func method() {
      print("it's done")
    }
  }

  public func testDoubleDelegate() throws {
    let classy = Class()
    let newBase = DoTheNewBase()
    classy.implementation = newBase
    classy.method()

    var implReturn = classy.implementation!
    implReturn.method()

    XCTAssertIdentical(implReturn, newBase, "incorrect swift object returned")

    classy.implementation = nil
    implReturn = classy.implementation // Will create a new implementation
    XCTAssertNotIdentical(implReturn, newBase, "incorrect swift object returned")

    let double = DoubleDelegate()
    classy.implementation = double
    try classy.setDelegate(double)

    classy.method()

    let delegate = try classy.getDelegate()!
    try delegate.doThis()
    try delegate.doThat(19)

    XCTAssertIdentical(double, delegate)
    XCTAssertIdentical(double, classy.implementation)
  }

  public func testIReference() throws {
    let classy = Class()
    XCTAssertEqual(classy.startValue, nil)

    classy.startValue = 23
    print("value: ", classy.startValue ?? "N/A")
    XCTAssertEqual(classy.startValue, 23)

    let uuidString = "E621E1F8-C36C-495A-93FC-0C247A3E6E5F"
    let id: Foundation.UUID? = .init(uuidString: uuidString)
    classy.id = id
    let unwrappedID = try XCTUnwrap(classy.id)
    XCTAssertEqual("\(unwrappedID)", uuidString)
    XCTAssertEqual(classy.id, id)
  }

  public func testUnicode() throws {
    let classy = Class()
    _ = try classy.inString("\u{E909}")
  }

  public func testNullValues() throws {
    XCTAssertTrue(try NullValues.isObjectNull(nil))
    XCTAssertTrue(try NullValues.isInterfaceNull(nil))
    XCTAssertTrue(try NullValues.isGenericInterfaceNull(nil))
    XCTAssertTrue(try NullValues.isClassNull(nil))
    XCTAssertTrue(try NullValues.isDelegateNull(nil))

    XCTAssertFalse(try NullValues.isObjectNull(NoopClosable()))
    XCTAssertFalse(try NullValues.isInterfaceNull(NoopClosable()))
    XCTAssertFalse(try NullValues.isGenericInterfaceNull([""].toVector()))
    XCTAssertFalse(try NullValues.isClassNull(NoopClosable()))
    XCTAssertFalse(try NullValues.isDelegateNull({}))

    XCTAssertNil(try NullValues.getNullObject())
    XCTAssertNil(try NullValues.getNullInterface())
    XCTAssertNil(try NullValues.getNullGenericInterface())
    XCTAssertNil(try NullValues.getNullClass())
    XCTAssertNil(try NullValues.getNullDelegate())
  }

  public func testCasing() {
    // Don't change the casing of these names! This test is to make sure the
    // casing is as expected. It doesn't need to run, just compile.
    print("\(SwiftifiableNames.camelCase)")
    print("\(SwiftifiableNames.pascalCase)")
    print("\(SwiftifiableNames.esingleLetterPrefixed)")
    print("\(SwiftifiableNames.leadingCaps)")
    print("\(SwiftifiableNames.r8g8b8a8Typeless)")
    print("\(SwiftifiableNames.uuid)")
  }

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
}

var tests: [XCTestCaseEntry] = [
  testCase([
    ("testBlittableStruct", SwiftWinRTTests.testBlittableStruct),
    ("testChar", SwiftWinRTTests.testChar),
    ("testCustomConstructors", SwiftWinRTTests.testCustomConstructors),
    ("testDelegate", SwiftWinRTTests.testDelegate),
    ("testDoubleDelegate", SwiftWinRTTests.testDoubleDelegate),
    ("testEnums", SwiftWinRTTests.testEnums),
    ("testIReference", SwiftWinRTTests.testIReference),
    ("testNonBlittableStruct", SwiftWinRTTests.testNonBlittableStruct),
    ("testNonDefaultMethods", SwiftWinRTTests.testNonDefaultMethods),
    ("testNullValues", SwiftWinRTTests.testNullValues),
    ("testOutParams", SwiftWinRTTests.testOutParams),
    ("testStaticMethods", SwiftWinRTTests.testStaticMethods),
    ("testStructWithIReference", SwiftWinRTTests.testStructWithIReference),
    ("testUnicode", SwiftWinRTTests.testUnicode),
    ("testErrorInfo", SwiftWinRTTests.testErrorInfo),
  ])
] + valueBoxingTests + eventTests + collectionTests + aggregationTests + asyncTests + memoryManagementTests + bufferTests + weakReferenceTests

// Have to start adding tests in different lines, otherwise we get the following error:
//  error: the compiler is unable to type-check this expression in reasonable time; try breaking up the expression into distinct sub-expressions
tests += arrayTests

RoInitialize(RO_INIT_MULTITHREADED)
XCTMain(tests)
