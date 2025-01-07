import C_BINDINGS_MODULE
import Foundation

@_spi(WinRTInternal)
public typealias WinRTArrayAbi<T> = (count: UInt32, start: UnsafeMutablePointer<T>?)

@_spi(WinRTInternal)
extension Array where Element: FromAbi {
    public static func from(abi: WinRTArrayAbi<Element.ABI>) -> [Element] {
        UnsafeBufferPointer(start: abi.start, count: Int(abi.count)).map { .from(abi: $0) }
    }
}

@_spi(WinRTInternal)
extension Array where Element: Numeric {
    public static func from(abi: WinRTArrayAbi<Element>) -> [Element] {
        Array(UnsafeBufferPointer(start: abi.start, count: Int(abi.count)))
    }
}

// RawRepresentable covers Enums, which are simply numberic types, but the where Element: Numeric doesn't
// cover them. These particular cases are written to ensure no accidental conversion of types that can't
// be simply cast to a C-style Array accidentally sneak in
@_spi(WinRTInternal)
extension Array where Element: RawRepresentable, Element.RawValue: Numeric {
    public static func from(abi: WinRTArrayAbi<Element>) -> [Element] {
        Array(UnsafeBufferPointer(start: abi.start, count: Int(abi.count)))
    }
}

@_spi(WinRTInternal)
extension Array {
    public static func from<Bridge: AbiInterfaceBridge>(abiBridge: Bridge.Type, abi: WinRTArrayAbi<UnsafeMutablePointer<Bridge.CABI>?>) -> [Element] where Element == Bridge.SwiftProjection? {
        UnsafeBufferPointer(start: abi.start, count: Int(abi.count)).map { InterfaceWrapperBase<Bridge>.unwrapFrom(abi: ComPtr($0)) }
    }
}

@_spi(WinRTInternal)
extension Array {
    public static func from<Bridge: AbiBridge>(abiBridge: Bridge.Type, abi: WinRTArrayAbi<UnsafeMutablePointer<Bridge.CABI>?>) -> [Element] where Element == Bridge.SwiftProjection?, Bridge.SwiftProjection: WinRTClass {
        UnsafeBufferPointer(start: abi.1, count: Int(abi.0)).map { Bridge.from(abi: ComPtr($0)) }
    }
}
