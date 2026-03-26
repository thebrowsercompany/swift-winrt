import WinSDK

/// A lightweight reader-writer lock wrapping the Win32 `SRWLOCK`.
internal struct SRWLock: ~Copyable {
    enum Mode {
        case shared
        case exclusive
    }

    /// SRWLOCK_INIT is {0}, and Swift zero-initializes imported C structs (SE-0189),
    /// so `SRWLock()` is ready to use without calling `InitializeSRWLock`. The Swift
    /// stdlib's `Synchronization.Mutex` uses the same pattern on Windows.
    private var lock = SRWLOCK()

    init() {}

    mutating func withLock<Result>(_ mode: Mode, _ body: () throws -> Result) rethrows -> Result {
        switch mode {
        case .shared:
            AcquireSRWLockShared(&lock)
            defer { ReleaseSRWLockShared(&lock) }
            return try body()
        case .exclusive:
            AcquireSRWLockExclusive(&lock)
            defer { ReleaseSRWLockExclusive(&lock) }
            return try body()
        }
    }
}
