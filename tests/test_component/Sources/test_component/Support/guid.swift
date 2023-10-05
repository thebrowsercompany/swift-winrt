import Ctest_component

extension GUID: CustomStringConvertible {
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

extension GUID: Equatable {
   public static func ==(_ lhs: GUID, _ rhs: GUID) -> Bool {
      return lhs.Data1 == rhs.Data1 &&
         lhs.Data2 == rhs.Data2 &&
         lhs.Data3 == rhs.Data3 &&
         lhs.Data4 == rhs.Data4
   }
}

public func ~=(_ lhs: GUID, _ rhs: GUID) -> Bool { lhs == rhs}
