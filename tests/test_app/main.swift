import WinSDK
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

class SwiftWinRTTests {
  public func TestBlittableStruct()
  {
    print("  ** Starting test case:TestBlittableStruct **")

    let simple = Simple()
    var blittableStruct = simple.ReturnBlittableStruct()
    print("first:", blittableStruct.First)
    print("second: ", blittableStruct.Second)

    assert(blittableStruct.First == 123, "not copied correctly")
    assert(blittableStruct.Second == 456, "not copied correctly")

    // TakeBlittableStruct requires the struct to have these values
    let putBlittableStruct = BlittableStruct(First: 654, Second: 321)

    simple.TakeBlittableStruct(putBlittableStruct)

    blittableStruct = simple.BlittableStructProperty
    print("first:", blittableStruct.First)
    print("second: ", blittableStruct.Second)

    assert(blittableStruct.First == 0, "not copied correctly")
    assert(blittableStruct.Second == 0, "not copied correctly")

    simple.BlittableStructProperty = putBlittableStruct

    blittableStruct = simple.BlittableStructProperty
    print("first:", blittableStruct.First)
    print("second: ", blittableStruct.Second)

    assert(blittableStruct.First == 654, "not copied correctly")
    assert(blittableStruct.Second == 321, "not copied correctly")

    print("  ** Test passed! **")
  }

  public func TestNonBlittableStruct()
  {
    print("  ** Starting test case:TestNonBlittableStruct **")

    let simple = Simple()
    var nonBlittableStruct = simple.ReturnNonBlittableStruct()
    print("first:", nonBlittableStruct.First!)
    print("second: ", nonBlittableStruct.Second!)

    assert(nonBlittableStruct.First == "Hello", "not copied correctly")
    assert(nonBlittableStruct.Second == "World", "not copied correctly")

    // TakeBlittableStruct requires the struct to have these values
    let putNonBlittableStruct = NonBlittableStruct(First: "From", Second: "Swift!", Third: 3, Fourth: "Yay!")

    simple.TakeNonBlittableStruct(putNonBlittableStruct)

    nonBlittableStruct = simple.NonBlittableStructProperty
    print("first:", nonBlittableStruct.First!)
    print("second: ", nonBlittableStruct.Second!)

    assert(nonBlittableStruct.First == "", "not copied correctly")
    assert(nonBlittableStruct.Second == "", "not copied correctly")

    simple.NonBlittableStructProperty = putNonBlittableStruct

    nonBlittableStruct = simple.NonBlittableStructProperty
    print("first:", nonBlittableStruct.First!)
    print("second: ", nonBlittableStruct.Second!)
    print("third:", nonBlittableStruct.Third)
    print("fourth: ", nonBlittableStruct.Fourth!)

    assert(nonBlittableStruct.First == "From", "not copied correctly")
    assert(nonBlittableStruct.Second == "Swift!", "not copied correctly")
    assert(nonBlittableStruct.Third == 3, "not copied correctly")
    assert(nonBlittableStruct.Fourth == "Yay!", "not copied correctly")

    print("  ** Test passed! **")
  }

  public func TestEnums()
  {
    print("  ** Starting test case:TestEnums **")
    let classy = Class()

    var enumVal = Signed.Second
    let returned = classy.InEnum(enumVal);
    assert(returned == "Second", "improper value returned")

    enumVal = classy.ReturnEnum()
    assert(enumVal == Signed.First, "improper value returned")

    var enumProp = classy.EnumProperty
    print("class has: ", enumProp)
    assert(enumProp == Fruit.Banana, "fruit should be b-a-n-a-n-a-s")

    print("setting enum to Apple")
    classy.EnumProperty = .Apple
    enumProp = classy.EnumProperty
    print("an", enumProp, "a day keeps the bugs away")
    assert(enumProp == Fruit.Apple, "fruit should be apple")
    print("  ** Test passed! **")
  }

  public func TestCustomConstructors()
  {
      print("  ** Starting test case:TestCustomConstructors **")
      var classy = Class("hello")
      assert(classy.EnumProperty == Fruit.Banana, "fruit should be bananas")

      classy = Class("world", Fruit.Pineapple)
      assert(classy.EnumProperty == Fruit.Pineapple, "fruit should be Pineapple")

      print("  ** Test passed! **")
  }

