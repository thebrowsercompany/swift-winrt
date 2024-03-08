import Foundation
import WinSDK

protocol SwiftToWinRTBridge {
   associatedtype WinRTAbiType
   func toABI(_ body: (ABIType) throws -> Void) throws
}

extension String: SwiftToWinRTBridge {
    public typealias ABIType = C_BINDINGS_MODULE.HSTRING
    public func toABI(_ body: (ABIType) throws -> Void) throws {
        try self.withCString(encodedAs: UTF16.self) {
            var header: C_BINDINGS_MODULE.HSTRING_HEADER = .init()
            var hstring: C_BINDINGS_MODULE.HSTRING?
            try CHECKED(WindowsCreateStringReference($0, UINT32(wcslen($0)), &header, &hstring))
            try body(hstring!)
        }
    }
}

extension Bool: SwiftToWinRTBridge {
    public typealias ABIType = ABIType.boolean
    public func toABI(_ body: (ABIType) throws -> Void) throws {
        try body(self ? 1 : 0)
    }
}

public class AbiHelper {
    public static func toABI<T: SwiftToWinRTBridge>(_ value: T, _ body: (T.ABIType) throws -> Void) rethrows {
        return try value.toABI { $0 }
    }

    public static func toABI<T: SwiftToWinRTBridge, T2: SwiftToWinRTBridge>(_ value: T, _ value2: T2, (T.ABIType, T2.ABIType) throws -> Void) rethrows {
        return try value.toABI {
            return try value2.toABI {
                return try body($0, $1)
            }
        }
    }

    public static func toABI<T: SwiftToWinRTBridge, T2: SwiftToWinRTBridge, T3: SwiftToWinRTBridge>(_ value: T, _ value2: T2, _ value3: T3, (T.ABIType, T2.ABIType, T3.ABIType) throws -> Void) rethrows {
        return try value.toABI {
            return try value2.toABI {
                return try value3.toABI {
                    return try body($0, $1, $2)
                }
            }
        }
    }
}