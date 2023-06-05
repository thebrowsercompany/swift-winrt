import Ctest_component
import Foundation

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

  static func unboxValue(_ value: UnsafeMutablePointer<Ctest_component.IInspectable>?) -> Any? {
    guard let value else { return nil }
    // Try to unwrap an app implemented object. If one doesn't exist then we'll create the proper WinRT type below
    if let instance = __ABI.AnyObjectWrapper.tryUnwrapFrom(abi: value) {
      return instance
    }

    // Try unwrapping the object, if the type name isn't specified then we'll default to just returning an IInspectable
    // object.
    // Note that we'll *never* be trying to create an app implemented object at this point
    let inspVal = IInspectable(value)
    let className = try! inspVal.GetSwiftClassName() 
    guard let baseType = NSClassFromString(className) as? any UnsealedWinRTClass.Type else {
      print("unable to unwrap \(className), defaulting to IInspectable")
      return inspVal
    }
    return baseType._makeFromAbi.from(abi: value)
  }

}