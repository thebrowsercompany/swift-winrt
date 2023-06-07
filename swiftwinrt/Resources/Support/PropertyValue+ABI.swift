import Ctest_component
import WinSDK

// Internal implementation of IPropertyValue
fileprivate var IID___x_ABI_CWindows_CFoundation_CIPropertyValueStatics: IID {
    IID(Data1: 0x629BDBC8, Data2: 0xD932, Data3: 0x4FF4, Data4: ( 0x96,0xB9,0x8D,0x96,0xC5,0xC1,0xE8,0x58 ))// 629BDBC8-D932-4FF4-96B9-8D96C5C1E858
}

internal class IPropertyValueStatics: test_component.IInspectable {
      override public class var IID: IID { IID___x_ABI_CWindows_CFoundation_CIPropertyValueStatics }

      internal func CreateEmptyImpl() throws -> UnsafeMutablePointer<Ctest_component.IInspectable>? {
          var propertyValue: UnsafeMutablePointer<Ctest_component.IInspectable>?
          _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
              try CHECKED(pThis.pointee.lpVtbl.pointee.CreateEmpty(pThis, &propertyValue))
          }
          return propertyValue
      }

      internal func CreateUInt8Impl(_ value: UINT8) throws -> UnsafeMutablePointer<Ctest_component.IInspectable>? {
          var propertyValue: UnsafeMutablePointer<Ctest_component.IInspectable>?
          _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
              try CHECKED(pThis.pointee.lpVtbl.pointee.CreateUInt8(pThis, value, &propertyValue))
          }
          return propertyValue
      }

      internal func CreateInt16Impl(_ value: INT16) throws -> UnsafeMutablePointer<Ctest_component.IInspectable>? {
          var propertyValue: UnsafeMutablePointer<Ctest_component.IInspectable>?
          _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
              try CHECKED(pThis.pointee.lpVtbl.pointee.CreateInt16(pThis, value, &propertyValue))
          }
          return propertyValue
      }

      internal func CreateUInt16Impl(_ value: UINT16) throws -> UnsafeMutablePointer<Ctest_component.IInspectable>? {
          var propertyValue: UnsafeMutablePointer<Ctest_component.IInspectable>?
          _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
              try CHECKED(pThis.pointee.lpVtbl.pointee.CreateUInt16(pThis, value, &propertyValue))
          }
          return propertyValue
      }

      internal func CreateInt32Impl(_ value: INT32) throws -> UnsafeMutablePointer<Ctest_component.IInspectable>? {
          var propertyValue: UnsafeMutablePointer<Ctest_component.IInspectable>?
          _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
              try CHECKED(pThis.pointee.lpVtbl.pointee.CreateInt32(pThis, value, &propertyValue))
          }
          return propertyValue
      }

      internal func CreateUInt32Impl(_ value: UINT32) throws -> UnsafeMutablePointer<Ctest_component.IInspectable>? {
          var propertyValue: UnsafeMutablePointer<Ctest_component.IInspectable>?
          _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
              try CHECKED(pThis.pointee.lpVtbl.pointee.CreateUInt32(pThis, value, &propertyValue))
          }
          return propertyValue
      }

      internal func CreateInt64Impl(_ value: INT64) throws -> UnsafeMutablePointer<Ctest_component.IInspectable>? {
          var propertyValue: UnsafeMutablePointer<Ctest_component.IInspectable>?
          _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
              try CHECKED(pThis.pointee.lpVtbl.pointee.CreateInt64(pThis, value, &propertyValue))
          }
          return propertyValue
      }

      internal func CreateUInt64Impl(_ value: UINT64) throws -> UnsafeMutablePointer<Ctest_component.IInspectable>? {
          var propertyValue: UnsafeMutablePointer<Ctest_component.IInspectable>?
          _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
              try CHECKED(pThis.pointee.lpVtbl.pointee.CreateUInt64(pThis, value, &propertyValue))
          }
          return propertyValue
      }

      internal func CreateSingleImpl(_ value: FLOAT) throws -> UnsafeMutablePointer<Ctest_component.IInspectable>? {
          var propertyValue: UnsafeMutablePointer<Ctest_component.IInspectable>?
          _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
              try CHECKED(pThis.pointee.lpVtbl.pointee.CreateSingle(pThis, value, &propertyValue))
          }
          return propertyValue
      }

      internal func CreateDoubleImpl(_ value: DOUBLE) throws -> UnsafeMutablePointer<Ctest_component.IInspectable>? {
          var propertyValue: UnsafeMutablePointer<Ctest_component.IInspectable>?
          _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
              try CHECKED(pThis.pointee.lpVtbl.pointee.CreateDouble(pThis, value, &propertyValue))
          }
          return propertyValue
      }

      internal func CreateChar16Impl(_ value: WCHAR) throws -> UnsafeMutablePointer<Ctest_component.IInspectable>? {
          var propertyValue: UnsafeMutablePointer<Ctest_component.IInspectable>?
          _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
              try CHECKED(pThis.pointee.lpVtbl.pointee.CreateChar16(pThis, value, &propertyValue))
          }
          return propertyValue
      }

      internal func CreateBooleanImpl(_ value: boolean) throws -> UnsafeMutablePointer<Ctest_component.IInspectable>? {
          var propertyValue: UnsafeMutablePointer<Ctest_component.IInspectable>?
          _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
              try CHECKED(pThis.pointee.lpVtbl.pointee.CreateBoolean(pThis, value, &propertyValue))
          }
          return propertyValue
      }

      internal func CreateStringImpl(_ value: HSTRING?) throws -> UnsafeMutablePointer<Ctest_component.IInspectable>? {
          var propertyValue: UnsafeMutablePointer<Ctest_component.IInspectable>?
          _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
              try CHECKED(pThis.pointee.lpVtbl.pointee.CreateString(pThis, value, &propertyValue))
          }
          return propertyValue
      }

      internal func CreateInspectableImpl(_ value: UnsafeMutablePointer<Ctest_component.IInspectable>?) throws -> UnsafeMutablePointer<Ctest_component.IInspectable>? {
          var propertyValue: UnsafeMutablePointer<Ctest_component.IInspectable>?
          _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
              try CHECKED(pThis.pointee.lpVtbl.pointee.CreateInspectable(pThis, value, &propertyValue))
          }
          return propertyValue
      }

      internal func CreateGuidImpl(_ value: GUID) throws -> UnsafeMutablePointer<Ctest_component.IInspectable>? {
          var propertyValue: UnsafeMutablePointer<Ctest_component.IInspectable>?
          _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
              try CHECKED(pThis.pointee.lpVtbl.pointee.CreateGuid(pThis, value, &propertyValue))
          }
          return propertyValue
      }

      internal func CreateDateTimeImpl(_ value: __x_ABI_CWindows_CFoundation_CDateTime) throws -> UnsafeMutablePointer<Ctest_component.IInspectable>? {
          var propertyValue: UnsafeMutablePointer<Ctest_component.IInspectable>?
          _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
              try CHECKED(pThis.pointee.lpVtbl.pointee.CreateDateTime(pThis, value, &propertyValue))
          }
          return propertyValue
      }

      internal func CreateTimeSpanImpl(_ value: __x_ABI_CWindows_CFoundation_CTimeSpan) throws -> UnsafeMutablePointer<Ctest_component.IInspectable>? {
          var propertyValue: UnsafeMutablePointer<Ctest_component.IInspectable>?
          _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
              try CHECKED(pThis.pointee.lpVtbl.pointee.CreateTimeSpan(pThis, value, &propertyValue))
          }
          return propertyValue
      }

      internal func CreatePointImpl(_ value: __x_ABI_CWindows_CFoundation_CPoint) throws -> UnsafeMutablePointer<Ctest_component.IInspectable>? {
          var propertyValue: UnsafeMutablePointer<Ctest_component.IInspectable>?
          _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
              try CHECKED(pThis.pointee.lpVtbl.pointee.CreatePoint(pThis, value, &propertyValue))
          }
          return propertyValue
      }

      internal func CreateSizeImpl(_ value: __x_ABI_CWindows_CFoundation_CSize) throws -> UnsafeMutablePointer<Ctest_component.IInspectable>? {
          var propertyValue: UnsafeMutablePointer<Ctest_component.IInspectable>?
          _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
              try CHECKED(pThis.pointee.lpVtbl.pointee.CreateSize(pThis, value, &propertyValue))
          }
          return propertyValue
      }

      internal func CreateRectImpl(_ value: __x_ABI_CWindows_CFoundation_CRect) throws -> UnsafeMutablePointer<Ctest_component.IInspectable>? {
          var propertyValue: UnsafeMutablePointer<Ctest_component.IInspectable>?
          _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
              try CHECKED(pThis.pointee.lpVtbl.pointee.CreateRect(pThis, value, &propertyValue))
          }
          return propertyValue
      }
  }