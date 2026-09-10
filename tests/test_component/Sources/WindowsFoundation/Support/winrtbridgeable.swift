@_spi(WinRTInternal)
public protocol ToAbi {
    associatedtype ABI
    func toABI() throws -> ABI
    // Balance ownership returned by toABI(); borrowed ABI values must not be released.
    static func release(abi: ABI)
}

@_spi(WinRTInternal)
extension ToAbi {
    public static func release(abi: ABI) {}
}

@_spi(WinRTInternal)
public protocol FromAbi {
    associatedtype ABI
    static func from(abi: ABI) -> Self
}

@_spi(WinRTInternal)
public typealias WinRTBridgeable = ToAbi & FromAbi
