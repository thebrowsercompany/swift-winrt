import Foundation

/// A `WaitableEvent` can be a useful task synchronization tool when you want to
/// allow one task to wait for another task to finish some work.
///
/// A `WaitableEvent` has an internal signaled state. This value can be specified
/// at initialization, and by default is false.
///
/// A task can call the `wait` method to block until the signaled state is true.
/// A task can call the `signal` method to set the signaled state to true.
/// A task can call the `reset` method to set the signaled state back to false.
actor WaitableEvent {
    typealias Observer = @Sendable () -> Void

    private var observers: [Observer] = []
    private var value: Bool

    /// Initialize with an optional signaled state value. Default is `false`.
    init(signaled: Bool = false) {
        value = signaled
    }

    /// Block until the signaled state is `true`.
    func wait() async {
        guard !value else { return }
        await withCheckedContinuation { continuation in
            observers.append {
                continuation.resume(returning: ())
            }
        }
    }

    /// Set the signaled state to `true`, unblocking any tasks that have called `wait`
    /// or call `wait` in the future.
    func signal() async {
        value = true
        popObservers().forEach { $0() }
    }

    /// Reset the signaled state to `false`.
    func reset() async {
        value = false
    }

    private func appendObserver(_ observer: @escaping Observer) {
        observers.append(observer)
    }

    private func popObservers() -> [Observer] {
        let observers = self.observers
        self.observers = []
        return observers
    }
}
