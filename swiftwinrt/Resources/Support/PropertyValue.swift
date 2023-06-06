import Foundation

// Handwritten implementation for PropertyValue which *doesn't* try to do the IInspectable <-> Any mapping.
// This class is used by the AnyWrapper to create IInspectable instances from this subset of known Any values.
internal final class PropertyValue {
    private static let _IPropertyValueStatics: __ABI_Windows_Foundation.IPropertyValueStatics = try! RoGetActivationFactory(HString("Windows.Foundation.PropertyValue"))
    public static func createEmpty() -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateEmptyImpl()
        return .init(propertyValue)
    }

    public static func createUInt8(_ value: UInt8) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateUInt8Impl(value)
        return .init(propertyValue)
    }

    public static func createInt16(_ value: Int16) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateInt16Impl(value)
        return .init(propertyValue)
    }

    public static func createUInt16(_ value: UInt16) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateUInt16Impl(value)
        return .init(propertyValue)
    }

    public static func createInt32(_ value: Int32) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateInt32Impl(value)
        return .init(propertyValue)
    }

    public static func createUInt32(_ value: UInt32) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateUInt32Impl(value)
        return .init(propertyValue)
    }

    public static func createInt64(_ value: Int64) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateInt64Impl(value)
        return .init(propertyValue)
    }

    public static func createUInt64(_ value: UInt64) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateUInt64Impl(value)
        return .init(propertyValue)
    }

    public static func createSingle(_ value: Float) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateSingleImpl(value)
        return .init(propertyValue)
    }

    public static func createDouble(_ value: Double) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateDoubleImpl(value)
        return .init(propertyValue)
    }

    public static func createChar16(_ value: Character) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateChar16Impl(.init(from: value))
        return .init(propertyValue)
    }

    public static func createBoolean(_ value: Bool) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateBooleanImpl(.init(from: value))
        return .init(propertyValue)
    }

    public static func createString(_ value: String) -> SUPPORT_MODULE.IInspectable {
        let _value = try! HString(value)
        let propertyValue = try! _IPropertyValueStatics.CreateStringImpl(_value.get())
        return .init(propertyValue)
    }

    public static func createGuid(_ value: UUID) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateGuidImpl(value)
        return .init(propertyValue)
    }

    public static func createDateTime(_ value: DateTime) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateDateTimeImpl(.from(swift: value))
        return .init(propertyValue)
    }

    public static func createTimeSpan(_ value: TimeSpan) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateTimeSpanImpl(.from(swift: value))
        return .init(propertyValue)
    }

    public static func createPoint(_ value: Point) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreatePointImpl(.from(swift: value))
        return .init(propertyValue)
    }

    public static func createSize(_ value: Size) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateSizeImpl(.from(swift: value))
        return .init(propertyValue)
    }

    public static func createRect(_ value: Rect) -> SUPPORT_MODULE.IInspectable {
        let propertyValue = try! _IPropertyValueStatics.CreateRectImpl(.from(swift: value))
        return .init(propertyValue)
    }
}