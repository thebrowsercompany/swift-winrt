import test_component
import WinSDK

class DoubleDelegate : IBasic, ISimpleDelegate {
  func method() {
    print("method doubled up")
  }

  func doThis() {
    print("Swift Double! - doThis!")
  }

  func doThat(_ val: Int32) {
    print("Swift Double! - Do that: ", val)
  }

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
      .third
    }

    var enumProperty: Fruit = .apple

    var id: WinSDK.UUID?
    func fireEvent(_ data: String) {
      _implementableEvent.raise?(data)
    }

    private var object: Any?
    func inObject(_ value: Any!) throws -> String {
      object = value
      guard let value else { return "nil" }
      return String(describing: value)
    }

    func outObject(_ value: inout Any!) throws {
      value = object
    }

    func returnObject() throws -> Any! {
      return object
    }

    @EventSource<InDelegate> var implementableEvent: Event<InDelegate>
} 