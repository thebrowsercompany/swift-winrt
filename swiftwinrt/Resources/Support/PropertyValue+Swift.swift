import C_BINDINGS_MODULE

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