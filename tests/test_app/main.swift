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
  override func OnDoTheThing() {
    print("we in the app yoooo")
    count+=1
  }
}

class AppDerived2 : UnsealedDerived {
  var count = 0

  override func OnDoTheThing() {
    print("pt2: we in the app yoooo")
    count+=1
  }
}

class AppDerived3 : UnsealedDerived2 {
  var count = 0
  var beforeCount = 0
  override func OnDoTheThing() {
    print("pt3: we in the app yoooo")
    count+=1
  }

  override func OnBeforeDoTheThing() {
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
  public func TestBlittableStruct() {
    let simple = Simple()
    var blittableStruct = simple.ReturnBlittableStruct()
    print("first:", blittableStruct.First)
    print("second: ", blittableStruct.Second)

    XCTAssertEqual(blittableStruct.First, 123, "not copied correctly")
    XCTAssertEqual(blittableStruct.Second, 456, "not copied correctly")

    // TakeBlittableStruct requires the struct to have these values
    let putBlittableStruct = BlittableStruct(First: 654, Second: 321)

    simple.TakeBlittableStruct(putBlittableStruct)

    blittableStruct = simple.BlittableStructProperty
    print("first:", blittableStruct.First)
    print("second: ", blittableStruct.Second)

    XCTAssertEqual(blittableStruct.First, 0, "not copied correctly")
    XCTAssertEqual(blittableStruct.Second, 0, "not copied correctly")

    simple.BlittableStructProperty = putBlittableStruct

    blittableStruct = simple.BlittableStructProperty
    print("first:", blittableStruct.First)
    print("second: ", blittableStruct.Second)

    XCTAssertEqual(blittableStruct.First, 654, "not copied correctly")
    XCTAssertEqual(blittableStruct.Second, 321, "not copied correctly")
  }

  public func TestNonBlittableStruct() {
    let simple = Simple()
    var nonBlittableStruct = simple.ReturnNonBlittableStruct()
    print("first:", nonBlittableStruct.First)
    print("second: ", nonBlittableStruct.Second)

    XCTAssertEqual(nonBlittableStruct.First, "Hello", "not copied correctly")
    XCTAssertEqual(nonBlittableStruct.Second, "World", "not copied correctly")

    // TakeBlittableStruct requires the struct to have these values
    let putNonBlittableStruct = NonBlittableStruct(First: "From", Second: "Swift!", Third: 3, Fourth: "Yay!")

    simple.TakeNonBlittableStruct(putNonBlittableStruct)

    nonBlittableStruct = simple.NonBlittableStructProperty
    print("first:", nonBlittableStruct.First)
    print("second: ", nonBlittableStruct.Second)

    XCTAssertEqual(nonBlittableStruct.First, "", "not copied correctly")
    XCTAssertEqual(nonBlittableStruct.Second, "", "not copied correctly")

    simple.NonBlittableStructProperty = putNonBlittableStruct

    nonBlittableStruct = simple.NonBlittableStructProperty
    print("first:", nonBlittableStruct.First)
    print("second: ", nonBlittableStruct.Second)
    print("third:", nonBlittableStruct.Third)
    print("fourth: ", nonBlittableStruct.Fourth)

    XCTAssertEqual(nonBlittableStruct.First, "From", "not copied correctly")
    XCTAssertEqual(nonBlittableStruct.Second, "Swift!", "not copied correctly")
    XCTAssertEqual(nonBlittableStruct.Third, 3, "not copied correctly")
    XCTAssertEqual(nonBlittableStruct.Fourth, "Yay!", "not copied correctly")
  }

  public func TestEnums() {
    let classy = Class()

    var enumVal = Signed.Second
    let returned = classy.InEnum(enumVal);
    XCTAssertEqual(returned, "Second", "improper value returned")

    enumVal = classy.ReturnEnum()
    XCTAssertEqual(enumVal, Signed.First, "improper value returned")

    var enumProp = classy.EnumProperty
    print("class has: ", enumProp)
    XCTAssertEqual(enumProp, Fruit.Banana, "fruit should be b-a-n-a-n-a-s")

    print("setting enum to Apple")
    classy.EnumProperty = .Apple
    enumProp = classy.EnumProperty
    print("an", enumProp, "a day keeps the bugs away")
    XCTAssertEqual(enumProp, Fruit.Apple, "fruit should be apple")
  }

  public func TestCustomConstructors() {
    var classy = Class("hello")
    XCTAssertEqual(classy.EnumProperty, Fruit.Banana, "fruit should be bananas")

    classy = Class("world", Fruit.Pineapple)
    XCTAssertEqual(classy.EnumProperty, Fruit.Pineapple, "fruit should be Pineapple")
  }

  public func TestStaticMethods() {
    Class.StaticTest()

    var result = Class.StaticTestReturn()
    print("result: ", result);
    XCTAssertEqual(result, 42);

    result = Class.StaticProperty
    print("result: ", result);
    XCTAssertEqual(result, 18);

    print("calling static class methods")
    let output = StaticClass.InEnum(Signed.First)
    print("result: ", output)
    XCTAssertEqual(output, "First")

    let fruit = StaticClass.EnumProperty
    print("result: ", fruit)
    XCTAssertEqual(fruit, Fruit.Orange, "expected Orange!")

    StaticClass.EnumProperty = Fruit.Banana
    XCTAssertEqual(StaticClass.EnumProperty, Fruit.Banana, "expected Banana!")

    let putNonBlittableStruct = NonBlittableStruct(First: "From", Second: "Swift!", Third: 1, Fourth: "Yay!")

    let putStructResult = StaticClass.InNonBlittableStruct(putNonBlittableStruct)
    print("result: ", putStructResult)
    XCTAssertEqual(putStructResult, "FromSwift!Yay!")
  }

  public func TestOutParams() {
    let classy = Class()
    print(">> testing OutInt32")
    var outInt: Int32 = 0
    classy.OutInt32(&outInt)
    print("     result: ", outInt)
    XCTAssertEqual(outInt, 123)

    print(">> testing OutString")
    var outString: String = ""
    classy.OutString(&outString)
    print("     result: ", outString)
    XCTAssertEqual(outString, "123")

    print(">> testing OutEnum")
    var outEnum: Signed = .Second
    classy.OutEnum(&outEnum)
    print("     result: ", outEnum)
    XCTAssertEqual(outEnum, .First)

    print(">> testing OutBlittableStruct")
    var outBlittableStruct = BlittableStruct()
    classy.OutBlittableStruct(&outBlittableStruct)
    print("     result: ", outBlittableStruct)
    XCTAssertEqual(outBlittableStruct.First, 867)
    XCTAssertEqual(outBlittableStruct.Second, 5309)

    print(">> testing OutNonBlittableStruct")
    var outNonBlittableStruct: NonBlittableStruct = .init()
    classy.OutNonBlittableStruct(&outNonBlittableStruct)
    print("     result: ", outNonBlittableStruct)
    XCTAssertEqual(outNonBlittableStruct.First, "please")
    XCTAssertEqual(outNonBlittableStruct.Second, "vote")
    XCTAssertEqual(outNonBlittableStruct.Third, 4)
    XCTAssertEqual(outNonBlittableStruct.Fourth, "pedro")
  }

  class MySimpleDelegate : ISimpleDelegate {
    private var thisCount: Int32 = 0
    private var that: Int32 = 0
    func DoThis()
    {
      print("Swift - DoThis!")
      thisCount += 1
    }

    func DoThat(_ val: Int32)
    {
      print("Swift - Do that: ", val)
      that = val
    }

    func GetThisCount() -> Int32 { thisCount }
    func GetThat() -> Int32 { that }
  }
  
  class MyImplementableDelegate: IIAmImplementable {
    private var thisCount = 9
    func InInt32(_ value: Int32) -> String {
      return .init(repeating: "abc", count: Int(value))
    }

    func InString(_ value: String) -> String {
      return .init(value.reversed())
    }

    func InEnum(_ value: Signed) -> String {
      switch value {
        case .First: return "1 banana"
        case .Second: return "2 banana"
        case .Third: return "3 banana"
        default: return "n/a"
      }
    }

    func OutInt32(_ value: inout Int32) {
      value = 987
    }

    func OutString(_ value: inout String) {
      value = "987"
    }

    func OutBlittableStruct(_ value: inout BlittableStruct) {
      value = .init(First: 9876, Second: 54321)
    }

    func OutNonBlittableStruct(_ value: inout NonBlittableStruct) {
      value = .init(First: "to be", Second: "or not", Third: 2, Fourth: "be, that is the question")
    }

    func OutEnum(_ value: inout Signed) {
      value = .Second
    }

    func ReturnEnum() -> Signed {
      .Third
    }

    var EnumProperty: Fruit = .Apple

    var ID: WinSDK.UUID?
    func FireEvent() {
    }
  } 

  public func TestDelegate() {
    var classy = Class()

    do
    {
      print("TESTING ISimpleDelegate")

      let mySwiftDelegate = MySimpleDelegate()

      classy.SetDelegate(mySwiftDelegate)
      XCTAssertEqual(mySwiftDelegate.GetThisCount(), 1)
      XCTAssertEqual(mySwiftDelegate.GetThat(), 3)

      mySwiftDelegate.DoThis()
      mySwiftDelegate.DoThat(6)
      XCTAssertEqual(mySwiftDelegate.GetThisCount(), 2)
      XCTAssertEqual(mySwiftDelegate.GetThat(), 6)

      // Get the delegate back and make sure it works
      // TODO: WIN-78 retrieving the delegate causes a memory
      // leak. We should verify the delegate is fully cleaned up
      var retrievedDelegate = classy.GetDelegate()!
      retrievedDelegate.DoThis()
      retrievedDelegate.DoThat(9)

      XCTAssertEqual(mySwiftDelegate.GetThisCount(), 3)
      XCTAssertEqual(mySwiftDelegate.GetThat(), 9)
    
      // Null out the delegate from the C++ and retrieve a new one. Note that
      // we're initializing the swift object with `nil`, which is similar to 
      // how in C++/WinRT this works.

      classy.SetDelegate(nil)

      // This will hand us a new C++ class which implements the interface.
      retrievedDelegate = classy.GetDelegate()!
      retrievedDelegate.DoThis()
      retrievedDelegate.DoThat(15)
      XCTAssertEqual(mySwiftDelegate.GetThisCount(), 3)
      XCTAssertEqual(mySwiftDelegate.GetThat(), 9)

      classy.SetDelegate(retrievedDelegate)
    }
  
    do
    {
      print("TESTING IIAmImplementable")

      let impl = MyImplementableDelegate()
      classy = Class("with delegate", .Orange, impl)

      var enumVal = Signed.Second
      let returned = classy.InEnum(enumVal);
      XCTAssertEqual(returned, impl.InEnum(enumVal), "improper value returned")

      enumVal = classy.ReturnEnum()
      XCTAssertEqual(enumVal, impl.ReturnEnum(), "improper value returned")

      var enumProp = classy.EnumProperty
      print("class has: ", enumProp)
      XCTAssertEqual(enumProp, impl.EnumProperty, "i'd be apple'd if this was wrong")

      print("setting enum to Pineapple")
      classy.EnumProperty = .Pineapple
      enumProp = classy.EnumProperty
      print("a", enumProp, "is prickly")
      XCTAssertEqual(enumProp, impl.EnumProperty, "fruit should be prickly")

      print(">> testing OutInt32")
      var outInt: Int32 = 0
      classy.OutInt32(&outInt)
      print("     result: ", outInt)

      var expectedOutInt: Int32 = 0
      impl.OutInt32(&expectedOutInt)
      XCTAssertEqual(outInt, expectedOutInt)

      print(">> testing OutString")
      var outString: String = ""
      classy.OutString(&outString)

      var expectedOutString: String = ""
      impl.OutString(&expectedOutString)
      print("     result: ", outString)
      XCTAssertEqual(outString, expectedOutString)

      print(">> testing OutEnum")
      var outEnum: Signed = .Second
      classy.OutEnum(&outEnum)
      print("     result: ", outEnum)

      var expectedOutEnum: Signed = .Second
      impl.OutEnum(&expectedOutEnum)
      XCTAssertEqual(outEnum, expectedOutEnum)

      print(">> testing OutBlittableStruct")
      var outBlittableStruct = BlittableStruct()
      classy.OutBlittableStruct(&outBlittableStruct)
      print("     result: ", outBlittableStruct)

      var outBlittableStructExpected = BlittableStruct()
      impl.OutBlittableStruct(&outBlittableStructExpected)
      // TODO: Implement equatable on structs
      //XCTAssertEqual(outBlittableStruct, outBlittableStructExpected)
      XCTAssertEqual(outBlittableStruct.First, outBlittableStructExpected.First)
      XCTAssertEqual(outBlittableStruct.Second, outBlittableStructExpected.Second)

      print(">> testing OutNonBlittableStruct")
      var outNonBlittableStruct: NonBlittableStruct = .init()
      classy.OutNonBlittableStruct(&outNonBlittableStruct)
      print("     result: ", outNonBlittableStruct)

      var outNonBlittableStructExected = NonBlittableStruct()
      impl.OutNonBlittableStruct(&outNonBlittableStructExected)
      //XCTAssertEqual(outNonBlittableStruct, outNonBlittableStructExected)
      XCTAssertEqual(outNonBlittableStruct.First, outNonBlittableStructExected.First)
      XCTAssertEqual(outNonBlittableStruct.Second, outNonBlittableStructExected.Second)
      XCTAssertEqual(outNonBlittableStruct.Third, outNonBlittableStructExected.Third)
      XCTAssertEqual(outNonBlittableStruct.Fourth, outNonBlittableStructExected.Fourth)
    }
  }

  public func TestNonDefaultMethods() {
    Class.StaticPropertyFloat = 4.0
    XCTAssertEqual(Class.StaticPropertyFloat, 4.0)
    XCTAssertEqual(Class.StaticTestReturnFloat(), 42.24)
    let classy = Class()
    classy.Method()
  }

  public func TestChar() {
    let classy = Class()
    print("classy ReturnChar: ", classy.ReturnChar())
    XCTAssertEqual(classy.ReturnChar(), "d")

    let result = classy.InChar("x")
    print("classy InChar: ", result)
    XCTAssertEqual(result, "x")

    var out: Character = "_"
    classy.OutChar(&out)
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

    func DoThis() {
      print("Swift Double! - DoThis!")
    }

    func DoThat(_ val: Int32) {
      print("Swift Double! - Do that: ", val)
    }
  }

  public func TestDoubleDelegate() {
    let classy = Class()
    let newBase = DoTheNewBase()
    classy.Implementation = newBase
    classy.Method()

    var implReturn = classy.Implementation!
    implReturn.Method()

    XCTAssertIdentical(implReturn, newBase, "incorrect swift object returned")

    classy.Implementation = nil
    implReturn = classy.Implementation! // Will create a new implementation
    XCTAssertNotIdentical(implReturn, newBase, "incorrect swift object returned")

    let double = DoubleDelegate()
    classy.Implementation = double
    classy.SetDelegate(double)

    classy.Method()

    let delegate = classy.GetDelegate()!
    delegate.DoThis()
    delegate.DoThat(19)

    XCTAssertIdentical(double, delegate)
    XCTAssertIdentical(double, classy.Implementation)
  }

  public func TestIReference() {
    let classy = Class()
    XCTAssertEqual(classy.StartValue, nil)

    classy.StartValue = 23
    print("value: ", classy.StartValue ?? "N/A")
    XCTAssertEqual(classy.StartValue, 23)

    let id: WinSDK.UUID? = .init(uuidString: "E621E1F8-C36C-495A-93FC-0C247A3E6E5F")
    classy.ID = id

    print("ID: ", classy.ID ?? "00000000-0000-0000-0000-0000000")
    XCTAssertEqual(classy.ID, id)
  }

  public func TestEvents() {
    let simple = Simple()
    var count = 0
    var static_count = 0
    
    var disposable = [Disposable?]()

    disposable.append(simple.SignalEvent.addHandler {
      print("handler 1 called")
      count+=1
    })

    disposable.append(simple.SignalEvent += {
      print("handler 2 called")
      count+=1
    })

    disposable.append(Simple.StaticEvent.addHandler { (_,_) in
      print("static handler 1 called")
      static_count+=1
    })

    disposable.append(Simple.StaticEvent += { (_,_) in
      print("static handler 2 called")
      static_count+=1
    })

    simple.FireEvent()
    XCTAssertEqual(count, 2)

    simple.FireEvent()
    XCTAssertEqual(count, 4)

    Simple.FireStaticEvent()
    XCTAssertEqual(static_count, 2)
    Simple.FireStaticEvent()
    XCTAssertEqual(static_count, 4)

    // dispose of the handlers and make sure we
    // aren't getting more events
    for dispose in disposable {
      if let dispose = dispose {
        dispose.dispose()
      }
    }
    disposable.removeAll(keepingCapacity: true)

    simple.FireEvent()
    XCTAssertEqual(count, 4)

    Simple.FireStaticEvent()
    XCTAssertEqual(static_count, 4)

    // hookup the handlers again and make sure it works, just to be safe
    _ = simple.SignalEvent.addHandler {
      print("handler 1 called")
      count+=1
    }

    print("hooking up handler 2")
    disposable.append(simple.SignalEvent += {
      print("handler 2 called")
      count+=1
    })

    disposable.append(Simple.StaticEvent.addHandler { (_,_) in
      print("static handler 1 called")
      static_count+=1
    })

    disposable.append(Simple.StaticEvent += { (_,_) in
      print("static handler 2 called")
      static_count+=1
    })

    simple.FireEvent()
    XCTAssertEqual(count, 6)

    Simple.FireStaticEvent()
    XCTAssertEqual(static_count, 6)
  }

  public func TestAggregation() {
    let derived = Derived()
    derived.DoTheThing()

    let appDerived = AppDerived()
    print(type(of: appDerived))

    let b: Base = appDerived as Base
    print(type(of: b))

    appDerived.DoTheThing()
    XCTAssertEqual(appDerived.count, 1, "1. count not expected")

    print("foo");
    let appDerived2 = AppDerived2()
    print("foo");
    appDerived2.DoTheThing()
    print("foo");
    XCTAssertEqual(appDerived2.count, 1, "2. count not expected")

    print("foo");
    let appDerived3 = AppDerived3()
    appDerived3.DoTheThing()
    XCTAssertEqual(appDerived3.count, 1, "3. count not expected")
    XCTAssertEqual(appDerived3.beforeCount, 1, "4. count not expected")

    StaticClass.TakeBase(appDerived)
    XCTAssertEqual(appDerived.count, 2, "5. count not expected")
    
    StaticClass.TakeBase(appDerived2)
    XCTAssertEqual(appDerived2.count, 2, "6. count not expected")

    StaticClass.TakeBase(appDerived3)
    XCTAssertEqual(appDerived3.count, 2, "7. count not expected")
    XCTAssertEqual(appDerived3.beforeCount, 2, "8. count not expected")


    print("Testing unwrapping proper types from return values")
    let classy = Class()
    var base_returned = classy.BaseProperty!
    XCTAssert(type(of: base_returned) == Derived.self)

    print("testing app derived")
    classy.BaseProperty = appDerived

    base_returned = classy.BaseProperty!
    XCTAssert(type(of: base_returned) == AppDerived.self)
    XCTAssertIdentical(base_returned, appDerived)

    print("testing app derived2")

    classy.BaseProperty = appDerived2

    base_returned = classy.BaseProperty!
    XCTAssert(type(of: base_returned) == AppDerived2.self)
    XCTAssertIdentical(base_returned, appDerived2)

    print("testing app derived3")

    classy.BaseProperty = appDerived3

    base_returned = classy.BaseProperty!
    XCTAssert(type(of: base_returned) == AppDerived3.self)
    XCTAssertIdentical(base_returned, appDerived3)

    print("testing app derived no overrides")

    let derivedNoOverrides = AppDerivedNoOverrides()
    classy.BaseNoOverridesProperty = derivedNoOverrides

    var baseNoOverrides_returned = classy.BaseNoOverridesProperty!
    XCTAssert(type(of: baseNoOverrides_returned) == AppDerivedNoOverrides.self)
    XCTAssertIdentical(baseNoOverrides_returned, derivedNoOverrides)

    let derivedNoOverrides2 = AppDerivedNoOverrides2()
    classy.BaseNoOverridesProperty = derivedNoOverrides2
    baseNoOverrides_returned = classy.BaseNoOverridesProperty!
    XCTAssert(type(of: baseNoOverrides_returned) == AppDerivedNoOverrides2.self)
    XCTAssertIdentical(baseNoOverrides_returned, derivedNoOverrides2)
  }

  func TestUnicode() {
    let classy = Class()
    _ = classy.InString("\u{E909}")
  }
  
  public func TestVector() {
    let array = ["Hello", "Goodbye", "Goodnight"]

    var result = Class.InVector(array.toVector())
    print(result)
    XCTAssertEqual(result, "Hello")

    let classy = Class()
    let vector = classy.ReturnStoredStringVector()!
    XCTAssertEqual(vector.count, 1)
    print(vector[0])

    result = Class.InVector(vector)

    print(result)
    XCTAssertEqual(result, vector[0])

    vector.append("Goodbye")
    XCTAssertEqual(vector.count, 2)
    // Make sure the returned vector has the same data
    // as the one we modified
    let vector2 = classy.ReturnStoredStringVector()!
    XCTAssertEqual(vector2.count, vector.count)
    XCTAssertEqual(vector2[0], vector2[0])
    XCTAssertEqual(vector2[1], vector2[1])

    vector2.append("Goodnight")

    XCTAssertEqual(vector2.count, 3)
    XCTAssertEqual(vector.count, 3)
    XCTAssertEqual(vector2[2], vector2[2])
  }

  public func TestMap_asInput() {
    let dictionary = ["A": "Alpha"]
    let value = Class.InMap(dictionary.toMap())
    XCTAssertEqual(value, "Alpha")
  }
  
  public func TestMap_asReturn() {
    let classy = Class()
    let map = classy.ReturnMapFromStringToString()!
    XCTAssertEqual(map.count, 1)
    XCTAssert(map.HasKey("A"))
    XCTAssertEqual(map.Lookup("A"), "Alpha")
    XCTAssert(!map.HasKey("Z"))
  }

  public func TestMap_mutate() {
    let classy = Class()
    let map = classy.ReturnMapFromStringToString()!
    XCTAssert(map.HasKey("A"))
    XCTAssertEqual(map.Lookup("A"), "Alpha")
    
    XCTAssert(map.Insert("A", "Aleph")) // Returns true if replacing
    XCTAssert(map.HasKey("A"))
    XCTAssertEqual(map.Lookup("A"), "Aleph")
    let value = Class.InMap(map)
    XCTAssertEqual(value, "Aleph")
  }
  
  public func TestNullValues() {
    XCTAssertTrue(NullValues.IsObjectNull(nil))
    XCTAssertTrue(NullValues.IsInterfaceNull(nil))
    XCTAssertTrue(NullValues.IsGenericInterfaceNull(nil))
    XCTAssertTrue(NullValues.IsClassNull(nil))
    XCTAssertTrue(NullValues.IsDelegateNull(nil))
    
    XCTAssertFalse(NullValues.IsObjectNull(NoopClosable().getDefaultAsIInspectable()))
    XCTAssertFalse(NullValues.IsInterfaceNull(NoopClosable()))
    XCTAssertFalse(NullValues.IsGenericInterfaceNull([""].toVector()))
    XCTAssertFalse(NullValues.IsClassNull(NoopClosable()))
    XCTAssertFalse(NullValues.IsDelegateNull(VoidToVoidDelegate(handler: {})))

    XCTAssertNil(NullValues.GetNullObject())
    XCTAssertNil(NullValues.GetNullInterface())
    XCTAssertNil(NullValues.GetNullGenericInterface())
    XCTAssertNil(NullValues.GetNullClass())
    XCTAssertNil(NullValues.GetNullDelegate())
  }
}

var tests: [XCTestCaseEntry] = [
  testCase([
    ("TestAggregation", SwiftWinRTTests.TestAggregation),
    ("TestBlittableStruct", SwiftWinRTTests.TestBlittableStruct),
    ("TestChar", SwiftWinRTTests.TestChar),
    ("TestCustomConstructors", SwiftWinRTTests.TestCustomConstructors),
    ("TestDelegate", SwiftWinRTTests.TestDelegate),
    ("TestDoubleDelegate", SwiftWinRTTests.TestDoubleDelegate),
    ("TestEnums", SwiftWinRTTests.TestEnums),
    ("TestEvents", SwiftWinRTTests.TestEvents),
    ("TestIReference", SwiftWinRTTests.TestIReference),
    ("TestMap_asInput", SwiftWinRTTests.TestMap_asInput),
    ("TestMap_asReturn", SwiftWinRTTests.TestMap_asReturn),
    ("TestMap_mutate", SwiftWinRTTests.TestMap_mutate),
    ("TestNonBlittableStruct", SwiftWinRTTests.TestNonBlittableStruct),
    ("TestNonDefaultMethods", SwiftWinRTTests.TestNonDefaultMethods),
    ("TestNullValues", SwiftWinRTTests.TestNullValues),
    ("TestOutParams", SwiftWinRTTests.TestOutParams),
    ("TestStaticMethods", SwiftWinRTTests.TestStaticMethods),
    ("TestUnicode", SwiftWinRTTests.TestUnicode),
    ("TestVector", SwiftWinRTTests.TestVector),
  ])
]

RoInitialize(RO_INIT_MULTITHREADED)
XCTMain(tests)
