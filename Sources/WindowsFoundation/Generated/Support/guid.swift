import Foundation
import CWinRT

extension GUID: @retroactive CustomStringConvertible {
   public var description: String {
      withUnsafePointer(to: self) { pGUID in
         Array<WCHAR>(unsafeUninitializedCapacity: 40) {
            $1 = Int(StringFromGUID2(pGUID, $0.baseAddress, CInt($0.count)))
         }.withUnsafeBufferPointer {
            String(decodingCString: $0.baseAddress!, as: UTF16.self)
         }
      }
   }
}

extension GUID {
   /// Create a GUID from a string such as "E621E1F8-C36C-495A-93FC-0C247A3E6E5F"
   ///
   /// returns nil for invalid strings
   public init?(parsingString string: String){
      var _self : GUID = .init()

      // use UuidFromString because it expects the correct format.
      // See https://devblogs.microsoft.com/oldnewthing/20151015-00/?p=91351
      var bytes = string.utf8.map { UInt8($0) } + [0]
      guard bytes.withUnsafeMutableBufferPointer({ UuidFromStringA($0.baseAddress, &_self) }) == S_OK else { return nil }
      self = _self
   }
}

extension GUID: @retroactive Equatable {
   public static func ==(_ lhs: GUID, _ rhs: GUID) -> Bool {
      return lhs.Data1 == rhs.Data1 &&
         lhs.Data2 == rhs.Data2 &&
         lhs.Data3 == rhs.Data3 &&
         lhs.Data4 == rhs.Data4
   }
}

public func ~=(_ lhs: GUID, _ rhs: GUID) -> Bool { lhs == rhs}

public extension Foundation.UUID {
    init(from guid: GUID) {
        let uuid: uuid_t = (
            UInt8((guid.Data1 >> 24) & 0xff),
            UInt8((guid.Data1 >> 16) & 0xff),
            UInt8((guid.Data1 >> 8) & 0xff),
            UInt8(guid.Data1 & 0xff),
            UInt8((guid.Data2 >> 8) & 0xff),
            UInt8(guid.Data2 & 0xff),
            UInt8((guid.Data3 >> 8) & 0xff),
            UInt8(guid.Data3 & 0xff),
            guid.Data4.0,
            guid.Data4.1,
            guid.Data4.2,
            guid.Data4.3,
            guid.Data4.4,
            guid.Data4.5,
            guid.Data4.6,
            guid.Data4.7
        )
        self.init(uuid: uuid)
    }
}

public extension GUID {
    init(from uuid: Foundation.UUID) {
        self.init(
            Data1: UInt32((UInt32(uuid.uuid.0) << 24) | (UInt32(uuid.uuid.1) << 16) | (UInt32(uuid.uuid.2) << 8) | UInt32(uuid.uuid.3)),
            Data2: UInt16((UInt16(uuid.uuid.4) << 8) | UInt16(uuid.uuid.5)),
            Data3: UInt16((UInt16(uuid.uuid.6) << 8) | UInt16(uuid.uuid.7)),
            Data4: (
                uuid.uuid.8,
                uuid.uuid.9,
                uuid.uuid.10,
                uuid.uuid.11,
                uuid.uuid.12,
                uuid.uuid.13,
                uuid.uuid.14,
                uuid.uuid.15
            )
        )
    }
}