import WinSDK
import WinRT_Test
import TestComponent_CWinRT

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
      func DoThis() throws
      {
        print("Swift - DoThis!")
        thisCount += 1
      }

      func DoThat(_ val: Int32) throws
      {
        print("Swift - Do that: ", val)
        that = val
      }

      func GetThisCount() -> Int32 { thisCount }
      func GetThat() -> Int32 { that }
  }
  
  class MyImplementableDelegate: IIAmImplementable {
    private var thisCount = 9
    func InInt32(_ value: Int32) throws -> String {
        return .init(repeating: "abc", count: Int(value))
      }

    func InString(_ value: String) throws -> String {
        return .init(value.reversed())
      }

    func InEnum(_ value: Signed) throws -> String {
        switch value {
          case .First: return "1 banana"
          case .Second: return "2 banana"
          case .Third: return "3 banana"
          default: return "n/a"
        }
      }

      func OutInt32(_ value: inout Int32) throws {
        value = 987
      }

      func OutString(_ value: inout String?) throws {
        value = "987"
      }

      func OutBlittableStruct(_ value: inout BlittableStruct) throws {
        value = .init(First: 9876, Second: 54321)
      }

      func OutNonBlittableStruct(_ value: inout NonBlittableStruct) throws {
        value = .init(First: "to be", Second: "or not", Third: 2, Fourth: "be, that is the question")
      }

      func OutEnum(_ value: inout Signed) throws {
        value = .Second
      }

      func ReturnEnum() throws -> Signed {
        .Third
      }

      // TODO: this needs to swiftified
      private var enumProp: Fruit = .Apple
      func get_EnumProperty() throws -> Fruit {
        enumProp
      }

      func put_EnumProperty(_ value: Fruit) throws {
        enumProp = value
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

      try! mySwiftDelegate.DoThis()
      try! mySwiftDelegate.DoThat(6)
      assert(mySwiftDelegate.GetThisCount() == 2)
      assert(mySwiftDelegate.GetThat() == 6)

      // Get the delegate back and make sure it works
      // TODO: WIN-78 retrieving the delegate causes a memory
      // leak. We should verify the delegate is fully cleaned up
      var retrievedDelegate = classy.GetDelegate()
      try! retrievedDelegate.DoThis()
      try! retrievedDelegate.DoThat(9)

      assert(mySwiftDelegate.GetThisCount() == 3)
      assert(mySwiftDelegate.GetThat() == 9)
    
      // Null out the delegate from the C++ and retrieve a new one. Note that
      // we're initializing the swift object with `nil`, which is similar to 
      // how in C++/WinRT this works.

      classy.SetDelegate(MySimpleDelegate.none)

      // This will hand us a C++ class which implements the interface.
      retrievedDelegate = classy.GetDelegate()
      try! retrievedDelegate.DoThis()
      try! retrievedDelegate.DoThat(15)
      assert(mySwiftDelegate.GetThisCount() == 3)
      assert(mySwiftDelegate.GetThat() == 9)
    }
  
    do
    {
      print("TESTING IIAmImplementable")

      let impl = MyImplementableDelegate()
      classy = Class("with delegate", .Orange, impl)

      var enumVal = Signed.Second
      let returned = classy.InEnum(enumVal);
      assert(try! returned == impl.InEnum(enumVal), "improper value returned")

      enumVal = classy.ReturnEnum()
      assert(try! enumVal == impl.ReturnEnum(), "improper value returned")

      var enumProp = classy.EnumProperty
      print("class has: ", enumProp)
      assert(try! enumProp == impl.get_EnumProperty(), "i'd be apple'd if this was wrong")

      print("setting enum to Pineapple")
      classy.EnumProperty = .Pineapple
      enumProp = classy.EnumProperty
      print("a", enumProp, "is prickly")
      assert(try! enumProp == impl.get_EnumProperty(), "fruit should be prickly")

      print(">> testing OutInt32")
      var outInt: Int32 = 0
      classy.OutInt32(&outInt)
      print("     result: ", outInt)

      var expectedOutInt: Int32 = 0
      try! impl.OutInt32(&expectedOutInt)
      assert(outInt == expectedOutInt)

      print(">> testing OutString")
      var outString: String? 
      classy.OutString(&outString)

      var expectedOutString: String?
      try! impl.OutString(&expectedOutString)
      print("     result: ", outString!)
      assert(outString == expectedOutString)

      print(">> testing OutEnum")
      var outEnum: Signed = .Second
      classy.OutEnum(&outEnum)
      print("     result: ", outEnum)

      var expectedOutEnum: Signed = .Second
      try! impl.OutEnum(&expectedOutEnum)
      assert(outEnum == expectedOutEnum)

      print(">> testing OutBlittableStruct")
      var outBlittableStruct = BlittableStruct()
      classy.OutBlittableStruct(&outBlittableStruct)
      print("     result: ", outBlittableStruct)

      var outBlittableStructExpected = BlittableStruct()
      try! impl.OutBlittableStruct(&outBlittableStructExpected)
      // TODO: Implement equatable on structs
      //assert(outBlittableStruct == outBlittableStructExpected)
      assert(outBlittableStruct.First == outBlittableStructExpected.First)
      assert(outBlittableStruct.Second == outBlittableStructExpected.Second)

      print(">> testing OutNonBlittableStruct")
      var outNonBlittableStruct: NonBlittableStruct = .init()
      classy.OutNonBlittableStruct(&outNonBlittableStruct)
      print("     result: ", outNonBlittableStruct)

      var outNonBlittableStructExected = NonBlittableStruct()
      try! impl.OutNonBlittableStruct(&outNonBlittableStructExected)
      //assert(outNonBlittableStruct == outNonBlittableStructExected)
      assert(outNonBlittableStruct.First == outNonBlittableStructExected.First)
      assert(outNonBlittableStruct.Second == outNonBlittableStructExected.Second)
      assert(outNonBlittableStruct.Third == outNonBlittableStructExected.Third)
      assert(outNonBlittableStruct.Fourth == outNonBlittableStructExected.Fourth)
    
    }
   
    print("  ** Test passed! **")
  }
}

RoInitialize(RO_INIT_SINGLETHREADED)
let tests = SwiftWinRTTests()
tests.TestBlittableStruct()
tests.TestNonBlittableStruct()
tests.TestEnums()
tests.TestCustomConstructors()
tests.TestStaticMethods()
tests.TestOutParams()
tests.TestDelegate()

print("all tests passed!")
