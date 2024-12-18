@_spi(WinRTInternal)
public protocol ToAbi {
    associatedtype ABI
    func toABI() throws -> ABI
}

@_spi(WinRTInternal)
public protocol FromAbi {
    associatedtype ABI
    static func from(abi: ABI) -> Self
}

@_spi(WinRTInternal)
public typealias WinRTBridgeable = ToAbi & FromAbi
