import Foundation
import CWinRT
import WinSDK

// Handwritten implementation for PropertyValue which *doesn't* try to do the IInspectable <-> Any mapping.
// This class is used by the AnyWrapper to create IInspectable instances from this subset of known Any values.
internal final class PropertyValue {
    private static let _IPropertyValueStatics: IPropertyValueStatics = try! RoGetActivationFactory("Windows.Foundation.PropertyValue")

    public static func createUInt8(_ value: UInt8) -> WindowsFoundation.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateUInt8Impl(value)
        return .init(propertyValue!)
    }

    public static func createInt16(_ value: Int16) -> WindowsFoundation.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateInt16Impl(value)
        return .init(propertyValue!)
    }

    public static func createUInt16(_ value: UInt16) -> WindowsFoundation.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateUInt16Impl(value)
        return .init(propertyValue!)
    }

    public static func createInt32(_ value: Int32) -> WindowsFoundation.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateInt32Impl(value)
        return .init(propertyValue!)
    }

    public static func createUInt32(_ value: UInt32) -> WindowsFoundation.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateUInt32Impl(value)
        return .init(propertyValue!)
    }

    public static func createInt64(_ value: Int64) -> WindowsFoundation.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateInt64Impl(value)
        return .init(propertyValue!)
    }

    public static func createUInt64(_ value: UInt64) -> WindowsFoundation.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateUInt64Impl(value)
        return .init(propertyValue!)
    }

    public static func createSingle(_ value: Float) -> WindowsFoundation.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateSingleImpl(value)
        return .init(propertyValue!)
    }

    public static func createDouble(_ value: Double) -> WindowsFoundation.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateDoubleImpl(value)
        return .init(propertyValue!)
    }

    public static func createBoolean(_ value: Bool) -> WindowsFoundation.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateBooleanImpl(.init(from: value))
        return .init(propertyValue!)
    }

    public static func createString(_ value: String) -> WindowsFoundation.IInspectable {
        let _value = try! HString(value)
        let propertyValue = try! _IPropertyValueStatics.CreateStringImpl(_value.get())
        return .init(propertyValue!)
    }

    public static func createGuid(_ value: GUID) -> WindowsFoundation.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateGuidImpl(value)
        return .init(propertyValue!)
    }

    public static func createDateTime(_ value: DateTime) -> WindowsFoundation.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateDateTimeImpl(.from(swift: value))
        return .init(propertyValue!)
    }

    public static func createTimeSpan(_ value: TimeSpan) -> WindowsFoundation.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateTimeSpanImpl(.from(swift: value))
        return .init(propertyValue!)
    }

    public static func createPoint(_ value: Point) -> WindowsFoundation.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreatePointImpl(.from(swift: value))
        return .init(propertyValue!)
    }

    public static func createSize(_ value: Size) -> WindowsFoundation.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateSizeImpl(.from(swift: value))
        return .init(propertyValue!)
    }

    public static func createRect(_ value: Rect) -> WindowsFoundation.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateRectImpl(.from(swift: value))
        return .init(propertyValue!)
    }
}

extension PropertyValue
{
    static func createInt(_ value: Int) -> WindowsFoundation.IInspectable {
       #if arch(x86_64) || arch(arm64)
           return PropertyValue.createInt64(Int64(value))
        #elseif arch(i386) || arch(arm)
          return PropertyValue.createInt32(Int32(value))
        #else
          fatalError("unknown process architecture size")
        #endif
    }

    static func createUInt(_ value: UInt) -> WindowsFoundation.IInspectable {
        #if arch(x86_64) || arch(arm64)
           return PropertyValue.createUInt64(UInt64(value))
        #elseif arch(i386) || arch(arm)
          return PropertyValue.createUInt32(UInt32(value))
        #else
          fatalError("unknown process architecture size")
        #endif
    }

    static func createFrom(_ any: Any) -> WindowsFoundation.IInspectable? {
        switch any {
            case let value as String:  return PropertyValue.createString(value)
            case let value as Int:     return PropertyValue.createInt(value)
            case let value as Int16:   return PropertyValue.createInt16(value)
            case let value as Int32:   return PropertyValue.createInt32(value)
            case let value as Int64:   return PropertyValue.createInt64(value)
            case let value as UInt:    return PropertyValue.createUInt(value)
            case let value as UInt8:   return PropertyValue.createUInt8(value)
            case let value as UInt32:  return PropertyValue.createUInt32(value)
            case let value as UInt64:  return PropertyValue.createUInt64(value)
            case let value as Float:   return PropertyValue.createSingle(value)
            case let value as Double:  return PropertyValue.createDouble(value)
            case let value as Bool:    return PropertyValue.createBoolean(value)
            case let value as GUID:    return PropertyValue.createGuid(value)
            case let value as DateTime: return PropertyValue.createDateTime(value)
            case let value as TimeSpan: return PropertyValue.createTimeSpan(value)
            case let value as Point:    return PropertyValue.createPoint(value)
            case let value as Size:     return PropertyValue.createSize(value)
            case let value as Rect:     return PropertyValue.createRect(value)
            default: return nil
        }
    }
}
