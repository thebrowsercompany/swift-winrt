import Foundation
import C_BINDINGS_MODULE
import WinSDK

// Handwritten implementation for PropertyValue which *doesn't* try to do the IInspectable <-> Any mapping.
// This class is used by the AnyWrapper to create IInspectable instances from this subset of known Any values.
internal final class PropertyValue {
    private static let _IPropertyValueStatics: IPropertyValueStatics = try! RoGetActivationFactory(HString("Windows.Foundation.PropertyValue"))

    public static func createUInt8(_ value: UInt8) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateUInt8Impl(value)
        return .init(propertyValue!)
    }

    public static func createInt16(_ value: Int16) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateInt16Impl(value)
        return .init(propertyValue!)
    }

    public static func createUInt16(_ value: UInt16) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateUInt16Impl(value)
        return .init(propertyValue!)
    }

    public static func createInt32(_ value: Int32) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateInt32Impl(value)
        return .init(propertyValue!)
    }

    public static func createUInt32(_ value: UInt32) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateUInt32Impl(value)
        return .init(propertyValue!)
    }

    public static func createInt64(_ value: Int64) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateInt64Impl(value)
        return .init(propertyValue!)
    }

    public static func createUInt64(_ value: UInt64) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateUInt64Impl(value)
        return .init(propertyValue!)
    }

    public static func createSingle(_ value: Float) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateSingleImpl(value)
        return .init(propertyValue!)
    }

    public static func createDouble(_ value: Double) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateDoubleImpl(value)
        return .init(propertyValue!)
    }

    public static func createChar16(_ value: Character) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateChar16Impl(.init(from: value))
        return .init(propertyValue!)
    }

    public static func createBoolean(_ value: Bool) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateBooleanImpl(.init(from: value))
        return .init(propertyValue!)
    }

    public static func createString(_ value: String) -> SUPPORT_MODULE.IInspectable {
        let _value = try! HString(value)
        let propertyValue = try! _IPropertyValueStatics.CreateStringImpl(_value.get())
        return .init(propertyValue!)
    }

    public static func createGuid(_ value: WinSDK.UUID) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateGuidImpl(value)
        return .init(propertyValue!)
    }

    public static func createDateTime(_ value: DateTime) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateDateTimeImpl(.from(swift: value))
        return .init(propertyValue!)
    }

    public static func createTimeSpan(_ value: TimeSpan) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateTimeSpanImpl(.from(swift: value))
        return .init(propertyValue!)
    }

    public static func createPoint(_ value: Point) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreatePointImpl(.from(swift: value))
        return .init(propertyValue!)
    }

    public static func createSize(_ value: Size) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateSizeImpl(.from(swift: value))
        return .init(propertyValue!)
    }

    public static func createRect(_ value: Rect) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateRectImpl(.from(swift: value))
        return .init(propertyValue!)
    }
}

extension PropertyValue
{
    static func createInt(_ value: Int) -> SUPPORT_MODULE.IInspectable {
       #if arch(x86_64) || arch(arm64)
           return PropertyValue.createInt64(Int64(value))
        #elseif arch(i386) || arch(arm)
          return PropertyValue.createInt32(Int32(value))
        #else 
          fatalError("unknown process architecture size")
        #endif
    }
    
    static func createUInt(_ value: UInt) -> SUPPORT_MODULE.IInspectable {
        #if arch(x86_64) || arch(arm64)
           return PropertyValue.createUInt64(UInt64(value))
        #elseif arch(i386) || arch(arm)
          return PropertyValue.createUInt32(UInt32(value))
        #else 
          fatalError("unknown process architecture size")
        #endif
    }

    static func createFrom(_ any: Any) -> SUPPORT_MODULE.IInspectable? {
        if let _value = any as? String {
          return PropertyValue.createString(_value)
        } else if let _value = any as? Int {
          return PropertyValue.createInt(_value)
        } else if let _value = any as? UInt {
          return PropertyValue.createUInt(_value)
        } else if let _value = any as? Int32 {
          return PropertyValue.createInt32(_value)
        } else if let _value = any as? UInt8 {
          return PropertyValue.createUInt8(_value)
        } else if let _value = any as? Int16 {
          return PropertyValue.createInt16(_value)
        } else if let _value = any as? UInt32 {
          return PropertyValue.createUInt32(_value)
        } else if let _value = any as? Int64 {
          return PropertyValue.createInt64(_value)
        } else if let _value = any as? UInt64 {
          return PropertyValue.createUInt64(_value)
        } else if let _value = any as? Float {
          return PropertyValue.createSingle(_value)
        } else if let _value = any as? Double {
          return PropertyValue.createDouble(_value)
        } else if let _value = any as? Character {
          return PropertyValue.createChar16(_value)
        } else if let _value = any as? Bool {
          return PropertyValue.createBoolean(_value)
        } else if let _value = any as? DateTime {
          return PropertyValue.createDateTime(_value)
        } else if let _value = any as? TimeSpan {
          return PropertyValue.createTimeSpan(_value)
        } else if let _value = any as? Point {
          return PropertyValue.createPoint(_value)
        } else if let _value = any as? Size {
          return PropertyValue.createSize(_value)
        } else {
          return nil
        }
    }
}
