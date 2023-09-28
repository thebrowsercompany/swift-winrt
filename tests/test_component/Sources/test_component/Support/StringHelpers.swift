import Ctest_component

extension String {
    /// Calls the given closure with a pointer to the contents of the string,
    /// represented as a null-terminated wchar_t array.
    func withWideChars<Result>(_ body: (UnsafePointer<WCHAR>) throws -> Result) throws -> Result {
        var u16 = self.utf16.map { WCHAR(bitPattern: Int16(truncatingIfNeeded: $0)) }
        u16.append(0)
        return try u16.withUnsafeBufferPointer { try body($0.baseAddress!) }
    }
}