import Foundation

actor WaitableEvent {
    typealias Observer = @Sendable () -> Void

    private var observer: Observer?
    private var signaled = false

    init() {}

    /// Block until the signaled state is `true`.
    func wait() async {
        guard !signaled else { return }
        guard observer == nil else {
            #if DEBUG
            preconditionFailure("message has already been waited on")
            #else
            return
            #endif
        }
        await withCheckedContinuation { continuation in
            observer = {
                continuation.resume(returning: ())
            }
        }
    }

    /// Signals the event, unblocking any current or future waiter.
    func signal() async {
        guard !signaled else {
            #if DEBUG
            preconditionFailure("message already signaled")
            #else
            return
            #endif
        }
        signaled = true
        observer?()
    }
}
