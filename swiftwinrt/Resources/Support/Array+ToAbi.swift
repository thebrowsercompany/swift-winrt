import C_BINDINGS_MODULE
import Foundation

@_spi(WinRTInternal)
extension Array where Element: ToAbi {
    public func toABI(_ withAbi: (_ length: UInt32, _ bytes: UnsafeMutablePointer<Element.ABI>?) throws -> Void) throws {
        let abiArray: [Element.ABI] = try map { try $0.toABI() }
        try abiArray.withUnsafeBytes { (bytes: UnsafeRawBufferPointer) in
            let bytesPtr = bytes.baseAddress?.assumingMemoryBound(to: Element.ABI.self)
            try withAbi(UInt32(count), .init(mutating: bytesPtr))
        }
    }
}

@_spi(WinRTInternal)
extension Array where Element: Numeric {
    public func toABI(_ withAbi: (_ length: UInt32, _ bytes: UnsafeMutablePointer<Element>?) throws -> Void) throws {
        try withUnsafeBytes { (bytes: UnsafeRawBufferPointer) in
            let bytesPtr = bytes.baseAddress?.assumingMemoryBound(to: Element.self)
            try withAbi(UInt32(count), .init(mutating: bytesPtr))
        }
    }
}

@_spi(WinRTInternal)
extension Array {
      public func toABI<Bridge: AbiInterfaceBridge>(abiBridge: Bridge.Type, _ withAbi: (_ length: UInt32, _ bytes: UnsafeMutablePointer<UnsafeMutablePointer<Bridge.CABI>?>?) throws -> Void) throws where Element == Bridge.SwiftProjection?  {
        let abiWrapperArray: [InterfaceWrapperBase<Bridge>?] = map { .init($0) }
        let abiArray = try abiWrapperArray.map { try $0?.toABI{ $0 } }
        try abiArray.withUnsafeBytes { (bytes: UnsafeRawBufferPointer) in
            let bytesPtr = bytes.baseAddress?.assumingMemoryBound(to: UnsafeMutablePointer<Bridge.CABI>?.self)
            try withAbi(UInt32(count), .init(mutating: bytesPtr))
        }
    }
}

@_spi(WinRTInternal)
extension Array {
      public func toABI<Bridge: AbiBridge>(abiBridge: Bridge.Type, _ withAbi: (_ length: UInt32, _ bytes: UnsafeMutablePointer<UnsafeMutablePointer<Bridge.CABI>?>?) throws -> Void) throws where Element == Bridge.SwiftProjection?, Bridge.SwiftProjection: WinRTClass {
        let abiArray: [UnsafeMutablePointer<Bridge.CABI>?] = map { RawPointer($0) }
        try abiArray.withUnsafeBytes { (bytes: UnsafeRawBufferPointer) in
            let bytesPtr = bytes.baseAddress?.assumingMemoryBound(to: UnsafeMutablePointer<Bridge.CABI>?.self)
            try withAbi(UInt32(count), .init(mutating: bytesPtr))
        }
    }
}

