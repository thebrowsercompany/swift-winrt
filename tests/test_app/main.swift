import WinSDK
import WinRT_Test
import TestComponent_CWinRT

RoInitialize(RO_INIT_SINGLETHREADED)

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