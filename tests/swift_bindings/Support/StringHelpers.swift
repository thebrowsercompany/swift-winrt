import WinSDK

// TODO: Find clean wchar_t conversions and switch to W form of Win32 APIs.

// https://gist.github.com/yossan/51019a1af9514831f50bb196b7180107
//
// https://stackoverflow.com/questions/49451164/convert-swift-string-to-wchar-t
public func makeCString(from str: String) -> UnsafeMutablePointer<Int8> {
    let count = str.utf8.count + 1
    let result = UnsafeMutablePointer<Int8>.allocate(capacity: count)
    str.withCString { (baseAddress) in
        // func initialize(from: UnsafePointer<Pointee>, count: Int) 
        result.initialize(from: baseAddress, count: count)
    }
    return result
}

extension String {
    /// Calls the given closure with a pointer to the contents of the string,
    /// represented as a null-terminated wchar_t array.
    func withWideChars<Result>(_ body: (UnsafePointer<wchar_t>) -> Result) -> Result {
        let u32 = self.unicodeScalars.map { wchar_t(bitPattern: Int16($0.value)) } + [0]
        return u32.withUnsafeBufferPointer { body($0.baseAddress!) }
    }
}