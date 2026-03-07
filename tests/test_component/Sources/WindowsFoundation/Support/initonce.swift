import CWinRT

/// A thread-safe one-time initialization primitive wrapping the Win32 `INIT_ONCE`.
///
/// Like `INIT_ONCE`, this is zero-initialized and ready to use (SE-0189). The
/// closure passed to `performOnce` is executed exactly once, even under concurrent
/// calls from multiple threads.
///
@_spi(WinRTInternal)
public struct InitOnce: ~Copyable {
    private var once = INIT_ONCE()

    public init() {}

    /// Executes `body` exactly once. Concurrent callers block until the first
    /// caller's `body` completes; subsequent callers return immediately.
    public mutating func performOnce(_ body: () -> Void) {
        var pending: WindowsBool = false
        guard InitOnceBeginInitialize(&once, 0, &pending, nil), pending == true else { return }
        body()
        InitOnceComplete(&once, 0, nil)
    }
}
