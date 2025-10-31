import CWinRT
import WinSDK

// Internal implementation of IPropertyValue
fileprivate var IID___x_ABI_CWindows_CFoundation_CIPropertyValueStatics: WindowsFoundation.IID {
    .init(Data1: 0x629BDBC8, Data2: 0xD932, Data3: 0x4FF4, Data4: ( 0x96,0xB9,0x8D,0x96,0xC5,0xC1,0xE8,0x58 ))// 629BDBC8-D932-4FF4-96B9-8D96C5C1E858
}

internal class IPropertyValueStatics: WindowsFoundation.IInspectable {
      override public class var IID: WindowsFoundation.IID { IID___x_ABI_CWindows_CFoundation_CIPropertyValueStatics }

      internal func CreateUInt8Impl(_ value: UINT8) throws -> ComPtr<C_IInspectable>? {
          let (propertyValue) = try ComPtrs.initialize { abi in
            _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
                try CHECKED(pThis.pointee.lpVtbl.pointee.CreateUInt8(pThis, value, &abi))
            }
          }
          return propertyValue
      }

      internal func CreateInt16Impl(_ value: INT16) throws -> ComPtr<C_IInspectable>? {
          let (propertyValue) = try ComPtrs.initialize { abi in
            _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
                try CHECKED(pThis.pointee.lpVtbl.pointee.CreateInt16(pThis, value, &abi))
            }
          }
          return propertyValue
      }

      internal func CreateUInt16Impl(_ value: UINT16) throws -> ComPtr<C_IInspectable>? {
          let (propertyValue) = try ComPtrs.initialize { abi in
            _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
                try CHECKED(pThis.pointee.lpVtbl.pointee.CreateUInt16(pThis, value, &abi))
            }
          }
          return propertyValue
      }

      internal func CreateInt32Impl(_ value: INT32) throws -> ComPtr<C_IInspectable>? {
          let (propertyValue) = try ComPtrs.initialize { abi in
            _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
                try CHECKED(pThis.pointee.lpVtbl.pointee.CreateInt32(pThis, value, &abi))
            }
          }
          return propertyValue
      }

      internal func CreateUInt32Impl(_ value: UINT32) throws -> ComPtr<C_IInspectable>? {
          let (propertyValue) = try ComPtrs.initialize { abi in
            _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
                try CHECKED(pThis.pointee.lpVtbl.pointee.CreateUInt32(pThis, value, &abi))
            }
          }
          return propertyValue
      }

      internal func CreateInt64Impl(_ value: INT64) throws -> ComPtr<C_IInspectable>? {
           let (propertyValue) = try ComPtrs.initialize { abi in
            _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
                try CHECKED(pThis.pointee.lpVtbl.pointee.CreateInt64(pThis, value, &abi))
            }
          }
          return propertyValue
      }

      internal func CreateUInt64Impl(_ value: UINT64) throws -> ComPtr<C_IInspectable>? {
          let (propertyValue) = try ComPtrs.initialize { abi in
            _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
                try CHECKED(pThis.pointee.lpVtbl.pointee.CreateUInt64(pThis, value, &abi))
            }
          }
          return propertyValue
      }

      internal func CreateSingleImpl(_ value: FLOAT) throws -> ComPtr<C_IInspectable>? {
          let (propertyValue) = try ComPtrs.initialize { abi in
            _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
                try CHECKED(pThis.pointee.lpVtbl.pointee.CreateSingle(pThis, value, &abi))
            }
          }
          return propertyValue
      }

      internal func CreateDoubleImpl(_ value: DOUBLE) throws -> ComPtr<C_IInspectable>? {
          let (propertyValue) = try ComPtrs.initialize { abi in
            _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
                try CHECKED(pThis.pointee.lpVtbl.pointee.CreateDouble(pThis, value, &abi))
            }
          }
          return propertyValue
      }

      internal func CreateChar16Impl(_ value: WCHAR) throws -> ComPtr<C_IInspectable>? {
          let (propertyValue) = try ComPtrs.initialize { abi in
            _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
                try CHECKED(pThis.pointee.lpVtbl.pointee.CreateChar16(pThis, value, &abi))
            }
          }
          return propertyValue
      }

      internal func CreateBooleanImpl(_ value: boolean) throws -> ComPtr<C_IInspectable>? {
          let (propertyValue) = try ComPtrs.initialize { abi in
            _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
                try CHECKED(pThis.pointee.lpVtbl.pointee.CreateBoolean(pThis, value, &abi))
            }
          }
          return propertyValue
      }

      internal func CreateStringImpl(_ value: HSTRING?) throws -> ComPtr<C_IInspectable>? {
          let (propertyValue) = try ComPtrs.initialize { abi in
            _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
                try CHECKED(pThis.pointee.lpVtbl.pointee.CreateString(pThis, value, &abi))
            }
          }
          return propertyValue
      }

      internal func CreateGuidImpl(_ value: GUID) throws -> ComPtr<C_IInspectable>? {
          let (propertyValue) = try ComPtrs.initialize { abi in
            _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
                try CHECKED(pThis.pointee.lpVtbl.pointee.CreateGuid(pThis, value, &abi))
            }
          }
          return propertyValue
      }

      internal func CreateDateTimeImpl(_ value: __x_ABI_CWindows_CFoundation_CDateTime) throws -> ComPtr<C_IInspectable>? {
          let (propertyValue) = try ComPtrs.initialize { abi in
            _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
                try CHECKED(pThis.pointee.lpVtbl.pointee.CreateDateTime(pThis, value, &abi))
            }
          }
          return propertyValue
      }

      internal func CreateTimeSpanImpl(_ value: __x_ABI_CWindows_CFoundation_CTimeSpan) throws -> ComPtr<C_IInspectable>? {
          let (propertyValue) = try ComPtrs.initialize { abi in
            _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
                try CHECKED(pThis.pointee.lpVtbl.pointee.CreateTimeSpan(pThis, value, &abi))
            }
          }
          return propertyValue
      }

      internal func CreatePointImpl(_ value: __x_ABI_CWindows_CFoundation_CPoint) throws -> ComPtr<C_IInspectable>? {
          let (propertyValue) = try ComPtrs.initialize { abi in
            _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
                try CHECKED(pThis.pointee.lpVtbl.pointee.CreatePoint(pThis, value, &abi))
            }
          }
          return propertyValue
      }

      internal func CreateSizeImpl(_ value: __x_ABI_CWindows_CFoundation_CSize) throws -> ComPtr<C_IInspectable>? {
          let (propertyValue) = try ComPtrs.initialize { abi in
            _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
                try CHECKED(pThis.pointee.lpVtbl.pointee.CreateSize(pThis, value, &abi))
            }
          }
          return propertyValue
      }

      internal func CreateRectImpl(_ value: __x_ABI_CWindows_CFoundation_CRect) throws -> ComPtr<C_IInspectable>? {
          let (propertyValue) = try ComPtrs.initialize { abi in
            _ = try perform(as: __x_ABI_CWindows_CFoundation_CIPropertyValueStatics.self) { pThis in
                try CHECKED(pThis.pointee.lpVtbl.pointee.CreateRect(pThis, value, &abi))
            }
          }
          return propertyValue
      }
  }