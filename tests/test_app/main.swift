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
    print("first:", nonBlittableStruct.First)
    print("second: ", nonBlittableStruct.Second)

    assert(nonBlittableStruct.First == "Hello", "not copied correctly")
    assert(nonBlittableStruct.Second == "World", "not copied correctly")

    // TakeBlittableStruct requires the struct to have these values
    let putNonBlittableStruct = NonBlittableStruct(First: "From", Second: "Swift!", Third: 3, Fourth: "Yay!")

    simple.TakeNonBlittableStruct(putNonBlittableStruct)

    nonBlittableStruct = simple.NonBlittableStructProperty
    print("first:", nonBlittableStruct.First)
    print("second: ", nonBlittableStruct.Second)

    assert(nonBlittableStruct.First == "", "not copied correctly")
    assert(nonBlittableStruct.Second == "", "not copied correctly")

    simple.NonBlittableStructProperty = putNonBlittableStruct

    nonBlittableStruct = simple.NonBlittableStructProperty
    print("first:", nonBlittableStruct.First)
    print("second: ", nonBlittableStruct.Second)
    print("third:", nonBlittableStruct.Third)
    print("fourth: ", nonBlittableStruct.Fourth)

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
    classy.EnumProperty = Fruit.Apple
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
}

RoInitialize(RO_INIT_SINGLETHREADED)
let tests = SwiftWinRTTests()
tests.TestBlittableStruct()
tests.TestNonBlittableStruct()
tests.TestEnums()
tests.TestCustomConstructors()
tests.TestStaticMethods()