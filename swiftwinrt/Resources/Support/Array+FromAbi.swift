import C_BINDINGS_MODULE
import Foundation

@_spi(WinRTInternal)
extension Array where Element: FromAbi {
    public static func from(abi: (UInt32, UnsafeMutablePointer<Element.ABI>?)) throws -> [Element] {
        try UnsafeBufferPointer(start: abi.1, count: Int(abi.0)).map { try .from(abi: $0) }
    }
}

@_spi(WinRTInternal)
extension Array where Element: Numeric {
    public static func from(abi: (UInt32, UnsafeMutablePointer<Element>?)) throws -> [Element] {
        Array(UnsafeBufferPointer(start: abi.1, count: Int(abi.0)))
    }
}

@_spi(WinRTInternal)
extension Array {
    public func from<Bridge: AbiInterfaceBridge>(abiBridge: Bridge.Type, abi: (UInt32, UnsafeMutablePointer<UnsafeMutablePointer<Bridge.CABI>?>?)) throws -> [Element] where Element == Bridge.SwiftProjection? {
        UnsafeBufferPointer(start: abi.1, count: Int(abi.0)).map { InterfaceWrapperBase<Bridge>.unwrapFrom(abi: ComPtr($0)) }
    }
}

@_spi(WinRTInternal)
extension Array {
    public func from<Bridge: AbiBridge>(abiBridge: Bridge.Type, abi: (UInt32, UnsafeMutablePointer<UnsafeMutablePointer<Bridge.CABI>?>?)) throws -> [Element] where Element == Bridge.SwiftProjection?, Bridge.SwiftProjection: WinRTClass {
        UnsafeBufferPointer(start: abi.1, count: Int(abi.0)).map { Bridge.from(abi: ComPtr($0)) }
    }
}
