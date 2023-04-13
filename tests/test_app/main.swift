import WinSDK
import XCTest
import test_component
import Ctest_component
import Foundation

class AppDerived : Base {
  // overriding the default initialzer is a simple test so that we don't need to override any
  // other initializers
  override init(){ super.init() }

  var count = 0
  override func onDoTheThing() {
    print("we in the app yoooo")
    count+=1
  }
}

class AppDerived2 : UnsealedDerived {
  var count = 0

  override func onDoTheThing() {
    print("pt2: we in the app yoooo")
    count+=1
  }
}

class AppDerived3 : UnsealedDerived2 {
  var count = 0
  var beforeCount = 0
  override func onDoTheThing() {
    print("pt3: we in the app yoooo")
    count+=1
  }

  override func onBeforeDoTheThing() {
    print("before doing it")
    beforeCount+=1
  }
}

class AppDerivedNoOverrides : BaseNoOverrides {
  override init() {
    super.init()
  }
}


class AppDerivedNoOverrides2 : UnsealedDerivedNoOverrides {
  override init() {
    super.init()
  }
}

class SwiftWinRTTests : XCTestCase {
  public func testBlittableStruct() {
    let simple = Simple()
    var blittableStruct = simple.returnBlittableStruct()
    print("first:", blittableStruct.first)
    print("second: ", blittableStruct.second)

    XCTAssertEqual(blittableStruct.first, 123, "not copied correctly")
    XCTAssertEqual(blittableStruct.second, 456, "not copied correctly")

    // TakeBlittableStruct requires the struct to have these values
    let putBlittableStruct = BlittableStruct(first: 654, second: 321)

    simple.takeBlittableStruct(putBlittableStruct)

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

  public func testNonBlittableStruct() {
    let simple = Simple()
    var nonBlittableStruct = simple.returnNonBlittableStruct()
    print("first:", nonBlittableStruct.first)
    print("second: ", nonBlittableStruct.second)

    XCTAssertEqual(nonBlittableStruct.first, "Hello", "not copied correctly")
    XCTAssertEqual(nonBlittableStruct.second, "World", "not copied correctly")

    // TakeBlittableStruct requires the struct to have these values
    let putNonBlittableStruct = NonBlittableStruct(first: "From", second: "Swift!", third: 3, fourth: "Yay!")

    simple.takeNonBlittableStruct(putNonBlittableStruct)

    nonBlittableStruct = simple.nonBlittableStructProperty
    print("first:", nonBlittableStruct.first)
    print("second: ", nonBlittableStruct.second)

    XCTAssertEqual(nonBlittableStruct.first, "", "not copied correctly")
    XCTAssertEqual(nonBlittableStruct.second, "", "not copied correctly")

    simple.nonBlittableStructProperty = putNonBlittableStruct

    nonBlittableStruct = simple.nonBlittableStructProperty
    print("first:", nonBlittableStruct.first)
    print("second: ", nonBlittableStruct.second)
    print("third:", nonBlittableStruct.Third)
    print("fourth: ", nonBlittableStruct.Fourth)

    XCTAssertEqual(nonBlittableStruct.first, "From", "not copied correctly")
    XCTAssertEqual(nonBlittableStruct.second, "Swift!", "not copied correctly")
    XCTAssertEqual(nonBlittableStruct.Third, 3, "not copied correctly")
    XCTAssertEqual(nonBlittableStruct.Fourth, "Yay!", "not copied correctly")
  }

  public func testEnums() {
    let classy = Class()

    var enumVal = Signed.second
    let returned = classy.inEnum(enumVal);
    XCTAssertEqual(returned, "Second", "improper value returned")

    enumVal = classy.returnEnum()
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

  public func testCustomConstructors() {
    var classy = Class("hello")
    XCTAssertEqual(classy.enumProperty, Fruit.banana, "fruit should be bananas")

    classy = Class("world", Fruit.pineapple)
    XCTAssertEqual(classy.enumProperty, Fruit.pineapple, "fruit should be Pineapple")
  }

  public func testStaticMethods() {
    Class.staticTest()

    var result = Class.staticTestReturn()
    print("result: ", result);
    XCTAssertEqual(result, 42);

    result = Class.staticProperty
    print("result: ", result);
    XCTAssertEqual(result, 18);

    print("calling static class methods")
    let output = StaticClass.inEnum(Signed.first)
    print("result: ", output)
    XCTAssertEqual(output, "First")

    let fruit = StaticClass.enumProperty
    print("result: ", fruit)
    XCTAssertEqual(fruit, Fruit.orange, "expected Orange!")

    StaticClass.enumProperty = Fruit.banana
    XCTAssertEqual(StaticClass.enumProperty, Fruit.banana, "expected Banana!")

    let putNonBlittableStruct = NonBlittableStruct(first: "From", second: "Swift!", third: 1, fourth: "Yay!")

    let putStructResult = StaticClass.inNonBlittableStruct(putNonBlittableStruct)
    print("result: ", putStructResult)
    XCTAssertEqual(putStructResult, "FromSwift!Yay!")
  }

  public func testOutParams() {
    let classy = Class()
    print(">> testing OutInt32")
    var outInt: Int32 = 0
    classy.outInt32(&outInt)
    print("     result: ", outInt)
    XCTAssertEqual(outInt, 123)

    print(">> testing OutString")
    var outString: String = ""
    classy.outString(&outString)
    print("     result: ", outString)
    XCTAssertEqual(outString, "123")

    print(">> testing OutEnum")
    var outEnum: Signed = .second
    classy.outEnum(&outEnum)
    print("     result: ", outEnum)
    XCTAssertEqual(outEnum, .first)

    print(">> testing OutBlittableStruct")
    var outBlittableStruct = BlittableStruct()
    classy.outBlittableStruct(&outBlittableStruct)
    print("     result: ", outBlittableStruct)
    XCTAssertEqual(outBlittableStruct.first, 867)
    XCTAssertEqual(outBlittableStruct.second, 5309)

    print(">> testing OutNonBlittableStruct")
    var outNonBlittableStruct: NonBlittableStruct = .init()
    classy.outNonBlittableStruct(&outNonBlittableStruct)
    print("     result: ", outNonBlittableStruct)
    XCTAssertEqual(outNonBlittableStruct.first, "please")
    XCTAssertEqual(outNonBlittableStruct.second, "vote")
    XCTAssertEqual(outNonBlittableStruct.Third, 4)
    XCTAssertEqual(outNonBlittableStruct.Fourth, "pedro")
  }

  class MySimpleDelegate : ISimpleDelegate {
    private var thisCount: Int32 = 0
    private var that: Int32 = 0
    func doThis()
    {
      print("Swift - doThis!")
      thisCount += 1
    }

    func doThat(_ val: Int32)
    {
      print("Swift - Do that: ", val)
      that = val
    }

    func getThisCount() -> Int32 { thisCount }
    func getThat() -> Int32 { that }
  }
  
  class MyImplementableDelegate: IIAmImplementable {
    private var thisCount = 9
    func inInt32(_ value: Int32) -> String {
      return .init(repeating: "abc", count: Int(value))
    }

    func inString(_ value: String) -> String {
      return .init(value.reversed())
    }

    func inEnum(_ value: Signed) -> String {
      switch value {
        case .first: return "1 banana"
        case .second: return "2 banana"
        case .third: return "3 banana"
        default: return "n/a"
      }
    }

    func outInt32(_ value: inout Int32) {
      value = 987
    }

    func outString(_ value: inout String) {
      value = "987"
    }

    func outBlittableStruct(_ value: inout BlittableStruct) {
      value = .init(first: 9876, second: 54321)
    }

    func outNonBlittableStruct(_ value: inout NonBlittableStruct) {
      value = .init(first: "to be", second: "or not", third: 2, fourth: "be, that is the question")
    }

    func outEnum(_ value: inout Signed) {
      value = .second
    }

    func returnEnum() -> Signed {
      .Third
    }

    var enumProperty: Fruit = .apple

    var id: WinSDK.UUID?
    func fireEvent() {
    }
  } 

  public func testDelegate() {
    var classy = Class()

    do
    {
      print("TESTING ISimpleDelegate")

      let mySwiftDelegate = MySimpleDelegate()

      classy.setDelegate(mySwiftDelegate)
      XCTAssertEqual(mySwiftDelegate.getThisCount(), 1)
      XCTAssertEqual(mySwiftDelegate.getThat(), 3)

      mySwiftDelegate.doThis()
      mySwiftDelegate.doThat(6)
      XCTAssertEqual(mySwiftDelegate.getThisCount(), 2)
      XCTAssertEqual(mySwiftDelegate.getThat(), 6)

      // Get the delegate back and make sure it works
      // TODO: WIN-78 retrieving the delegate causes a memory
      // leak. We should verify the delegate is fully cleaned up
      var retrievedDelegate = classy.getDelegate()!
      retrievedDelegate.doThis()
      retrievedDelegate.doThat(9)

      XCTAssertEqual(mySwiftDelegate.getThisCount(), 3)
      XCTAssertEqual(mySwiftDelegate.getThat(), 9)
    
      // Null out the delegate from the C++ and retrieve a new one. Note that
      // we're initializing the swift object with `nil`, which is similar to 
      // how in C++/WinRT this works.

      classy.setDelegate(nil)

      // This will hand us a new C++ class which implements the interface.
      retrievedDelegate = classy.getDelegate()!
      retrievedDelegate.doThis()
      retrievedDelegate.doThat(15)
      XCTAssertEqual(mySwiftDelegate.getThisCount(), 3)
      XCTAssertEqual(mySwiftDelegate.getThat(), 9)

      classy.setDelegate(retrievedDelegate)
    }
  
    do
    {
      print("TESTING IIAmImplementable")

      let impl = MyImplementableDelegate()
      classy = Class("with delegate", .orange, impl)

      var enumVal = Signed.second
      let returned = classy.inEnum(enumVal);
      XCTAssertEqual(returned, impl.inEnum(enumVal), "improper value returned")

      enumVal = classy.returnEnum()
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
      classy.outInt32(&outInt)
      print("     result: ", outInt)

      var expectedOutInt: Int32 = 0
      impl.outInt32(&expectedOutInt)
      XCTAssertEqual(outInt, expectedOutInt)

      print(">> testing OutString")
      var outString: String = ""
      classy.outString(&outString)

      var expectedOutString: String = ""
      impl.outString(&expectedOutString)
      print("     result: ", outString)
      XCTAssertEqual(outString, expectedOutString)

      print(">> testing OutEnum")
      var outEnum: Signed = .second
      classy.outEnum(&outEnum)
      print("     result: ", outEnum)

      var expectedOutEnum: Signed = .second
      impl.outEnum(&expectedOutEnum)
      XCTAssertEqual(outEnum, expectedOutEnum)

      print(">> testing OutBlittableStruct")
      var outBlittableStruct = BlittableStruct()
      classy.outBlittableStruct(&outBlittableStruct)
      print("     result: ", outBlittableStruct)

      var outBlittableStructExpected = BlittableStruct()
      impl.outBlittableStruct(&outBlittableStructExpected)
      // TODO: Implement equatable on structs
      //XCTAssertEqual(outBlittableStruct, outBlittableStructExpected)
      XCTAssertEqual(outBlittableStruct.first, outBlittableStructExpected.first)
      XCTAssertEqual(outBlittableStruct.second, outBlittableStructExpected.second)

      print(">> testing OutNonBlittableStruct")
      var outNonBlittableStruct: NonBlittableStruct = .init()
      classy.outNonBlittableStruct(&outNonBlittableStruct)
      print("     result: ", outNonBlittableStruct)

      var outNonBlittableStructExected = NonBlittableStruct()
      impl.outNonBlittableStruct(&outNonBlittableStructExected)
      //XCTAssertEqual(outNonBlittableStruct, outNonBlittableStructExected)
      XCTAssertEqual(outNonBlittableStruct.first, outNonBlittableStructExected.first)
      XCTAssertEqual(outNonBlittableStruct.second, outNonBlittableStructExected.second)
      XCTAssertEqual(outNonBlittableStruct.Third, outNonBlittableStructExected.Third)
      XCTAssertEqual(outNonBlittableStruct.Fourth, outNonBlittableStructExected.Fourth)
    }
  }

  public func testNonDefaultMethods() {
    Class.staticPropertyFloat = 4.0
    XCTAssertEqual(Class.staticPropertyFloat, 4.0)
    XCTAssertEqual(Class.staticTestReturnFloat(), 42.24)
    let classy = Class()
    classy.method()
  }

  public func testChar() {
    let classy = Class()
    print("classy ReturnChar: ", classy.returnChar())
    XCTAssertEqual(classy.returnChar(), "d")

    let result = classy.inChar("x")
    print("classy InChar: ", result)
    XCTAssertEqual(result, "x")

    var out: Character = "_"
    classy.outChar(&out)
    print("classy OutChar: ", out)
    XCTAssertEqual(out, "z")
  }

  class DoTheNewBase : IBasic {
    func Method() {
      print("it's done")
    }
  }

  class DoubleDelegate : IBasic, ISimpleDelegate {
    func Method() {
      print("method doubled up")
    }

    func doThis() {
      print("Swift Double! - doThis!")
    }

    func doThat(_ val: Int32) {
      print("Swift Double! - Do that: ", val)
    }
  }

  public func testDoubleDelegate() {
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
    classy.setDelegate(double)

    classy.method()

    let delegate = classy.getDelegate()!
    delegate.doThis()
    delegate.doThat(19)

    XCTAssertIdentical(double, delegate)
    XCTAssertIdentical(double, classy.implementation)
  }

  public func testIReference() {
    let classy = Class()
    XCTAssertEqual(classy.startValue, nil)

    classy.startValue = 23
    print("value: ", classy.startValue ?? "N/A")
    XCTAssertEqual(classy.startValue, 23)

    let id: WinSDK.UUID? = .init(uuidString: "E621E1F8-C36C-495A-93FC-0C247A3E6E5F")
    classy.id = id

    print("ID: ", classy.id ?? "00000000-0000-0000-0000-0000000")
    XCTAssertEqual(classy.id, id)
  }

  public func testEvents() {
    let simple = Simple()
    var count = 0
    var static_count = 0
    
    var disposable = [Disposable?]()

    disposable.append(simple.signalEvent.addHandler {
      print("handler 1 called")
      count+=1
    })

    disposable.append(simple.signalEvent += {
      print("handler 2 called")
      count+=1
    })

    disposable.append(Simple.staticEvent.addHandler { (_,_) in
      print("static handler 1 called")
      static_count+=1
    })

    disposable.append(Simple.staticEvent += { (_,_) in
      print("static handler 2 called")
      static_count+=1
    })

    simple.fireEvent()
    XCTAssertEqual(count, 2)

    simple.fireEvent()
    XCTAssertEqual(count, 4)

    Simple.fireStaticEvent()
    XCTAssertEqual(static_count, 2)
    Simple.fireStaticEvent()
    XCTAssertEqual(static_count, 4)

    // dispose of the handlers and make sure we
    // aren't getting more events
    for dispose in disposable {
      if let dispose = dispose {
        dispose.dispose()
      }
    }
    disposable.removeAll(keepingCapacity: true)

    simple.fireEvent()
    XCTAssertEqual(count, 4)

    Simple.fireStaticEvent()
    XCTAssertEqual(static_count, 4)

    // hookup the handlers again and make sure it works, just to be safe
    _ = simple.signalEvent.addHandler {
      print("handler 1 called")
      count+=1
    }

    print("hooking up handler 2")
    disposable.append(simple.signalEvent += {
      print("handler 2 called")
      count+=1
    })

    disposable.append(Simple.staticEvent.addHandler { (_,_) in
      print("static handler 1 called")
      static_count+=1
    })

    disposable.append(Simple.staticEvent += { (_,_) in
      print("static handler 2 called")
      static_count+=1
    })

    simple.fireEvent()
    XCTAssertEqual(count, 6)

    Simple.fireStaticEvent()
    XCTAssertEqual(static_count, 6)
  }

  public func testAggregation() {
    let derived = Derived()
    derived.doTheThing()

    let appDerived = AppDerived()
    print(type(of: appDerived))

    let b: Base = appDerived as Base
    print(type(of: b))

    appDerived.doTheThing()
    XCTAssertEqual(appDerived.count, 1, "1. count not expected")

    print("foo");
    let appDerived2 = AppDerived2()
    print("foo");
    appDerived2.doTheThing()
    print("foo");
    XCTAssertEqual(appDerived2.count, 1, "2. count not expected")

    print("foo");
    let appDerived3 = AppDerived3()
    appDerived3.doTheThing()
    XCTAssertEqual(appDerived3.count, 1, "3. count not expected")
    XCTAssertEqual(appDerived3.beforeCount, 1, "4. count not expected")

    StaticClass.takeBase(appDerived)
    XCTAssertEqual(appDerived.count, 2, "5. count not expected")
    
    StaticClass.takeBase(appDerived2)
    XCTAssertEqual(appDerived2.count, 2, "6. count not expected")

    StaticClass.takeBase(appDerived3)
    XCTAssertEqual(appDerived3.count, 2, "7. count not expected")
    XCTAssertEqual(appDerived3.beforeCount, 2, "8. count not expected")


    print("testing unwrapping proper types from return values")
    let classy = Class()
    var base_returned = classy.baseProperty!
    XCTAssert(type(of: base_returned) == Derived.self)

    print("testing app derived")
    classy.baseProperty = appDerived

    base_returned = classy.baseProperty
    XCTAssert(type(of: base_returned) == AppDerived.self)
    XCTAssertIdentical(base_returned, appDerived)

    print("testing app derived2")

    classy.baseProperty = appDerived2

    base_returned = classy.baseProperty
    XCTAssert(type(of: base_returned) == AppDerived2.self)
    XCTAssertIdentical(base_returned, appDerived2)

    print("testing app derived3")

    classy.baseProperty = appDerived3

    base_returned = classy.baseProperty
    XCTAssert(type(of: base_returned) == AppDerived3.self)
    XCTAssertIdentical(base_returned, appDerived3)

    print("testing app derived no overrides")

    let derivedNoOverrides = AppDerivedNoOverrides()
    classy.baseNoOverridesProperty = derivedNoOverrides

    var baseNoOverrides_returned = classy.baseNoOverridesProperty!
    XCTAssert(type(of: baseNoOverrides_returned) == AppDerivedNoOverrides.self)
    XCTAssertIdentical(baseNoOverrides_returned, derivedNoOverrides)

    let derivedNoOverrides2 = AppDerivedNoOverrides2()
    classy.baseNoOverridesProperty = derivedNoOverrides2
    baseNoOverrides_returned = classy.baseNoOverridesProperty
    XCTAssert(type(of: baseNoOverrides_returned) == AppDerivedNoOverrides2.self)
    XCTAssertIdentical(baseNoOverrides_returned, derivedNoOverrides2)
  }

  func testUnicode() {
    let classy = Class()
    _ = classy.inString("\u{E909}")
  }
  
  public func testVector() {
    let array = ["Hello", "Goodbye", "Goodnight"]

    var result = Class.inVector(array.toVector())
    print(result)
    XCTAssertEqual(result, "Hello")

    let classy = Class()
    let vector = classy.returnStoredStringVector()!
    XCTAssertEqual(vector.count, 1)
    print(vector[0])

    result = Class.inVector(vector)

    print(result)
    XCTAssertEqual(result, vector[0])

    vector.append("Goodbye")
    XCTAssertEqual(vector.count, 2)
    // Make sure the returned vector has the same data
    // as the one we modified
    let vector2 = classy.returnStoredStringVector()!
    XCTAssertEqual(vector2.count, vector.count)
    XCTAssertEqual(vector2[0], vector2[0])
    XCTAssertEqual(vector2[1], vector2[1])

    vector2.append("Goodnight")

    XCTAssertEqual(vector2.count, 3)
    XCTAssertEqual(vector.count, 3)
    XCTAssertEqual(vector2[2], vector2[2])
  }

  public func testMap_asInput() {
    let dictionary = ["A": "Alpha"]
    let value = Class.inMap(dictionary.toMap())
    XCTAssertEqual(value, "Alpha")
  }
  
  public func testMap_asReturn() {
    let classy = Class()
    let map = classy.returnMapFromStringToString()!
    XCTAssertEqual(map.count, 1)
    XCTAssert(map.hasKey("A"))
    XCTAssertEqual(map.lookup("A"), "Alpha")
    XCTAssert(!map.hasKey("Z"))
  }

  public func testMap_mutate() {
    let classy = Class()
    let map = classy.returnMapFromStringToString()!
    XCTAssert(map.hasKey("A"))
    XCTAssertEqual(map.lookup("A"), "Alpha")
    
    XCTAssert(map.insert("A", "Aleph")) // Returns true if replacing
    XCTAssert(map.hasKey("A"))
    XCTAssertEqual(map.lookup("A"), "Aleph")
    let value = Class.inMap(map)
    XCTAssertEqual(value, "Aleph")
  }
  
  public func testNullValues() {
    XCTAssertTrue(NullValues.isObjectNull(nil))
    XCTAssertTrue(NullValues.isInterfaceNull(nil))
    XCTAssertTrue(NullValues.isGenericInterfaceNull(nil))
    XCTAssertTrue(NullValues.isClassNull(nil))
    XCTAssertTrue(NullValues.isDelegateNull(nil))
    
    XCTAssertFalse(NullValues.isObjectNull(try NoopClosable().as()))
    XCTAssertFalse(NullValues.isInterfaceNull(NoopClosable()))
    XCTAssertFalse(NullValues.isGenericInterfaceNull([""].toVector()))
    XCTAssertFalse(NullValues.isClassNull(NoopClosable()))
    XCTAssertFalse(NullValues.isDelegateNull(VoidToVoidDelegate(handler: {})))

    XCTAssertNil(NullValues.getNullObject())
    XCTAssertNil(NullValues.getNullInterface())
    XCTAssertNil(NullValues.getNullGenericInterface())
    XCTAssertNil(NullValues.getNullClass())
    XCTAssertNil(NullValues.getNullDelegate())
  }
}
 
var tests: [XCTestCaseEntry] = [
  testCase([
    ("testAggregation", SwiftWinRTTests.testAggregation),
    ("testBlittableStruct", SwiftWinRTTests.testBlittableStruct),
    ("testChar", SwiftWinRTTests.testChar),
    ("testCustomConstructors", SwiftWinRTTests.testCustomConstructors),
    ("testDelegate", SwiftWinRTTests.testDelegate),
    ("testDoubleDelegate", SwiftWinRTTests.testDoubleDelegate),
    ("testEnums", SwiftWinRTTests.testEnums),
    ("testEvents", SwiftWinRTTests.testEvents),
    ("testIReference", SwiftWinRTTests.testIReference),
    ("testMap_asInput", SwiftWinRTTests.testMap_asInput),
    ("testMap_asReturn", SwiftWinRTTests.testMap_asReturn),
    ("testMap_mutate", SwiftWinRTTests.testMap_mutate),
    ("testNonBlittableStruct", SwiftWinRTTests.testNonBlittableStruct),
    ("testNonDefaultMethods", SwiftWinRTTests.testNonDefaultMethods),
    ("testNullValues", SwiftWinRTTests.testNullValues),
    ("testOutParams", SwiftWinRTTests.testOutParams),
    ("testStaticMethods", SwiftWinRTTests.testStaticMethods),
    ("testUnicode", SwiftWinRTTests.testUnicode),
    ("testVector", SwiftWinRTTests.testVector),
  ])
]

RoInitialize(RO_INIT_MULTITHREADED)
XCTMain(tests)