  public func TestStaticMethods()
  {
      print("  ** Starting test case:TestStaticMethods **")
      Class.StaticTest()

      var result = Class.StaticTestReturn()
      print("result: ", result);
      assert(result == 42);

      result = Class.StaticProperty
      print("result: ", result);
      assert(result == 18);

      print("calling static class methods")
      let output = StaticClass.InEnum(Signed.First)
      print("result: ", output)
      assert(output == "First")

      let fruit = StaticClass.EnumProperty
      print("result: ", fruit)
      assert(fruit == Fruit.Orange, "expected Orange!")

      StaticClass.EnumProperty = Fruit.Banana
      assert(StaticClass.EnumProperty == Fruit.Banana, "expected Banana!")

      let putNonBlittableStruct = NonBlittableStruct(First: "From", Second: "Swift!", Third: 1, Fourth: "Yay!")

      let putStructResult = StaticClass.InNonBlittableStruct(putNonBlittableStruct)
      print("result: ", putStructResult)
      assert(putStructResult == "FromSwift!Yay!")

      print("  ** Test passed! **")
  }

  public func TestOutParams()
  {
      print("  ** Starting test case:TestOutParams **")

      let classy = Class()
      print(">> testing OutInt32")
      var outInt: Int32 = 0
      classy.OutInt32(&outInt)
      print("     result: ", outInt)
      assert(outInt == 123)

      print(">> testing OutString")
      var outString: String? 
      classy.OutString(&outString)
      print("     result: ", outString!)
      assert(outString == "123")

      print(">> testing OutEnum")
      var outEnum: Signed = .Second
      classy.OutEnum(&outEnum)
      print("     result: ", outEnum)
      assert(outEnum == .First)

      print(">> testing OutBlittableStruct")
      var outBlittableStruct = BlittableStruct()
      classy.OutBlittableStruct(&outBlittableStruct)
      print("     result: ", outBlittableStruct)
      assert(outBlittableStruct.First == 867)
      assert(outBlittableStruct.Second == 5309)

      print(">> testing OutNonBlittableStruct")
      var outNonBlittableStruct: NonBlittableStruct = .init()
      classy.OutNonBlittableStruct(&outNonBlittableStruct)
      print("     result: ", outNonBlittableStruct)
      assert(outNonBlittableStruct.First == "please")
      assert(outNonBlittableStruct.Second == "vote")
      assert(outNonBlittableStruct.Third == 4)
      assert(outNonBlittableStruct.Fourth == "pedro")
      
      print("  ** Test passed! **")
  }

  class MySimpleDelegate : ISimpleDelegate
  {
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

