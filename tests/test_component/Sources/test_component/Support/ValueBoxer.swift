
public class ValueBoxer
{
  // Box the swift object into something that can be understood by WinRT, this is done
  // with the following conditions:
  //   1. If the object is itself an IWinRTObject, get the IInspectable ptr representing
  //      that object
  //   2. Check the known primitive types and Windows.Foundation types which can be wrapped
  //      in a PropertyValue
  //   3. Check if wrapping a WinRTEnum, if so -
  static func boxValue(_ swift: Any?) -> test_component.IInspectable? {
    guard let swift else { return nil }
    if let winrtObj = swift as? IWinRTObject {
      return winrtObj.thisPtr
    } else if let propertyValue = PropertyValue.createFrom(swift) {
      return propertyValue
    } else if swift is WinRTEnum {
      fatalError("cant create enum")
    } else if swift is WinRTStruct {
      fatalError("can't create struct")
    } else {
      let wrapper = __ABI.AnyObjectWrapper(swift as AnyObject)
      let abi = try! wrapper?.toABI{ $0 }
      guard let abi else { return nil }
      return .init(abi)
    }
  }

}