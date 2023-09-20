import C_BINDINGS_MODULE
import WinSDK

// TODO: Find clean wchar_t conversions and switch to W form of Win32 APIs.

extension String {
    /// Calls the given closure with a pointer to the contents of the string,
    /// represented as a null-terminated wchar_t array.
    func withWideChars<Result>(_ body: (UnsafePointer<wchar_t>) throws -> Result) throws -> Result {
        let u16 = self.utf16.map { wchar_t(bitPattern: Int16(truncatingIfNeeded: $0)) } + [0]
        return try u16.withUnsafeBufferPointer { try body($0.baseAddress!) }
    }
}