      func OutString(_ value: inout String?) {
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

  public func TestDelegate()
  {
    print("  ** Starting test case: TestDelegate **")
    var classy = Class()

    do
    {
      print("TESTING ISimpleDelegate")

      let mySwiftDelegate = MySimpleDelegate()

      classy.SetDelegate(mySwiftDelegate)
      assert(mySwiftDelegate.GetThisCount() == 1)
      assert(mySwiftDelegate.GetThat() == 3)

      mySwiftDelegate.DoThis()
      mySwiftDelegate.DoThat(6)
      assert(mySwiftDelegate.GetThisCount() == 2)
      assert(mySwiftDelegate.GetThat() == 6)

      // Get the delegate back and make sure it works
      // TODO: WIN-78 retrieving the delegate causes a memory
      // leak. We should verify the delegate is fully cleaned up
      var retrievedDelegate = classy.GetDelegate()
      retrievedDelegate.DoThis()
      retrievedDelegate.DoThat(9)

      assert(mySwiftDelegate.GetThisCount() == 3)
      assert(mySwiftDelegate.GetThat() == 9)
    
      // Null out the delegate from the C++ and retrieve a new one. Note that
      // we're initializing the swift object with `nil`, which is similar to 
      // how in C++/WinRT this works.

      classy.SetDelegate(MySimpleDelegate.none)

      // This will hand us a C++ class which implements the interface.
      retrievedDelegate = classy.GetDelegate()
      retrievedDelegate.DoThis()
      retrievedDelegate.DoThat(15)
      assert(mySwiftDelegate.GetThisCount() == 3)
      assert(mySwiftDelegate.GetThat() == 9)

      classy.SetDelegate(retrievedDelegate)
    }
  
    do
    {
      print("TESTING IIAmImplementable")

      let impl = MyImplementableDelegate()
      classy = Class("with delegate", .Orange, impl)

      var enumVal = Signed.Second
      let returned = classy.InEnum(enumVal);
      assert(returned == impl.InEnum(enumVal), "improper value returned")

      enumVal = classy.ReturnEnum()
      assert(enumVal == impl.ReturnEnum(), "improper value returned")

      var enumProp = classy.EnumProperty
      print("class has: ", enumProp)
      assert(enumProp == impl.EnumProperty, "i'd be apple'd if this was wrong")

      print("setting enum to Pineapple")
      classy.EnumProperty = .Pineapple
      enumProp = classy.EnumProperty
      print("a", enumProp, "is prickly")
      assert(enumProp == impl.EnumProperty, "fruit should be prickly")

      print(">> testing OutInt32")
      var outInt: Int32 = 0
      classy.OutInt32(&outInt)
      print("     result: ", outInt)

      var expectedOutInt: Int32 = 0
      impl.OutInt32(&expectedOutInt)
      assert(outInt == expectedOutInt)

      print(">> testing OutString")
      var outString: String? 
      classy.OutString(&outString)

      var expectedOutString: String?
      impl.OutString(&expectedOutString)
      print("     result: ", outString!)
      assert(outString == expectedOutString)

      print(">> testing OutEnum")
      var outEnum: Signed = .Second
      classy.OutEnum(&outEnum)
      print("     result: ", outEnum)

      var expectedOutEnum: Signed = .Second
      impl.OutEnum(&expectedOutEnum)
      assert(outEnum == expectedOutEnum)

      print(">> testing OutBlittableStruct")
      var outBlittableStruct = BlittableStruct()
      classy.OutBlittableStruct(&outBlittableStruct)
      print("     result: ", outBlittableStruct)

      var outBlittableStructExpected = BlittableStruct()
      impl.OutBlittableStruct(&outBlittableStructExpected)
      // TODO: Implement equatable on structs
      //assert(outBlittableStruct == outBlittableStructExpected)
      assert(outBlittableStruct.First == outBlittableStructExpected.First)
      assert(outBlittableStruct.Second == outBlittableStructExpected.Second)

      print(">> testing OutNonBlittableStruct")
      var outNonBlittableStruct: NonBlittableStruct = .init()
      classy.OutNonBlittableStruct(&outNonBlittableStruct)
      print("     result: ", outNonBlittableStruct)

      var outNonBlittableStructExected = NonBlittableStruct()
      impl.OutNonBlittableStruct(&outNonBlittableStructExected)
      //assert(outNonBlittableStruct == outNonBlittableStructExected)
      assert(outNonBlittableStruct.First == outNonBlittableStructExected.First)
      assert(outNonBlittableStruct.Second == outNonBlittableStructExected.Second)
      assert(outNonBlittableStruct.Third == outNonBlittableStructExected.Third)
      assert(outNonBlittableStruct.Fourth == outNonBlittableStructExected.Fourth)
    
    }
   
    print("  ** Test passed! **")
  }

  public func TestNonDefaultMethods()
  {
    print("  ** Starting test case: TestNonDefaultMethods **")
    Class.StaticPropertyFloat = 4.0
    assert(Class.StaticPropertyFloat == 4.0)
    assert(Class.StaticTestReturnFloat() == 42.24)
    let classy = Class()
    classy.Method()
    print("  ** Test passed! **")
  }

  public func TestChar()
  {
    print(" ** Starting Test case: TestChar **")
    let classy = Class()
    print("classy ReturnChar: ", classy.ReturnChar())
    assert(classy.ReturnChar() == "d")

    let result = classy.InChar("x")
    print("classy InChar: ", result)
    assert(result == "x")

    var out: Character = "_"
    classy.OutChar(&out)
    print("classy OutChar: ", out)
    assert(out == "z")
    print("  ** Test passed! **")

  }

  class DoTheNewBase : IBasic
  {
    func Method() {
      print("it's done")
    }
  }

  class DoubleDelegate : IBasic, ISimpleDelegate
  {
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

  public func TestDoubleDelegate()
  {
    print(" ** Starting Test case: TestDoubleDelegate **")

    let classy = Class()

    let newBase = DoTheNewBase()
    classy.Implementation = newBase
    classy.Method()

    var implReturn = classy.Implementation
    implReturn.Method()

    assert(implReturn === newBase, "incorrect swift object returned")

    classy.Implementation = DoTheNewBase.none

    implReturn = classy.Implementation
    assert(implReturn !== newBase, "incorrect swift object returned")
    implReturn.Method()

    let double = DoubleDelegate()
    classy.Implementation = double
    classy.SetDelegate(double)

    classy.Method()

    let delegate = classy.GetDelegate()
    delegate.DoThis()
    delegate.DoThat(19)

    assert(double === delegate)
    assert(double === classy.Implementation)

    print("  ** Test passed! **")

  }

  public func TestIReference()
  {
    print(" ** Starting Test case: TestIReference **")

    let classy = Class()
    assert(classy.StartValue == nil)

    classy.StartValue = 23
    print("value: ", classy.StartValue ?? "N/A")
    assert(classy.StartValue == 23)

    let id: WinSDK.UUID? = .init(uuidString: "E621E1F8-C36C-495A-93FC-0C247A3E6E5F")
    classy.ID = id

    print("ID: ", classy.ID ?? "00000000-0000-0000-0000-0000000")
    assert(classy.ID == id)

    print("  ** Test passed! **")

  }

  public func TestEvents()
  {
    print(" ** Starting Test case: TestEvents **")

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
    assert(count == 2)

    simple.FireEvent()
    assert(count == 4)

    Simple.FireStaticEvent()
    assert(static_count == 2)
    Simple.FireStaticEvent()
    assert(static_count == 4)

    // dispose of the handlers and make sure we
    // aren't getting more events
    for dispose in disposable {
      if let dispose = dispose {
        dispose.dispose()
      }
    }
    disposable.removeAll(keepingCapacity: true)

    simple.FireEvent()
    assert(count == 4)

    Simple.FireStaticEvent()
    assert(static_count == 4)

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
    assert(count == 6)

    Simple.FireStaticEvent()
    assert(static_count == 6)

    print("  ** Test passed! **")
  }

  public func TestAggregation() {
    print(" ** Starting Test case: TestAggregation **")

    let derived = Derived()
    derived.DoTheThing()

    let appDerived = AppDerived()
    print(type(of: appDerived))

    let b: Base = appDerived as Base
    print(type(of: b))

    appDerived.DoTheThing()
    assert(appDerived.count == 1, "1. count not expected")

    let appDerived2 = AppDerived2()
    appDerived2.DoTheThing()
    assert(appDerived2.count == 1, "2. count not expected")

    let appDerived3 = AppDerived3()
    appDerived3.DoTheThing()
    assert(appDerived3.count == 1, "3. count not expected")
    assert(appDerived3.beforeCount == 1, "4. count not expected")

    StaticClass.TakeBase(appDerived)
    assert(appDerived.count == 2, "5. count not expected")
    
    StaticClass.TakeBase(appDerived2)
    assert(appDerived2.count == 2, "6. count not expected")

    StaticClass.TakeBase(appDerived3)
    assert(appDerived3.count == 2, "7. count not expected")
    assert(appDerived3.beforeCount == 2, "8. count not expected")


    print("Testing unwrapping proper types from return values")
    let classy = Class()
    var base_returned = classy.BaseProperty
    assert(type(of: base_returned) == Derived.self)

    print("testing app derived")
    classy.BaseProperty = appDerived

    base_returned = classy.BaseProperty
    assert(type(of: base_returned) == AppDerived.self)
    assert(base_returned === appDerived)

    print("testing app derived2")

    classy.BaseProperty = appDerived2

    base_returned = classy.BaseProperty
    assert(type(of: base_returned) == AppDerived2.self)
    assert(base_returned === appDerived2)

    print("testing app derived3")

    classy.BaseProperty = appDerived3

    base_returned = classy.BaseProperty
    assert(type(of: base_returned) == AppDerived3.self)
    assert(base_returned === appDerived3)

    print("testing app derived no overrides")

    let derivedNoOverrides = AppDerivedNoOverrides()
    classy.BaseNoOverridesProperty = derivedNoOverrides

    var baseNoOverrides_returned = classy.BaseNoOverridesProperty
    assert(type(of: baseNoOverrides_returned) == AppDerivedNoOverrides.self)
    assert(baseNoOverrides_returned === derivedNoOverrides)

    let derivedNoOverrides2 = AppDerivedNoOverrides2()
    classy.BaseNoOverridesProperty = derivedNoOverrides2
    baseNoOverrides_returned = classy.BaseNoOverridesProperty
    assert(type(of: baseNoOverrides_returned) == AppDerivedNoOverrides2.self)
    assert(baseNoOverrides_returned === derivedNoOverrides2)

    print("  ** Test passed! **")
  }

  func TestUnicode() {
    print(" ** Starting Test case: TestUnicode **")

    let classy = Class()
    _ = classy.InString("\u{E909}")

    print("  ** Test passed! **")
  }
  
  public func TestVector() {
    print(" ** Starting Test case: TestVector **")

    let array = ["Hello", "Goodbye", "Goodnight"]

    var result = Class.InVector(array.toVector())
    print(result)
    assert(result == "Hello")

    let classy = Class()
    let vector = classy.ReturnStoredStringVector()
    assert(vector.count == 1)
    print(vector[0])

    result = Class.InVector(vector)

    print(result)
    assert(result == vector[0])

    vector.append("Goodbye")
    assert(vector.count == 2)
    // Make sure the returned vector has the same data
    // as the one we modified
    let vector2 = classy.ReturnStoredStringVector()
    assert(vector2.count == vector.count)
    assert(vector2[0] == vector2[0])
    assert(vector2[1] == vector2[1])

    vector2.append("Goodnight")

    assert(vector2.count == 3)
    assert(vector.count == 3)
    assert(vector2[2] == vector2[2])

    print("  ** Test passed! **")
  }

  public func TestMap_asInput() {
    print(" ** Starting Test case: TestMap_asInput **")

    let dictionary = ["A": "Alpha"]
    let value = Class.InMap(dictionary.toMap())
    assert(value == "Alpha")

    print("  ** Test passed! **")
  }
  
  public func TestMap_asReturn() {
    print(" ** Starting Test case: TestMap_asReturn **")

    let classy = Class()
    let map = classy.ReturnMapFromStringToString()
    assert(map.count == 1)
    assert(map.HasKey("A") && map.Lookup("A") == "Alpha")
    assert(!map.HasKey("Z"))
    
    print("  ** Test passed! **")
  }

  public func TestMap_mutate() {
    print(" ** Starting Test case: TestMap_mutate **")

    let classy = Class()
    let map = classy.ReturnMapFromStringToString()
    assert(map.HasKey("A") && map.Lookup("A") == "Alpha")
    
    assert(map.Insert("A", "Aleph")) // Returns true if replacing
    assert(map.HasKey("A") && map.Lookup("A") == "Aleph")
    let value = Class.InMap(map)
    assert(value == "Aleph")
    
    print("  ** Test passed! **")
  }
}

RoInitialize(RO_INIT_MULTITHREADED)
let tests = SwiftWinRTTests()

tests.TestBlittableStruct()
tests.TestNonBlittableStruct()
tests.TestEnums()
tests.TestCustomConstructors()
tests.TestStaticMethods()
tests.TestOutParams()
tests.TestDelegate()
tests.TestNonDefaultMethods()
tests.TestChar()
tests.TestDoubleDelegate()
tests.TestIReference()
tests.TestEvents()
tests.TestAggregation()
tests.TestUnicode()
tests.TestVector()
tests.TestMap_asInput()
tests.TestMap_asReturn()
tests.TestMap_mutate()
print("all tests passed!")
